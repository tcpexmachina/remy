#include <stdio.h>
#include <vector>

#include "dumbsender.hh"
#include "network.hh"
#include "receiver.hh"

#include "exponential.hh"

const int num_senders = 10;

int main( void )
{
  std::vector< DumbSender > senders;
  Network net( .1 );
  Receiver rec( num_senders );

  for ( int i = 0; i < num_senders; i++ ) {
    senders.emplace_back( i, 0.013, .0000001 );
  }

  uint64_t tick = 0;

  while ( 1 ) {
    for ( auto &x : senders ) {
      x.tick( net, rec, tick );
    }

    net.tick( rec, tick );

    tick++;
  }

  return 0;
}
