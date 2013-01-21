#include <assert.h>
#include <utility>

#include "rat.hh"

using namespace std;

template <class NextHop>
void Rat::send( const unsigned int id, NextHop & next, const unsigned int tickno )
{
  assert( _packets_sent >= _packets_received );

  _memory.advance_to( tickno );

  auto current_whisker( _whiskers.use_whisker( _memory, _track ) );

  const unsigned int window( current_whisker.window() );
  const double wait_time( current_whisker.intersend() );

  while ( _packets_sent < _packets_received + window ) {
    Packet p( id, _packets_sent++, tickno );
    _memory.packet_sent( p );
    next.accept( move( p ) );
    _internal_tick += wait_time;
    if ( _internal_tick >= tickno ) {
      return;
    }
  }
}
