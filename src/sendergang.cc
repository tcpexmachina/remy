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
template <class NextHop>
void SenderGang<SenderType>::tick( NextHop & next, Receiver & rec, const double & tickno )
{
  /* run senders */
  for ( auto &x : _gang ) {
    x.tick( next, rec, tickno, _start_distribution, _stop_distribution );
  }
}

template <class SenderType>
template <class NextHop>
void SenderGang<SenderType>::SwitchedSender::tick( NextHop & next, Receiver & rec,
						   const double & tickno,
						   Exponential & start_distribution,
						   Exponential & stop_distribution )
{
  /* should it switch? */
  while ( next_switch_tick < tickno ) {
    /* switch */
    sending = !sending;

    /* increment next switch time */
    next_switch_tick += (sending ? stop_distribution : start_distribution).sample();

    /* reset sender */
    sender.reset( tickno );
  }

  /* receive feedback */
  if ( rec.readable( id ) ) {
    const std::vector< Packet > & packets = rec.packets_for( id );

    utility.packets_received( packets );
    sender.packets_received( packets );

    rec.clear( id );
  }

  /* possibly send packets */
  if ( sending ) {
    sender.send( id, next, tickno );
    utility.sending_duration( tickno - internal_tick );
  }

  internal_tick = tickno;
}

template <class SenderType>
double SenderGang<SenderType>::utility( void ) const
{
  double total_utility = 0.0;
  for ( auto &x : _gang ) {
    total_utility += x.utility.utility();
  }

  return total_utility;
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
const vector< const SenderType * > SenderGang<SenderType>::senders( void ) const
{
  vector< const SenderType * > ret;
  ret.reserve( _gang.size() );

  for ( auto &x : _gang ) {
    ret.emplace_back( &x.sender );
  }

  return ret;
}

template <class SenderType>
double SenderGang<SenderType>::SwitchedSender::next_event_time( const double & tickno ) const
{
  assert( next_switch_tick >= tickno );

  return min( next_switch_tick, sender.next_event_time( tickno ) );
}

template <class SenderType>
double SenderGang<SenderType>::next_event_time( const double & tickno ) const
{
  double ret = std::numeric_limits<double>::max();
  for ( const auto & x : _gang ) {
    const double the_next_event = x.next_event_time();
    assert( the_next_event >= tickno );
    if ( the_next_event < ret ) {
      ret = the_next_event;
    }
  }

  return ret;
}
