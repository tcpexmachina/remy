#include <algorithm>

#include "sendergang.hh"

using namespace std;

template <class SenderType, class SwitcherType>
SenderGang<SenderType, SwitcherType>::SenderGang( const double mean_on_duration,
						  const double mean_off_duration,
						  const unsigned int num_senders,
						  const SenderType & exemplar,
						  PRNG & prng,
						  const unsigned int id_range_begin )
  : _gang(),
    _prng( prng ),
    _start_distribution( 1.0 / mean_off_duration ),
    _stop_distribution( 1.0 / mean_on_duration )
{
  for ( unsigned int i = 0; i < num_senders; i++ ) {
    _gang.emplace_back( i + id_range_begin,
			_start_distribution.sample( _prng ),
			exemplar );
  }
}

template <class SenderType, class SwitcherType>
SenderGang<SenderType, SwitcherType>::SenderGang()
  : _gang(),
    _prng( global_PRNG() ),
    _start_distribution( 1.0 ),
    _stop_distribution( 1.0 )
{
}

template <class SenderType, class SwitcherType>
void SenderGang<SenderType, SwitcherType>::switch_senders( const unsigned int num_sending, const double & tickno )
{
  /* let senders switch */
  for ( auto &x : _gang ) {
    x.switcher( tickno, _prng, _start_distribution, _stop_distribution, num_sending );
  }
}

template <class SenderType, class SwitcherType>
unsigned int SenderGang<SenderType, SwitcherType>::count_active_senders( void ) const
{
  return accumulate( _gang.begin(), _gang.end(),
		     0, []( const unsigned int a, const SwitchedSender<SenderType> & b )
		     { return a + b.sending; } );
}

template <class SenderType, class SwitcherType>
template <class NextHop>
void SenderGang<SenderType, SwitcherType>::tick( NextHop & next, Receiver & rec, const double & tickno )
{
  unsigned int num_sending = count_active_senders();

  switch_senders( num_sending, tickno );

  num_sending = count_active_senders();

  run_senders( next, rec, num_sending, tickno );
}

template <class SenderType, class SwitcherType>
template <class NextHop>
void SenderGang<SenderType, SwitcherType>::run_senders( NextHop & next, Receiver & rec,
							const unsigned int num_sending,
							const double & tickno )
{
  /* run senders in random order */
  vector<unsigned int> sender_indices;
  sender_indices.reserve( _gang.size() );
  for ( unsigned int i = 0; i < _gang.size(); i++ ) {
    sender_indices.emplace_back( i );
  }

  /* Fisher-Yates shuffle */
  shuffle( sender_indices.begin(), sender_indices.end(), _prng );

  for ( auto &x : sender_indices ) {
    _gang[ x ].tick( next, rec, tickno, num_sending, _prng, _start_distribution );
  }
}

template <class SenderType>
void TimeSwitchedSender<SenderType>::switcher( const double & tickno,
					       PRNG & prng,
					       Exponential & start_distribution,
					       Exponential & stop_distribution,
					       const unsigned int num_sending )
{
  /* should it switch? */
  while ( SwitchedSender<SenderType>::next_switch_tick <= tickno ) {
    assert( SwitchedSender<SenderType>::next_switch_tick == tickno );

    /* switch */
    SwitchedSender<SenderType>::sending ? SwitchedSender<SenderType>::switch_off( tickno, num_sending ) : SwitchedSender<SenderType>::switch_on( tickno );

    /* increment next switch time */
    SwitchedSender<SenderType>::next_switch_tick += (SwitchedSender<SenderType>::sending ? stop_distribution : start_distribution).sample( prng );
  }
}

template <class SenderType>
void ByteSwitchedSender<SenderType>::switcher( const double & tickno,
					       PRNG & prng,
					       Exponential & start_distribution __attribute((unused)),
					       Exponential & stop_distribution,
					       const unsigned int num_sending __attribute((unused)) )
{
  /* should it switch? */
  while ( SwitchedSender<SenderType>::next_switch_tick <= tickno ) {
    assert( SwitchedSender<SenderType>::next_switch_tick == tickno );

    assert( not SwitchedSender<SenderType>::sending ); /* never sets a time to switch off */

    /* switch on */
    SwitchedSender<SenderType>::switch_on( tickno );

    /* set next switch time to dummy value */
    SwitchedSender<SenderType>::next_switch_tick = numeric_limits<double>::max();

    /* set length of flow */
    unsigned int new_flow_length = lrint( ceil( stop_distribution.sample( prng ) ) );
    assert( new_flow_length > 0 );
    packets_sent_cap_ += new_flow_length;
  }
}

template <class SenderType>
void SwitchedSender<SenderType>::switch_on( const double & tickno )
{
  assert( !sending );
  sending = true;
  sender.reset( tickno );

  /* Advance internal_tick without accumulating sending time */
  internal_tick = tickno;
}

template <class SenderType>
void SwitchedSender<SenderType>::switch_off( const double & tickno,
					     const unsigned int num_sending )
{
  assert( sending );

  /* account for final extent of sending time */
  accumulate_sending_time_until( tickno, num_sending );

  sending = false;
}

