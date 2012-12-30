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
    _gang.emplace_back( _start_distribution.sample(),
			exemplar );
    get< 1 >( _gang.back() ).set_id( i );
  }
}

template <template< class NextHop > class SenderType, class NextHop>
void SenderGang<SenderType, NextHop>::tick( NextHop & next, Receiver & rec, const unsigned int tickno )
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

    get< 1 >( x ).tick( next, rec, tickno );
  }
}

template <template< class NextHop > class SenderType, class NextHop>
double SenderGang<SenderType, NextHop>::utility( void ) const
{
  double total_utility = 0.0;
  for ( auto &x : _gang ) {
    total_utility += get< 1 >( x ).utility();
  }

  return total_utility;
}
