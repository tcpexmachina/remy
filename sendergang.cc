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
void SenderGang<SenderType>::tick( NextHop & next, Receiver & rec, const unsigned int tickno )
{
  /* run senders */
  for ( auto &x : _gang ) {
    x.tick( next, rec, tickno, _start_distribution, _stop_distribution );
  }
}

template <class SenderType>
template <class NextHop>
void SenderGang<SenderType>::SwitchedSender::tick( NextHop & next, Receiver & rec,
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
