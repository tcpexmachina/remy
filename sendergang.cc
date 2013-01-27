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
  /* let senders switch */
  for ( auto &x : _gang ) {
    x.switcher( tickno, _start_distribution, _stop_distribution );
  }

  /* count number sending */
  unsigned int num_sending = 0;
  for ( auto &x : _gang ) {
    if ( x.sending ) {
      num_sending++;
    }
  }

  /* run senders */
  for ( auto &x : _gang ) {
    x.tick( next, rec, tickno, num_sending );
  }
}

template <class SenderType>
void SenderGang<SenderType>::SwitchedSender::switcher( unsigned int tickno,
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
 }

template <class SenderType>
template <class NextHop>
void SenderGang<SenderType>::SwitchedSender::tick( NextHop & next, Receiver & rec,
						   const unsigned int tickno,
						   const unsigned int num_sending )
{
  /* receive feedback */
  if ( rec.readable( id ) ) {
    const std::vector< Packet > & packets = rec.packets_for( id );

    utility.packets_received( packets );
    sender.packets_received( packets );

    rec.clear( id );
  }

  /* possibly send packets */
  if ( sending ) {
    utility.sending_tick( num_sending );
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
