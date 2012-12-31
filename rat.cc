#ifndef RAT_CC
#define RAT_CC

#include <assert.h>

#include <utility>

#include "rat.hh"

using namespace std;

template <class NextHop>
Rat<NextHop>::Rat( const Whiskers & s_whiskers )
  :  _whiskers( s_whiskers ),
     _memory(),
    _packets_sent( 0 ),
    _packets_received( 0 )
{
}

template <class NextHop>
void Rat<NextHop>::packets_received( const vector< Packet > & packets ) {
  _packets_received += packets.size();
  _memory.packets_received( packets );
}

template <class NextHop>
void Rat<NextHop>::send( const unsigned int id, NextHop & next, const unsigned int tickno )
{
  assert( _packets_sent >= _packets_received );

  const unsigned int the_window( window( tickno ) );

  while ( _packets_sent < _packets_received + the_window ) {
    Packet p( id, _packets_sent++, tickno );
    _memory.packet_sent( p );
    next.accept( move( p ) );
  }
}

template <class NextHop>
void Rat<NextHop>::dormant_tick( const unsigned int tickno __attribute((unused)) )
{
}

template <class NextHop>
unsigned int Rat<NextHop>::window( const unsigned int tickno )
{
  _memory.advance_to( tickno );
  return _whiskers.get_whisker( _memory ).window();
}

template <class NextHop>
const typename Rat<NextHop>::Whisker & Rat<NextHop>::Whiskers::get_whisker( const Rat<NextHop>::Memory & _memory __attribute((unused)) )
{
  return _whiskers[ 0 ]; /* XXX */
}

template <class NextHop>
Rat<NextHop>::Whisker::Whisker()
  : _generation( 0 ),
    _window( 100 ),
    _count( 0 )
{
}

#endif
