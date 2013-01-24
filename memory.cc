#include <vector>
#include <assert.h>

#include "memory.hh"

using namespace std;

static const double alpha = 1.0 / 8.0;

void Memory::packets_received( const vector< Packet > & packets )
{
  for ( const auto &x : packets ) {
    const unsigned int rtt = x.tick_received - x.tick_sent;
    if ( _last_tick_sent == 0 || _last_tick_received == 0 ) {
      _last_tick_sent = x.tick_sent;
      _last_tick_received = x.tick_received;
      _min_rtt = rtt;
    } else {
      _rec_send_ewma = (1 - alpha) * _rec_send_ewma + alpha * (x.tick_sent - _last_tick_sent);
      _rec_rec_ewma = (1 - alpha) * _rec_rec_ewma + alpha * (x.tick_received - _last_tick_received);
      _last_tick_sent = x.tick_sent;
      _last_tick_received = x.tick_received;

      _min_rtt = min( _min_rtt, rtt );
      _rtt_ratio = double( rtt ) / double( _min_rtt );
      assert( _rtt_ratio >= 1.0 );
    }
  }
}

string Memory::str( void ) const
{
  char tmp[ 256 ];
  snprintf( tmp, 256, "sewma=%f, rewma=%f, rttr=%f", _rec_send_ewma, _rec_rec_ewma, _rtt_ratio );
  return tmp;
}

const Memory & MAX_MEMORY( void )
{
  static const Memory max_memory( { 1024, 1024, 1024 } );
  return max_memory;
}
