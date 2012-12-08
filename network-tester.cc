#include <stdio.h>
#include <vector>

#include "dumbsender.hh"
#include "network.hh"
#include "receiver.hh"

const int num_senders = 100;

int main( void )
{

  DumbSender sender( 0, 0.2 );
  Network net( .1 );
  Receiver rec( num_senders );

  for ( int tick = 0; tick < 1000000000; tick++ ) {
    sender.tick( net, rec, tick );
    net.tick( rec, tick );
  }

  return 0;
}
