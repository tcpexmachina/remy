#include "window-sender.hh"

template <class NextHop>
WindowSender<NextHop>::WindowSender( const unsigned int s_id,
				     const unsigned int s_window )
  : _id( s_id ),
    _window( s_window ),
    _packets_sent( 0 ),
    _packets_received( 0 ),
    _sending( false ),
    _utility()
{
}

template <class NextHop>
void WindowSender<NextHop>::tick( NextHop & next, Receiver & rec, const unsigned int tickno )
{
  /* Receive feedback */
  const std::vector< Packet > packets = rec.collect( _id );

  _utility.packets_received( packets );
  _packets_received += packets.size();

  if ( !_sending ) {
    return;
  }

  /* Send */
  assert( _packets_sent >= _packets_received );

  _utility.sending_tick();

  while ( _packets_sent < _packets_received + _window ) {
    next.accept( Packet( _id, _packets_sent++, tickno ) );
  }
}
