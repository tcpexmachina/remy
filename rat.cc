#ifndef RAT_CC
#define RAT_CC

#include <assert.h>

#include "rat.hh"

template <class NextHop>
Rat<NextHop>::Rat( const unsigned int s_window )
  : _window( s_window ),
    _packets_sent( 0 ),
    _packets_received( 0 )
{
}

template <class NextHop>
void Rat<NextHop>::packets_received( const std::vector< Packet > & packets ) {
  _packets_received += packets.size();
}

template <class NextHop>
void Rat<NextHop>::send( const unsigned int id, NextHop & next, const unsigned int tickno )
{
  assert( _packets_sent >= _packets_received );

  while ( _packets_sent < _packets_received + _window ) {
    next.accept( Packet( id, _packets_sent++, tickno ) );
  }
}

template <class NextHop>
void Rat<NextHop>::dormant_tick( const unsigned int tickno __attribute((unused)) )
{
}

#endif
