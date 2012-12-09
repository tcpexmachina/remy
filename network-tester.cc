#include <stdio.h>
#include <vector>

#include "sendergang.hh"
#include "network.hh"
#include "receiver.hh"

int main( void )
{
  SenderGang senders( 100, 100, 5 );
  Network net( 1 );
  Receiver rec( 1 );

  for ( unsigned int tick = 0; tick < 100000000; tick++ ) {
    senders.tick( net, rec, tick );
    net.tick( rec, tick );
  }

  return 0;
}
