#include <vector>
#include <assert.h>

#include "memory.hh"

using namespace std;

static const unsigned int DELAY_BINSIZE = 20;
static const unsigned int WINDOW_BINSIZE = 10;

static const unsigned int NUM_DELAY_BINS = 10;
static const unsigned int NUM_WINDOW_BINS = 12;

vector< Memory > Memory::all_memories( void )
{
  vector< Memory > ret;
  for ( unsigned int i = 0; i < NUM_DELAY_BINS; i++ ) {
    for ( unsigned int j = 0; j < NUM_WINDOW_BINS; j++ ) {
      Memory new_mem;
      new_mem._last_delay = i * DELAY_BINSIZE;
      new_mem._last_window = j * WINDOW_BINSIZE;
      ret.push_back( new_mem );
    }
  }
  return ret;
}

unsigned int Memory::bin( const unsigned int max_val ) const
{
  unsigned int delay_index = _last_delay / DELAY_BINSIZE;
  unsigned int window_index = _last_window / WINDOW_BINSIZE;

  delay_index = min( delay_index, NUM_DELAY_BINS - 1 );
  window_index = min( window_index, NUM_WINDOW_BINS - 1 );

  unsigned int ret = delay_index * NUM_WINDOW_BINS + window_index;
  
  assert( ret <= max_val );

  return ret;
}

bool Memory::operator==( const Memory & other ) const
{
  return (_last_delay == other._last_delay) && (_last_window == other._last_window);
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
  snprintf( tmp, 64, "ld=%d lw=%d", _last_delay, _last_window );
  return tmp;
}

