#include <stdio.h>
#include <vector>

#include "sendergang.cc"
#include "network-templates.cc"
#include "delay.hh"
#include "receiver.hh"
#include "rat-templates.cc"

int main( void )
{
  const Rat::Whiskers default_whiskers;

  SenderGang<Rat> senders( 1000,
			   1000,
			   2,
			   default_whiskers );

  Network net( 1 );
  Delay delay( 100 );
  Receiver rec;

  unsigned int tick = 0;
  double last_utility = -INT_MAX;

  while ( 1 ) {
    for ( unsigned int j = 0; j < 100000; j++ ) {
      senders.tick( net, rec, tick );
      net.tick( delay, tick );
      delay.tick( rec, tick );
      tick++;
    }

    const double utility = senders.utility();

    if ( fabs( ( utility - last_utility ) / last_utility ) < .0001 ) {
      break;
    }

    last_utility = utility;
  }

  printf( "%d ticks: util=%9.5f", tick, senders.utility() );

  const auto tds = senders.throughputs_delays();
  for ( auto &x : tds ) {
    printf( "    [ tp=%.4f del=%.4f ]", x.first, x.second );
  }
  printf( "\n" );

  return 0;
}
