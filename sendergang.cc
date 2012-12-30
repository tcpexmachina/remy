#include "sendergang.hh"

using namespace std;

template <template< class NextHop > class SenderType, class NextHop>
SenderGang<SenderType, NextHop>::SenderGang( const double mean_on_duration,
					     const double mean_off_duration,
					     const unsigned int num_senders,
					     const SenderType<NextHop> & exemplar )
  : _gang(),
    _start_distribution( 1.0 / mean_off_duration ),
    _stop_distribution( 1.0 / mean_on_duration )
{
  for ( unsigned int i = 0; i < num_senders; i++ ) {
    _gang.emplace_back( i,
			_start_distribution.sample(),
			exemplar );
  }
}

template <template< class NextHop > class SenderType, class NextHop>
void SenderGang<SenderType, NextHop>::tick( NextHop & next, Receiver & rec, const unsigned int tickno )
{
  /* run senders */
  for ( auto &x : _gang ) {
    x.tick( next, rec, tickno, _start_distribution, _stop_distribution );
  }
}

template <template< class NextHop > class SenderType, class NextHop>
void SenderGang<SenderType, NextHop>::SwitchedSender::tick( NextHop & next, Receiver & rec,
							    const unsigned int tickno,
							    Exponential & start_distribution,
							    Exponential & stop_distribution )
{
  /* should it switch? */
  while ( next_switch_tick <= tickno ) {
    /* switch */
    sending = !sending;

    /* increment next switch time */
    next_switch_tick += (sending ? stop_distribution : start_distribution).sample();
  }

  /* receive feedback */
  if ( rec.readable( id ) ) {
    const std::vector< Packet > packets = rec.collect( id );

    utility.packets_received( packets );
    sender.packets_received( packets );
  }

  /* possibly send packets */
  if ( sending ) {
    utility.sending_tick();
    sender.send( id, next, tickno );
  } else {
    sender.dormant_tick( tickno );
  }
}

template <template< class NextHop > class SenderType, class NextHop>
double SenderGang<SenderType, NextHop>::utility( void ) const
{
  double total_utility = 0.0;
  for ( auto &x : _gang ) {
    total_utility += x.utility.utility();
  }

  return total_utility;
}
