#include "sendergang.hh"

using namespace std;

SenderGang::SenderGang( const double mean_on_duration,
			const double mean_off_duration,
			const unsigned int num_senders,
			const unsigned int window_size )
  : _gang(),
    _start_distribution( 1.0 / mean_off_duration ),
    _stop_distribution( 1.0 / mean_on_duration )
{
  for ( unsigned int i = 0; i < num_senders; i++ ) {
    _gang.emplace_back( _start_distribution.sample(),
			WindowSender( i, window_size ) );
  }
}

  void SenderGang::tick( Network & net, Receiver & rec, const unsigned int tickno )
{
  /* run senders */
  for ( auto &x : _gang ) {
    /* should it switch? */
    while ( get< 0 >( x ) <= tickno ) {
      /* switch */
      get< 1 >( x ).set_sending( ! get< 1 >( x ).sending() );

      /* increment next switch time */
      get< 0 >( x ) += (get< 1 >( x ).sending() ? _stop_distribution : _start_distribution).sample();
    }

    get< 1 >( x ).tick( net, rec, tickno );
  }
}
