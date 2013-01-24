#include <stdio.h>
#include <vector>

#include "ratbreeder.hh"

using namespace std;

int main( void )
{
  WhiskerTree whiskers;
  RatBreeder breeder;

  unsigned int run = 0;

  printf( "Optimizing whisker: %s\n", whiskers.str().c_str() );

  while ( 1 ) {
    auto outcome = breeder.improve( whiskers );
    printf( "run = %u, score = %f\n", run, outcome.score );

    for ( auto &run : outcome.throughputs_delays ) {
      for ( auto &x : run ) {
	printf( "sender: [tp=%f, del=%f]\n", x.first, x.second );
      }
    }

    fflush( NULL );
    run++;
  }

  return 0;
}
