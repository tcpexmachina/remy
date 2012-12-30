#include <stdio.h>
#include <vector>

#include "sendergang.cc"
#include "network.cc"
#include "delay.cc"
#include "receiver.hh"
#include "window-sender.cc"

using namespace std;

void utility( const unsigned int window_size )
{
  typedef Network< Delay< Receiver > > MyNetwork;

  const WindowSender< MyNetwork > exemplar( window_size );

  SenderGang< WindowSender, MyNetwork > senders( 1000,
						 1000,
						 2,
						 exemplar );

  MyNetwork net( 1 );
  Delay< Receiver > delay( 100 );
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
