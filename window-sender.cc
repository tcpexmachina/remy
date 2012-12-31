#include "window-sender.hh"

WindowSender::WindowSender( const unsigned int s_window )
  : _window( s_window ),
    _packets_sent( 0 ),
    _packets_received( 0 )
{
}

void WindowSender::packets_received( const std::vector< Packet > & packets ) {
  _packets_received += packets.size();
}

void WindowSender::dormant_tick( const unsigned int tickno __attribute((unused)) )
{
}
