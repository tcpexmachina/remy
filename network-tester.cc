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
    senders.emplace_back( i, 0.009 );
  }

  for ( int i = 0; i < 20000000; i++ ) {
    for ( auto &x : senders ) {
      x.tick( net, rec, i );
    }

    net.tick( rec, i );
  }

  return 0;
}
