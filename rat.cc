#include <assert.h>
#include <math.h>

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

vector< Rat::Memory > Rat::Memory::all_memories( void )
{
  vector< Memory > ret;
  for ( int i = 0; i < 16; i++ ) {
    Memory new_mem;
    new_mem._last_delay = i * binsize();
    ret.push_back( new_mem );
  }
  return ret;
}

Rat::Whiskers::Whiskers()
  : _whiskers()
{
  auto default_memories( Memory::all_memories() );

  for ( auto &x : default_memories ) {
    _whiskers.emplace_back( x );
  }
}

void Rat::Whiskers::reset_counts( void )
{
  for ( auto &x : _whiskers ) {
    x.reset_count();
  }
}

bool Rat::Memory::operator==( const Memory & other ) const
{
  return _last_delay == other._last_delay;
}

bool Rat::Whisker::operator==( const Whisker & other ) const
{
  return (_generation == other._generation) && (_window == other._window) && (_count == other._count) && (_representative_value == other._representative_value);
}

const typename Rat::Whisker & Rat::Whiskers::use_whisker( const Rat::Memory & _memory )
{
  const Rat::Whisker & ret( whisker( _memory ) );
  ret.use();
  return ret;
}

unsigned int Rat::Memory::bin( const unsigned int max_val ) const
{
  assert( _last_delay >= 0 );

  unsigned int index( _last_delay / binsize() );
  
  return min( max_val, index );
}

const typename Rat::Whisker & Rat::Whiskers::whisker( const Rat::Memory & _memory ) const
{
  unsigned int index( _memory.bin( _whiskers.size() - 1) );

  const Rat::Whisker & ret( _whiskers[ index ] );

  unsigned int loopback_index( ret.representative_value().bin( _whiskers.size() - 1 ) );

  assert( index == loopback_index );

  return ret;
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

  _last_delay = packets.back().tick_received - packets.back().tick_sent;

  assert( _last_delay >= 0 );
}

vector< Rat::Whisker > Rat::Whisker::next_generation( void ) const
{
  vector< Rat::Whisker > ret;

  /* generate all window sizes */
  for ( unsigned int i = 20; i <= 120; i += 10 ) {
    Whisker new_whisker( _representative_value );
    new_whisker._generation = _generation + 1;
    new_whisker._window = i;
    ret.push_back( new_whisker );
  }

  return ret;
}

string Rat::Whisker::summary( void ) const
{
  char tmp[ 64 ];
  /*
  snprintf( tmp, 64, "[%s gen=%u count=%u win=%u]", _representative_value.str().c_str(),
	    _generation, _count, _window );
  */
  if ( _count > 0 ) {
    snprintf( tmp, 64, "%u", _window );
  } else {
    snprintf( tmp, 64, " " );
  }

  string stars;
  if ( _count > 0 ) {
    for ( int i = 0; i < log10( _count ); i++ ) {
      stars += string( "*" );
    }
  }

  return string( "[" ) + string( tmp ) + string( stars ) + string( "]" );
}

const Rat::Whisker * Rat::Whiskers::most_used( const unsigned int max_generation ) const
{
  unsigned int count_max = 0;

  assert( !_whiskers.empty() );

  const Rat::Whisker * ret( nullptr );

  for ( auto &x : _whiskers ) {
    if ( (x.generation() <= max_generation) && (x.count() >= count_max) ) {
      ret = &x;
      count_max = x.count();
    }
  }

  return ret;
}

void Rat::Whiskers::promote( const unsigned int generation )
{
  for ( auto &x : _whiskers ) {
    x.promote( generation );
  }
}

void Rat::Whisker::promote( const unsigned int generation )
{
  _generation = min( _generation, generation );
}

string Rat::Memory::str( void ) const
{
  char tmp[ 64 ];
  snprintf( tmp, 64, "ld=%.0f", _last_delay );
  return tmp;
}

void Rat::Whiskers::replace( const Whisker & w )
{
  unsigned int index( w.representative_value().bin( _whiskers.size() - 1) );  
  _whiskers[ index ] = w;
}
