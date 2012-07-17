#include <assert.h>
#include <stdlib.h>

#include "network.hh"

Network::Network( const double s_rate )
  : _buffer(),
    _current_tick( 0 ),
    _egress_process( s_rate )
{
}

void Network::send( const Packet & p )
{
  assert( p.tick_sent == _current_tick );

  _buffer.push( p );
}

void Network::advance( const int tick_to, Receiver & rec )
{
  assert( _current_tick < tick_to );

  while ( (_current_tick < tick_to) && (!_buffer.empty()) ) {
    /* We do transmission just before departing each tick. */
    const int num = _egress_process.sample();
    for ( int i = 0; i < num; i++ ) {
      if ( _buffer.empty() ) {
	break;
      }

      Packet p( _buffer.front() );
      _buffer.pop();
      p.tick_received = _current_tick;

      rec.accept( p );
    }
  }

  _current_tick = tick_to;
}
