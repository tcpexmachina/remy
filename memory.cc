#include <vector>

#include "memory.hh"

using namespace std;

bool Memory::operator==( const Memory & other ) const
{
  return _last_delay == other._last_delay;
}

void Memory::packets_received( const vector< Packet > & packets )
{
  if ( packets.empty() ) {
    return;
  }

  _last_delay = packets.back().tick_received - packets.back().tick_sent;
}

string Memory::str( void ) const
{
  char tmp[ 64 ];
  snprintf( tmp, 64, "ld=%u", _last_delay );
  return tmp;
}

const Memory & MAX_MEMORY( void )
{
  static const Memory max_memory( { 1024 /* delay */ } );
  return max_memory;
}
