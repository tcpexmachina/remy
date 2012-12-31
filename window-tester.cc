#include <stdio.h>
#include <vector>

#include "sendergang.cc"
#include "network-templates.cc"
#include "delay.hh"
#include "receiver.hh"
#include "window-sender-templates.cc"

using namespace std;

void utility( const unsigned int window_size )
{
  SenderGang<WindowSender> senders( 1000,
				    1000,
				    2,
				    window_size );

  Network net( 1 );
  Delay delay( 100 );
  Receiver rec;

  for ( unsigned int tick = 0; tick < 100000; tick++ ) {
    senders.tick( net, rec, tick );
    net.tick( delay, tick );
    delay.tick( rec, tick );
  }

  printf( "%3d: util=%9.5f", window_size, senders.utility() );

  const auto tds = senders.throughputs_delays();
  for ( auto &x : tds ) {
    printf( "    [ tp=%.4f del=%.4f ]", x.first, x.second );
  }
  printf( "\n" );

  fflush( NULL );
}

int main( void )
{
  for ( unsigned int window_size = 1; window_size < 1000; window_size++ ) {
    utility( window_size );
  }

  return 0;
}
