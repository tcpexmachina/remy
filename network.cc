#include <assert.h>
#include <stdlib.h>

#include "network.hh"

Network::Network( const double s_rate )
  : _buffer(),
    _egress_process( s_rate )
{
}

void Network::send( const Packet & p )
{
  _buffer.push( p );
}

void Network::tick( Receiver & rec, const int tickno )
{
  const int num = _egress_process.sample();

  for ( int i = 0; i < num; i++ ) {
    if ( _buffer.empty() ) {
      break;
    }

    rec.accept( _buffer.front(), tickno );
    _buffer.pop();
  }
}
