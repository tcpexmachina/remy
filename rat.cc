#include <assert.h>

#include "rat.hh"

using namespace std;

Rat::Rat( const Whiskers & s_whiskers )
  :  _whiskers( s_whiskers ),
     _memory(),
     _packets_sent( 0 ),
     _packets_received( 0 )
{
}

void Rat::packets_received( const vector< Packet > & packets ) {
  _packets_received += packets.size();
  _memory.packets_received( packets );
}

void Rat::dormant_tick( const unsigned int tickno __attribute((unused)) )
{
}

unsigned int Rat::window( const unsigned int tickno )
{
  _memory.advance_to( tickno );
  return _whiskers.use_whisker( _memory ).window();
}

Rat::Whiskers::Whiskers()
  : _whiskers()
{
  _whiskers.reserve( 16 );
  for ( int i = 0; i < 16; i++ ) {
    Memory mem;
    mem.last_delay = i * 20.0;
    _whiskers.emplace_back( mem );
  }
}

bool Rat::Memory::operator==( const Memory & other ) const
{
  return last_delay == other.last_delay;
}

bool Rat::Whisker::operator==( const Whisker & other ) const
{
  return (_generation == other._generation) && (_window == other._window) && (_count == other._count) && (_representative_value == other._representative_value);
}

const typename Rat::Whisker & Rat::Whiskers::use_whisker( const Rat::Memory & _memory )
{
  Whisker & ret( mutable_whisker( _memory ) );
  Whisker & loopback( mutable_whisker( ret.representative_value() ) );

  assert( ret == loopback );

  ret.use();

  return ret;
}

typename Rat::Whisker & Rat::Whiskers::mutable_whisker( const Rat::Memory & _memory )
{
  unsigned int index = _memory.last_delay / 20.0;

  return index >= _whiskers.size() ? _whiskers.back() : _whiskers[ index ];
}

Rat::Whisker::Whisker( const Memory & s_representative_value )
  : _generation( 0 ),
    _window( 100 ),
    _count( 0 ),
    _representative_value( s_representative_value )
{
}

void Rat::Memory::packets_received( const vector< Packet > & packets )
{
  if ( packets.empty() ) {
    return;
  }

  last_delay = packets.back().tick_received - packets.back().tick_sent;

  assert( last_delay >= 0 );
}
