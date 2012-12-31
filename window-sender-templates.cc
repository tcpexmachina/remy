#ifndef WINDOW_SENDER_TEMPLATES_CC
#define WINDOW_SENDER_TEMPLATES_CC

#include <assert.h>

#include "window-sender.hh"

template <class NextHop>
void WindowSender::send( const unsigned int id, NextHop & next, const unsigned int tickno )
{
  assert( _packets_sent >= _packets_received );

  while ( _packets_sent < _packets_received + _window ) {
    next.accept( Packet( id, _packets_sent++, tickno ) );
  }
}

#endif
