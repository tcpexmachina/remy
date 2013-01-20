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
  const double rate_cap( current_whisker.rate() );

  while ( _packets_sent < _packets_received + window ) {
    Packet p( id, _packets_sent++, tickno );
    _memory.packet_sent( p );
    next.accept( move( p ) );
    _internal_tick += 1.0 / rate_cap;
    if ( _internal_tick >= tickno + 1 ) {
      return;
    }
  }
}
