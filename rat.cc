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
    _whiskers.emplace_back( i * 20.0 );
  }
}

const typename Rat::Whisker & Rat::Whiskers::use_whisker( const Rat::Memory & _memory )
{
  unsigned int index = _memory.last_delay / 20.0;

  Whisker & ret( index >= _whiskers.size() ? _whiskers.back() : _whiskers[ index ] );

  ret.use();

  return ret;
}

Rat::Whisker::Whisker( const double & s_representative_value )
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
