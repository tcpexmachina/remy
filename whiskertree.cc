#include <assert.h>
#include <math.h>
#include <algorithm>

#include "whiskertree.hh"

using namespace std;

WhiskerTree::WhiskerTree()
  : _domain( Memory(), MAX_MEMORY() ),
    _children(),
    _leaf( 1, Whisker( DEFAULT_WINDOW, DEFAULT_MULTIPLE, MIN_INTERSEND, _domain ) )
{
}

WhiskerTree::WhiskerTree( const Whisker & whisker, const bool bisect )
  : _domain( whisker.domain() ),
    _children(),
    _leaf()
{
  if ( !bisect ) {
    _leaf.push_back( whisker );
  } else {
    for ( auto &x : whisker.bisect() ) {
      _children.push_back( WhiskerTree( x, false ) );
    }
  }
}

void WhiskerTree::reset_counts( void )
{
  if ( is_leaf() ) {
    _leaf.front().reset_count();
  } else {
    for ( auto &x : _children ) {
      x.reset_counts();
    }
  }
}

const Whisker & WhiskerTree::use_whisker( const Memory & _memory, const bool track ) const
{
  const Whisker * ret( whisker( _memory ) );

  if ( !ret ) {
    fprintf( stderr, "ERROR: No whisker found for %s\n", _memory.str().c_str() );
    exit( 1 );
  }

  assert( ret );

  ret->use();

  if ( track ) {
    ret->domain().track( _memory );
  }

  return *ret;
}

const Whisker * WhiskerTree::whisker( const Memory & _memory ) const
{
  if ( !_domain.contains( _memory ) ) {
    return nullptr;
  }

  if ( is_leaf() ) {
    return &_leaf[ 0 ];
  }

  /* need to descend */
  for ( auto &x : _children ) {
    auto ret( x.whisker( _memory ) );
    if ( ret ) {
      return ret;
    }
  }

  assert( false );
}

const Whisker * WhiskerTree::most_used( const unsigned int max_generation ) const
{
  if ( is_leaf() ) {
    if ( (_leaf.front().generation() <= max_generation)
	 && (_leaf.front().count() > 0) ) {
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

void WhiskerTree::promote( const unsigned int generation )
{
  if ( is_leaf() ) {
    assert( _leaf.size() == 1 );
    assert( _children.empty() );
    _leaf.front().promote( generation );
  } else {
    for ( auto &x : _children ) {
      x.promote( generation );
    }
  }
}

bool WhiskerTree::replace( const Whisker & w )
{
  if ( !_domain.contains( w.domain().range_median() ) ) {
    return false;
  }

  if ( is_leaf() ) {
    assert( w.domain() == _leaf.front().domain() );
    _leaf.front() = w;
    return true;
  }

  for ( auto &x : _children ) {
    if ( x.replace( w ) ) {
      return true;
    }
  }

  assert( false );
}

bool WhiskerTree::replace( const Whisker & src, const WhiskerTree & dst )
{
  if ( !_domain.contains( src.domain().range_median() ) ) {
    return false;
  }
 
  if ( is_leaf() ) {
    assert( src.domain() == _leaf.front().domain() );
    /* convert from leaf to interior node */
    *this = dst;
    return true;
  }

  for ( auto &x : _children ) {
    if ( x.replace( src, dst ) ) {
      return true;
    }
  }

  assert( false );
}

string WhiskerTree::str( void ) const
{
  if ( is_leaf() ) {
    assert( _children.empty() );
    string tmp = string( "[" ) + _leaf.front().str().c_str() + "]";
    return tmp;
  }

  string ret;

  for ( auto &x : _children ) {
    ret += x.str();
  }

  return ret;
}

unsigned int WhiskerTree::num_children( void ) const
{
  if ( is_leaf() ) {
    assert( _leaf.size() == 1 );
    return 1;
  }

  return _children.size();
}

bool WhiskerTree::is_leaf( void ) const
{
  return !_leaf.empty();
}
