#include <stdio.h>
#include <vector>

#include "emil-templates.cc"
#include "random.hh"
#include "network.cc"

using namespace std;

void utility( void )
{
  Network<Emil> network( Emil( global_PRNG() ), global_PRNG(), NetConfig() );

  for ( unsigned int i = 0; i < 1000000; i++ ) {
    network.tick();
  }

  printf( "util=%9.5f", network.senders().utility() );

  const auto tds = network.senders().throughputs_delays();
  for ( auto &x : tds ) {
    printf( "    [ tp=%.4f del=%.4f ]", x.first, x.second );
  }
  printf( "\n" );

  fflush( NULL );
}

int main( void )
{
  utility();

  return 0;
}
