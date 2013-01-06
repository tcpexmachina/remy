#include <stdio.h>
#include <vector>

#include "window-sender-templates.cc"
#include "random.hh"
#include "network.cc"

using namespace std;

void utility( const unsigned int window_size )
{
  Network<WindowSender> network( window_size, global_PRNG()() );

  for ( unsigned int i = 0; i < 100000; i++ ) {
    network.tick();
  }

  printf( "%3d: util=%9.5f", window_size, network.senders().utility() );

  const auto tds = network.senders().throughputs_delays();
  for ( auto &x : tds ) {
    printf( "    [ tp=%.4f del=%.4f ]", x.first, x.second );
  }
  printf( "\n" );

  fflush( NULL );
}

int main( void )
{
  for ( unsigned int window_size = 1; window_size < 1000; window_size++ ) {
    utility( window_size );
  }

  return 0;
}
