#include <stdio.h>
#include <vector>

#include "dumbsender.hh"
#include "network.hh"
#include "receiver.hh"

const int num_senders = 10;

int main( void )
{
  std::vector< DumbSender > senders;
  Network net( .1 );
  Receiver rec( num_senders );

  for ( int i = 0; i < num_senders; i++ ) {
    senders.emplace_back( i, 0.013, .0000001 );
  }

  for ( int tick = 0; tick < 100000000; tick++ ) {
    for ( auto &x : senders ) {
      x.tick( net, rec, tick );
    }

    net.tick( rec, tick );
  }

  return 0;
}
