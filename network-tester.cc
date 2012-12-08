#include <stdio.h>
#include <vector>

#include "window-sender.hh"
#include "network.hh"
#include "receiver.hh"

const int num_senders = 100;

int main( void )
{
  WindowSender sender( 0, 3 );
  Network net( 1 );
  Receiver rec( num_senders );

  for ( int tick = 0; tick < 100000000; tick++ ) {
    sender.tick( net, rec, tick );
    net.tick( rec, tick );
  }

  return 0;
}
