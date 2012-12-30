#ifndef RAT_CC
#define RAT_CC

#include <assert.h>

#include <utility>

#include "rat.hh"

using namespace std;

template <class NextHop>
Rat<NextHop>::Rat( const Whiskers & s_whiskers )
  : _whiskers( s_whiskers ),
    _packets_sent( 0 ),
    _packets_received( 0 )
{
}

template <class NextHop>
void Rat<NextHop>::packets_received( const vector< Packet > & packets ) {
  _packets_received += packets.size();
  _whiskers.packets_received( packets );
}

template <class NextHop>
void Rat<NextHop>::send( const unsigned int id, NextHop & next, const unsigned int tickno )
{
  assert( _packets_sent >= _packets_received );

  const unsigned int window( _whiskers.window( tickno ) );

  while ( _packets_sent < _packets_received + window ) {
    Packet p( id, _packets_sent++, tickno );
    _whiskers.packet_sent( p );
    next.accept( move( p ) );
  }
}

template <class NextHop>
void Rat<NextHop>::dormant_tick( const unsigned int tickno __attribute((unused)) )
{
}

#endif
