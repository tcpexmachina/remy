#include <stdio.h>

#include "dumbsender.hh"
#include "network.hh"
#include "receiver.hh"

int main( void )
{
  DumbSender sender( 2, .1 );
  Network net( .1 );
  Receiver rec( 3 );

  for ( int i = 0; i < 20000000; i++ ) {
    sender.tick( net, rec, i );
    net.tick( rec, i );
  }

  return 0;
}
