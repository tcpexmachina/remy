#include <stdio.h>

#include "dumbsender.hh"
#include "network.hh"
#include "receiver.hh"

int main( void )
{
  DumbSender sender( 2, .15 );
  Network net( .1 );
  Receiver rec( 3 );

  int current_tick = 0;
  while ( 1 ) {
    sender.advance( current_tick, net );
    net.advance( current_tick, rec );
    std::vector< Packet > packets = rec.collect( 2 );

    for ( auto x = packets.begin(); x != packets.end(); x++ ) {
      /*
      printf( "[%d] Got packet < src=%d id=%d sent=%d recd=%d >\n",
	      current_tick, x->src, x->packet_id, x->tick_sent, x->tick_received );
      */
      printf( "%d %d\n", current_tick, x->tick_received - x->tick_sent );
    }

    current_tick++;
  }

  return 0;
}
