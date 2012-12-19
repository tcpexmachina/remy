#include <stdio.h>
#include <vector>

#include "sendergang.hh"
#include "window-sender.hh"
#include "network.hh"
#include "receiver.hh"

int main( void )
{
  SenderGang senders( 1000,
		      1000,
		      2,
		      50 );
  Network net( 1 );
  Receiver rec;

  for ( unsigned int tick = 0;; tick++ ) {
    senders.tick( net, rec, tick );
    net.tick( rec, tick );
  }

  return 0;
}
