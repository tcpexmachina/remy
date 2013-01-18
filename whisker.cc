#include <assert.h>
#include <math.h>

#include "whisker.hh"

static const unsigned int WINDOW_BINSIZE = 10;
static const unsigned int NUM_WINDOW_BINS = 12;
/* XXX */

using namespace std;

Whiskers::Whiskers()
  : _whiskers()
{
  auto default_memories( Memory::all_memories() );

  for ( auto &x : default_memories ) {
    _whiskers.emplace_back( x );
  }
}

void Whiskers::reset_counts( void )
{
  for ( auto &x : _whiskers ) {
    x.reset_count();
  }
}

bool Whisker::operator==( const Whisker & other ) const
{
  return (_generation == other._generation) && (_window == other._window) && (_count == other._count) && (_representative_value == other._representative_value);
}

const Whisker & Whiskers::use_whisker( const Memory & _memory )
{
  const Whisker & ret( whisker( _memory ) );
  ret.use();
  return ret;
}

const Whisker & Whiskers::whisker( const Memory & _memory ) const
{
  unsigned int index( _memory.bin( _whiskers.size() - 1) );

  const Whisker & ret( _whiskers[ index ] );

  unsigned int loopback_index( ret.representative_value().bin( _whiskers.size() - 1 ) );

  assert( index == loopback_index );

  return ret;
}

Whisker::Whisker( const Memory & s_representative_value )
  : _generation( 0 ),
    _window( 100 ),
    _count( 0 ),
    _representative_value( s_representative_value )
{
}

vector< Whisker > Whisker::next_generation( void ) const
{
  vector< Whisker > ret;

  /* generate all window sizes */
  for ( unsigned int i = WINDOW_BINSIZE; i <= WINDOW_BINSIZE * NUM_WINDOW_BINS; i += WINDOW_BINSIZE ) {
    Whisker new_whisker( _representative_value );
    new_whisker._generation = _generation + 1;
    new_whisker._window = i;
    ret.push_back( new_whisker );
  }

  return ret;
}

string Whisker::summary( void ) const
{
  char tmp[ 64 ];
  /*
  snprintf( tmp, 64, "[%s gen=%u count=%u win=%u]", _representative_value.str().c_str(),
	    _generation, _count, _window );
  */
  if ( _count > 0 ) {
    snprintf( tmp, 64, "%s %u", _representative_value.str().c_str(), _window );
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

const Whisker * Whiskers::most_used( const unsigned int max_generation ) const
{
  unsigned int count_max = 0;

  assert( !_whiskers.empty() );

  const Whisker * ret( nullptr );

  for ( auto &x : _whiskers ) {
    if ( (x.generation() <= max_generation) && (x.count() >= count_max) ) {
      ret = &x;
      count_max = x.count();
    }
  }

  return ret;
}

void Whiskers::promote( const unsigned int generation )
{
  for ( auto &x : _whiskers ) {
    x.promote( generation );
  }
}

void Whisker::promote( const unsigned int generation )
{
  _generation = min( _generation, generation );
}

void Whiskers::replace( const Whisker & w )
{
  unsigned int index( w.representative_value().bin( _whiskers.size() - 1 ) );  
  _whiskers[ index ] = w;
}
