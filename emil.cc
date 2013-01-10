#include "emil.hh"

Emil::Emil( PRNG & prng )
  : _ewma( 0 ),
    _packets_sent( 0 ),
    _packets_received( 0 ),
    _fast_send_process( 1, prng ),
    _slow_send_process( 0.5, prng )
{
}

void Emil::packets_received( const std::vector< Packet > & packets ) {
  _packets_received += packets.size();

  _ewma += packets.size();
}

void Emil::dormant_tick( const unsigned int tickno __attribute((unused)) )
{
  if ( _packets_sent == _packets_received ) {
    _ewma += 0.5;
  }

  _ewma *= alpha;
}
