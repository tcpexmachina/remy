#include <stdio.h>
#include <vector>

#include "rat-templates.cc"
#include "random.hh"
#include "network.cc"

int main( void )
{
  const Rat::Whiskers default_whiskers;

  Network<Rat> network( default_whiskers, global_PRNG() );

  for ( unsigned int i = 0; i < 1000000; i++ ) {
    network.tick();
  }

  printf( "util=%9.5f", network.senders().utility() );

  const auto tds = network.senders().throughputs_delays();
  for ( auto &x : tds ) {
    printf( "    [ tp=%.4f del=%.4f ]", x.first, x.second );
  }
  printf( "\n" );

  const auto senders = network.senders().senders();
  for ( auto &x : senders ) {
    for ( auto &y : x->whiskers().whiskers() ) {
      printf( " [val=%.0f, count=%d]", y.representative_value().last_delay, y.count() );
    }
    printf( "\n\n" );
  }

  return 0;
}
