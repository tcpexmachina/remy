#include "network.hh"

int main( void )
{
  Network net( .01 );
  Receiver rec( 5 );

  Packet pack( 0, 0, 15 );

  net.advance( 15, rec );
  net.send( pack );

  return 0;
}
