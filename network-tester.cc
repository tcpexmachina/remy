#include <stdio.h>

#include "dumbsender.hh"
#include "network.hh"
#include "receiver.hh"

int main( void )
{
  DumbSender sender( 2, .1 );
  Network net( .1 );
  Receiver rec( 3 );

  int current_tick = 0;

  while ( 1 ) {
    sender.tick( net, rec, current_tick );
    net.tick( rec, current_tick );

    current_tick++;
  }

  return 0;
}