template <class SenderType>
void SwitchedSender<SenderType>::accumulate_sending_time_until( const double & tickno,
								const unsigned int num_sending ) {
  assert( sending );
  assert( tickno >= internal_tick );

  if ( tickno > internal_tick ) {
      utility.sending_duration( tickno - internal_tick, num_sending );
      internal_tick = tickno;
    }
}

template <class SenderType>
void SwitchedSender<SenderType>::receive_feedback( Receiver & rec )
{
  if ( rec.readable( id ) ) {
    const std::vector< Packet > & packets = rec.packets_for( id );

    utility.packets_received( packets );
    sender.packets_received( packets );

    rec.clear( id );
  }
}

template <class SenderType>
template <class NextHop>
void TimeSwitchedSender<SenderType>::tick( NextHop & next, Receiver & rec,
					   const double & tickno,
					   const unsigned int num_sending,
					   PRNG & prng __attribute((unused)),
					   Exponential & start_distribution __attribute((unused)) )
{
  SwitchedSender<SenderType>::receive_feedback( rec );

  /* possibly send packets */
  if ( SwitchedSender<SenderType>::sending ) {
    SwitchedSender<SenderType>::sender.send( SwitchedSender<SenderType>::id, next, tickno );
    SwitchedSender<SenderType>::accumulate_sending_time_until( tickno, num_sending );
  }
}

template <class SenderType>
template <class NextHop>
void ExternalSwitchedSender<SenderType>::tick( NextHop & next, Receiver & rec,
					       const double & tickno,
					       const unsigned int num_sending,
					       PRNG & prng __attribute((unused)),
					       Exponential & start_distribution __attribute((unused)) )
{
  SwitchedSender<SenderType>::receive_feedback( rec );

  /* possibly send packets */
  if ( SwitchedSender<SenderType>::sending ) {
    SwitchedSender<SenderType>::sender.send( SwitchedSender<SenderType>::id, next, tickno );
    SwitchedSender<SenderType>::accumulate_sending_time_until( tickno, num_sending );
  }
}

template <class SenderType>
template <class NextHop>
void ByteSwitchedSender<SenderType>::tick( NextHop & next, Receiver & rec,
					   const double & tickno,
					   const unsigned int num_sending,
					   PRNG & prng,
					   Exponential & start_distribution )
{
  SwitchedSender<SenderType>::receive_feedback( rec );

  /* possibly send packets */
  if ( SwitchedSender<SenderType>::sending ) {
    assert( SwitchedSender<SenderType>::sender.packets_sent() < packets_sent_cap_ );
    SwitchedSender<SenderType>::sender.send( SwitchedSender<SenderType>::id, next, tickno, packets_sent_cap_ );
    SwitchedSender<SenderType>::accumulate_sending_time_until( tickno, num_sending );

    /* do we need to switch ourselves off? */
    if ( SwitchedSender<SenderType>::sender.packets_sent() == packets_sent_cap_ ) {
      SwitchedSender<SenderType>::switch_off( tickno, num_sending );
      SwitchedSender<SenderType>::next_switch_tick = tickno + start_distribution.sample( prng );
    }
  }
}

template <class SenderType, class SwitcherType>
double SenderGang<SenderType, SwitcherType>::utility( void ) const
{
  double total_utility = 0.0;

  /* If gang is empty, return zero */
  if ( _gang.empty() ) return 0.0;

  for ( auto &x : _gang ) {
    total_utility += x.utility.utility();
  }

  return total_utility / _gang.size(); /* mean utility per sender */
}

template <class SenderType, class SwitcherType>
vector< pair< double, double > > SenderGang<SenderType, SwitcherType>::throughputs_delays( void ) const
{
  vector< pair< double, double > > ret;
  ret.reserve( _gang.size() );

  for ( auto &x : _gang ) {
    ret.emplace_back( x.utility.average_throughput_normalized_to_equal_share(),
		      x.utility.average_delay() );
  }

  return ret;
}

template <class SenderType>
SenderDataPoint SwitchedSender<SenderType>::statistics_for_log( void ) const
{
  return SenderDataPoint( sender.state_DNA(), utility.DNA(), sending );
}

template <class SenderType, class SwitcherType>
vector< SenderDataPoint > SenderGang<SenderType, SwitcherType>::statistics_for_log( void ) const
{
  vector < SenderDataPoint > points;
  points.reserve( _gang.size() );
  for ( auto &x : _gang ) {
    points.push_back( x.statistics_for_log() );
  }
  return points;
}

template <class SenderType>
double SwitchedSender<SenderType>::next_event_time( const double & tickno ) const
{
  assert( next_switch_tick >= tickno );

  return min( next_switch_tick, sending ? sender.next_event_time( tickno ) : std::numeric_limits<double>::max() );
}

template <class SenderType, class SwitcherType>
double SenderGang<SenderType, SwitcherType>::next_event_time( const double & tickno ) const
{
  double ret = std::numeric_limits<double>::max();
  for ( const auto & x : _gang ) {
    const double the_next_event = x.next_event_time( tickno );
    assert( the_next_event >= tickno );
    if ( the_next_event < ret ) {
      ret = the_next_event;
    }
  }

  return ret;
}
