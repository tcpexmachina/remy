#include <stdio.h>
#include <vector>

#include "sendergang.cc"
#include "network.cc"
#include "delay.cc"
#include "receiver.hh"

int main( void )
{
  SenderGang< Network< Delay< Receiver> > > senders( 1000,
						1000,
						2,
						50 );
  Network< Delay< Receiver > > net( 1 );
  Delay< Receiver > delay( 1280 );
  Receiver rec;

  for ( unsigned int tick = 0;; tick++ ) {
    senders.tick( net, rec, tick );
    net.tick( delay, tick );
    delay.tick( rec, tick );
  }

  return 0;
}
