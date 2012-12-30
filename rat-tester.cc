#include <stdio.h>
#include <vector>

#include "sendergang.cc"
#include "network.cc"
#include "delay.cc"
#include "receiver.hh"
#include "rat.cc"

int main( void )
{
  typedef Network< Delay< Receiver > > MyNetwork;

  Rat< MyNetwork >::Whiskers default_whiskers;

  const Rat< MyNetwork > exemplar( default_whiskers );

  SenderGang< Rat, MyNetwork > senders( 1000,
					1000,
					2,
					exemplar );

  MyNetwork net( 1 );
  Delay< Receiver > delay( 100 );
  Receiver rec;

  unsigned int tick = 0;
  while ( 1 ) {
    for ( unsigned int j = 0; j < 100000; j++ ) {
      senders.tick( net, rec, tick );
      net.tick( delay, tick );
      delay.tick( rec, tick );
      tick++;
    }

    printf( "Utility ( @ %u ): %f\n", tick, senders.utility() );
  }

  return 0;
}
