#include <stdio.h>
#include <vector>

#include "sendergang.hh"
#include "network.hh"
#include "receiver.hh"

int main( void )
{
  SenderGang senders( 10000, 10000, 500 );
  Network net( 1 );
  Receiver rec;

  for ( unsigned int tick = 0;; tick++ ) {
    senders.tick( net, rec, tick );
    net.tick( rec, tick );
  }

  return 0;
}
