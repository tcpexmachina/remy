#include "window-sender.hh"

WindowSender::WindowSender( const unsigned int s_id,
			    const unsigned int s_flow_id,
			    const unsigned int s_window )
  : _id( s_id ),
    _flow_id( s_flow_id ),
    _window( s_window ),
    _packets_sent( 0 ),
    _packets_received( 0 ),
    _flow_first_received( -1 ),
    _total_delay( 0 )
{
}

void WindowSender::tick( Network & net, Receiver & rec, const unsigned int tickno )
{
  /* Receive feedback */
  const std::vector< Packet > packets = rec.collect( _id );

  for ( auto &x : packets ) {
    _packets_received++;
    _total_delay += x.tick_received - x.tick_sent;
    if ( _flow_first_received == (unsigned int)( -1 ) ) {
      _flow_first_received = x.tick_received;
    }
  }

  /* Send */
  assert( _packets_sent >= _packets_received );

  while ( _packets_sent < _packets_received + _window ) {
    net.accept( Packet( _id, _flow_id, _packets_sent++, tickno ) );
  }
}

std::pair< double, double > WindowSender::stats( const unsigned int tickno ) const
{
  if ( _packets_received == 0 ) {
    return std::make_pair( -1, -1 );
  }

  assert( _flow_first_received != (unsigned int)( -1 ) );
  assert( _flow_first_received <= tickno );

  const unsigned int age = tickno - _flow_first_received;

  const double average_throughput = double( _packets_received ) / double( age );
  const double average_delay = double( _total_delay ) / double( _packets_received );

  return std::make_pair( average_throughput, average_delay );
}
