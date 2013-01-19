#include <vector>

#include "memory.hh"

using namespace std;

bool Memory::operator==( const Memory & other ) const
{
  return _data == other._data;
}

void Memory::packets_received( const vector< Packet > & packets )
{
  if ( packets.empty() ) {
    return;
  }

  _data[ LAST_DELAY ] = packets.back().tick_received - packets.back().tick_sent;
}

string Memory::str( void ) const
{
  char tmp[ 64 ];
  snprintf( tmp, 64, "ld=%u lw=%u", _data[ LAST_DELAY ], _data[ LAST_WINDOW ] );
  return tmp;
}

const Memory & MAX_MEMORY( void )
{
  static const Memory max_memory( { 1024 /* delay */, 1024 /* window */ } );
  return max_memory;
}
