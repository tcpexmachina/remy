#include <vector>

#include "memory.hh"

using namespace std;

static const double alpha = 1.0 / 8.0;

void Memory::packets_received( const vector< Packet > & packets )
{
  for ( const auto &x : packets ) {
    if ( _last_tick_sent == 0 || _last_tick_received == 0 ) {
      _last_tick_sent = x.tick_sent;
      _last_tick_received = x.tick_received;
    } else {
      _rec_send_ewma = (1 - alpha) * _rec_send_ewma + alpha * (x.tick_sent - _last_tick_sent);
      _rec_rec_ewma = (1 - alpha) * _rec_rec_ewma + alpha * (x.tick_received - _last_tick_received);
      _last_tick_sent = x.tick_sent;
      _last_tick_received = x.tick_received;
    }
  }
}

string Memory::str( void ) const
{
  char tmp[ 64 ];
  snprintf( tmp, 64, "sewma=%f, rewma=%f", _rec_send_ewma, _rec_rec_ewma );
  return tmp;
}

const Memory & MAX_MEMORY( void )
{
  static const Memory max_memory( { 256, 256 } );
  return max_memory;
}
