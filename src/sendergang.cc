#include "sendergang.hh"

using namespace std;

template <class SenderType>
SenderGang<SenderType>::SenderGang( const double mean_on_duration,
				    const double mean_off_duration,
				    const unsigned int num_senders,
				    const SenderType & exemplar,
				    PRNG & prng )
  : _gang(),
    _start_distribution( 1.0 / mean_off_duration, prng ),
    _stop_distribution( 1.0 / mean_on_duration, prng )
{
  for ( unsigned int i = 0; i < num_senders; i++ ) {
    _gang.emplace_back( i,
			_start_distribution.sample(),
			exemplar );
  }
}

template <class SenderType>
void SenderGang<SenderType>::switch_senders( const unsigned int num_sending, const double & tickno )
{
  /* let senders switch */
  for ( auto &x : _gang ) {
    x.switcher( tickno, _start_distribution, _stop_distribution, num_sending );
  }
}

template <class SenderType>
unsigned int SenderGang<SenderType>::count_active_senders( void ) const
{
  return accumulate( _gang.begin(), _gang.end(),
		     0, []( const unsigned int a, const SwitchedSender & b )
		     { return a + b.sending; } );
}

template <class SenderType>
template <class NextHop>
void SenderGang<SenderType>::tick( NextHop & next, Receiver & rec, const double & tickno )
{
  unsigned int num_sending = count_active_senders();

  switch_senders( num_sending, tickno );

  num_sending = count_active_senders();

  /* run senders */
  for ( auto &x : _gang ) {
    x.tick( next, rec, tickno, num_sending, _start_distribution );
  }
}

template <class SenderType>
void SenderGang<SenderType>::TimeSwitchedSender::switcher( const double & tickno,
							   Exponential & start_distribution,
							   Exponential & stop_distribution,
							   const unsigned int num_sending )
{
  /* should it switch? */
  while ( SwitchedSender::next_switch_tick <= tickno ) {
    assert( SwitchedSender::next_switch_tick == tickno );

    /* switch */
    SwitchedSender::sending ? SwitchedSender::switch_off( tickno, num_sending ) : SwitchedSender::switch_on( tickno );

    /* increment next switch time */
    SwitchedSender::next_switch_tick += (SwitchedSender::sending ? stop_distribution : start_distribution).sample();
  }
}

template <class SenderType>
void SenderGang<SenderType>::ByteSwitchedSender::switcher( const double & tickno,
							   Exponential & start_distribution __attribute((unused)),
							   Exponential & stop_distribution,
							   const unsigned int num_sending __attribute((unused)) )
{
  /* should it switch? */
  while ( SwitchedSender::next_switch_tick <= tickno ) {
    assert( SwitchedSender::next_switch_tick == tickno );

    assert( not SwitchedSender::sending ); /* never sets a time to switch off */

    /* switch on */
    SwitchedSender::switch_on( tickno );

    /* set next switch time to dummy value */
    SwitchedSender::next_switch_tick = numeric_limits<double>::max();

    /* set length of flow */
    unsigned int new_flow_length = lrint( ceil( stop_distribution.sample() ) );
    assert( new_flow_length > 0 );
    packets_sent_cap_ += new_flow_length;
  }
}

template <class SenderType>
void SenderGang<SenderType>::SwitchedSender::switch_on( const double & tickno )
{
  assert( !sending );
  sending = true;
  sender.reset( tickno );

  /* Advance internal_tick without accumulating sending time */
  internal_tick = tickno;
}

template <class SenderType>
void SenderGang<SenderType>::SwitchedSender::switch_off( const double & tickno,
							 const unsigned int num_sending )
{
  assert( sending );

  /* account for final extent of sending time */
  accumulate_sending_time_until( tickno, num_sending );

  sending = false;
}

template <class SenderType>
void SenderGang<SenderType>::SwitchedSender::accumulate_sending_time_until( const double & tickno,
									    const unsigned int num_sending ) {
  assert( sending );
  assert( tickno >= internal_tick );

  utility.sending_duration( tickno - internal_tick, num_sending );
  internal_tick = tickno;
}

template <class SenderType>
void SenderGang<SenderType>::SwitchedSender::receive_feedback( Receiver & rec, const double & tickno )
{
  if ( rec.readable( id ) ) {
    const std::vector< Packet > & packets = rec.packets_for( id );

    utility.packets_received( packets );
    sender.packets_received( packets, tickno );

    rec.clear( id );
  }
}

template <class SenderType>
template <class NextHop>
void SenderGang<SenderType>::TimeSwitchedSender::tick( NextHop & next, Receiver & rec,
						       const double & tickno,
						       const unsigned int num_sending,
						       Exponential & start_distribution __attribute((unused)) )
{
  SwitchedSender::receive_feedback( rec, tickno );

  /* possibly send packets */
  if ( SwitchedSender::sending ) {
    SwitchedSender::sender.send( SwitchedSender::id, next, tickno );
    SwitchedSender::accumulate_sending_time_until( tickno, num_sending );
  }
}

template <class SenderType>
template <class NextHop>
void SenderGang<SenderType>::ByteSwitchedSender::tick( NextHop & next, Receiver & rec,
						       const double & tickno,
						       const unsigned int num_sending,
						       Exponential & start_distribution )
{
  SwitchedSender::receive_feedback( rec );

  /* possibly send packets */
  if ( SwitchedSender::sending ) {
    assert( SwitchedSender::sender.packets_sent() < packets_sent_cap_ );
    SwitchedSender::sender.send( SwitchedSender::id, next, tickno, packets_sent_cap_ );
    SwitchedSender::accumulate_sending_time_until( tickno, num_sending );

    /* do we need to switch ourselves off? */
    if ( SwitchedSender::sender.packets_sent() == packets_sent_cap_ ) {
      SwitchedSender::switch_off( tickno, num_sending );
      SwitchedSender::next_switch_tick = tickno + start_distribution.sample();
    }
  }
}

template <class SenderType>
double SenderGang<SenderType>::utility( void ) const
{
  double total_utility = 0.0;
  for ( auto &x : _gang ) {
    total_utility += x.utility.utility();
  }

  return total_utility / _gang.size(); /* mean utility per sender */
}

template <class SenderType>
vector< pair< double, double > > SenderGang<SenderType>::throughputs_delays( void ) const
{
  vector< pair< double, double > > ret;
  ret.reserve( _gang.size() );

  for ( auto &x : _gang ) {
    ret.emplace_back( x.utility.average_throughput(), x.utility.average_delay() );
  }

  return ret;
}

template <class SenderType>
double SenderGang<SenderType>::SwitchedSender::next_event_time( const double & tickno ) const
{
  assert( next_switch_tick >= tickno );

  return min( next_switch_tick, sending ? sender.next_event_time( tickno ) : std::numeric_limits<double>::max() );
}

template <class SenderType>
double SenderGang<SenderType>::next_event_time( const double & tickno ) const
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
