#include <assert.h>
#include <utility>

#include "rat.hh"

using namespace std;

template <class NextHop>
void Rat::send( const unsigned int id, NextHop & next, const unsigned int tickno )
{
  assert( _packets_sent >= _packets_received );

  const unsigned int the_window( window( tickno ) );

  while ( _packets_sent < _packets_received + the_window ) {
    Packet p( id, _packets_sent++, tickno );
    _memory.packet_sent( p );
    next.accept( move( p ) );
  }
}
