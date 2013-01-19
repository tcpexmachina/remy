#include <assert.h>
#include <math.h>
#include <algorithm>

#include "whisker.hh"

using namespace std;

static const unsigned int MAX_WINDOW = 256;
static const unsigned int DEFAULT_WINDOW = 1;

Whiskers::Whiskers()
  : _children(),
    _leaf( 1, Whisker( DEFAULT_WINDOW, MemoryRange( Memory(), MAX_MEMORY() ) ) )
{
}

void Whiskers::reset_counts( void )
{
  for ( auto &x : _leaf ) {
    x.reset_count();
  }

  for ( auto &x : _children ) {
    x.reset_counts();
  }
}

bool Whisker::operator==( const Whisker & other ) const
{
  return (_generation == other._generation) && (_window == other._window) && (_domain == other._domain); /* ignore count for now */
}

const Whisker & Whiskers::use_whisker( const Memory & _memory ) const
{
  const Whisker * ret( whisker( _memory ) );
  assert( ret );

  ret->use();
  return *ret;
}

const Whisker * Whiskers::whisker( const Memory & _memory ) const
{
  if ( !_leaf.empty() ) {
    assert( _children.empty() );
    assert( _leaf.front().domain().contains( _memory ) );
    return &_leaf[ 0 ];
  }

  /* need to descend */
  for ( auto &x : _children ) {
    auto ret( x.whisker( _memory ) );
    if ( ret ) {
      return ret;
    }
  }

  return nullptr;
}

Whisker::Whisker( const unsigned int s_window, const MemoryRange & s_domain )
  : _generation( 0 ),
    _window( s_window ),
    _count( 0 ),
    _domain( s_domain )
{
}

Whisker::Whisker( const Whisker & other )
  : _generation( other._generation ),
    _window( other._window ),
    _count( 0 ),
    _domain( other._domain )
{
}

vector< Whisker > Whisker::next_generation( void ) const
{
  vector< Whisker > ret;

  /* generate all window sizes */
  Whisker copy( *this );
  copy._generation++;
  ret.push_back( copy );

  for ( unsigned int i = 1; i <= MAX_WINDOW ; i *= 2 ) {
    Whisker new_whisker( *this );
    new_whisker._generation++;

    if ( _window + i <= MAX_WINDOW ) {
      new_whisker._window = _window + i;
      ret.push_back( new_whisker );
    }

    if ( _window > i ) {
      new_whisker._window = _window - i;
      ret.push_back( new_whisker );
    }
  }

  return ret;
}

const Whisker * Whiskers::most_used( const unsigned int max_generation ) const
{
  if ( !_leaf.empty() ) {
    if ( _leaf.front().generation() <= max_generation ) {
      return &_leaf[ 0 ];
    }
    return nullptr;
  }

  /* recurse */
  unsigned int count_max = 0;
  const Whisker * ret( nullptr );

  for ( auto &x : _children ) {
    const Whisker * candidate( x.most_used( max_generation ) );
    if ( candidate
	 && (candidate->generation() <= max_generation)
	 && (candidate->count() >= count_max) ) {
      ret = candidate;
      count_max = candidate->count();
    }
  }

  return ret;
}

void Whiskers::promote( const unsigned int generation )
{
  for ( auto &x : _leaf ) {
    x.promote( generation );
  }

  for ( auto &x : _children ) {
    x.promote( generation );
  }
}

void Whisker::promote( const unsigned int generation )
{
  _generation = min( _generation, generation );
}

bool Whiskers::replace( const Whisker & w )
{
  if ( !_leaf.empty() ) {
    if ( w.domain() == _leaf.front().domain() ) {
      _leaf.front() = w;
      return true;
    }

    return false;
  }

  for ( auto &x : _children ) {
    if ( x.replace( w ) ) {
      return true;
    }
  }

  return false;
}

string Whisker::str( void ) const
{
  char tmp[ 128 ];
  snprintf( tmp, 128, "{%s} gen=%u ct=%u => win=%u",
	    _domain.str().c_str(), _generation, _count, _window );
  return tmp;
}

string Whiskers::str( void ) const
{

  if ( !_leaf.empty() ) {
    assert( _children.empty() );
    char tmp[ 128 ];
    snprintf( tmp, 128, "[%s]", _leaf.front().str().c_str() );
    return tmp;
  }

  string ret;

  for ( auto &x : _children ) {
    ret += x.str();
  }

  return ret;
}
