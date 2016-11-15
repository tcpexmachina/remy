#include <numeric>

#include "fintree.hh"

using namespace std;

FinTree::FinTree()
  : _domain( Memory(), MAX_MEMORY(), { RemyBuffers::MemoryRange::RTT_DIFF } ),
    _children(),
    _leaf( 1, Fin( _domain ) )
{
}

FinTree::FinTree( const Fin & fin, const bool bisect )
  : _domain( fin.domain() ),
    _children(),
    _leaf()
{
  if ( !bisect ) {
    _leaf.push_back( fin );
  } else {
    for ( auto &x : fin.bisect<Fin>() ) {
      _children.push_back( FinTree( x, false ) );
    }
  }
}

void FinTree::reset_counts( void )
{
  if ( is_leaf() ) {
    _leaf.front().reset_count();
  } else {
    for ( auto &x : _children ) {
      x.reset_counts();
    }
  }
}

const Fin & FinTree::use_fin( const Memory & _memory, const bool track ) const
{
  const Fin * ret( fin( _memory ) );

  if ( !ret ) {
    fprintf( stderr, "ERROR: No Fin found for %s\n", _memory.str().c_str() );
    exit( 1 );
  }

  assert( ret );

  ret->use();

  if ( track ) {
    ret->domain().track( _memory );
  }

  return *ret;
}

const Fin * FinTree::fin( const Memory & _memory ) const
{
  if ( !_domain.contains( _memory ) ) {
    return nullptr;
  }

  if ( is_leaf() ) {
    return &_leaf[ 0 ];
  }

  /* need to descend */
  for ( auto &x : _children ) {
    auto ret( x.fin( _memory ) );
    if ( ret ) {
      return ret;
    }
  }

  assert( false );
  return nullptr;
}

const Fin * FinTree::most_used( const unsigned int max_generation ) const
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
  const Fin * ret( nullptr );

  for ( auto &x : _children ) {
    const Fin * candidate( x.most_used( max_generation ) );
    if ( candidate
	 && (candidate->generation() <= max_generation)
	 && (candidate->count() >= count_max) ) {
      ret = candidate;
      count_max = candidate->count();
    }
  }

  return ret;
}

void FinTree::reset_generation( void )
{
  if ( is_leaf() ) {
    assert( _leaf.size() == 1 );
    assert( _children.empty() );
    _leaf.front().demote( 0 );
  } else {
    for ( auto &x : _children ) {
      x.reset_generation();
    }
  }
}

void FinTree::promote( const unsigned int generation )
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

bool FinTree::replace( const Fin & w )
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
  return false;
}

bool FinTree::replace( const Fin & src, const FinTree & dst )
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
  return false;
}

unsigned int FinTree::total_fin_queries( void ) const
{
  if ( is_leaf() ) {
    assert( _children.empty() );
    return _leaf.front().domain().count();
  }

  return accumulate( _children.begin(),
		     _children.end(),
		     0,
		     []( const unsigned int sum, 
			 const FinTree & x )
		     { return sum + x.total_fin_queries(); } );
}

string FinTree::str() const
{
  return str( total_fin_queries() );
}

string FinTree::str( const unsigned int total ) const
{
  if ( is_leaf() ) {
    assert( _children.empty() );
    string tmp = string( "[" ) + _leaf.front().str( total ) + "]";
    return tmp;
  }

  string ret;

  for ( const auto &x : _children ) {
    ret += x.str( total );
  }

  return ret;
}

unsigned int FinTree::num_children( void ) const
{
  if ( is_leaf() ) {
    assert( _leaf.size() == 1 );
    return 1;
  }

  return _children.size();
}

bool FinTree::is_leaf( void ) const
{
  return !_leaf.empty();
}

RemyBuffers::FinTree FinTree::DNA( void ) const
{
  RemyBuffers::FinTree ret;

  /* set domain */
  ret.mutable_domain()->CopyFrom( _domain.DNA() );

  /* set children */
  if ( is_leaf() ) {
    ret.mutable_leaf()->CopyFrom( _leaf.front().DNA() );
  } else {
    for ( auto &x : _children ) {
      RemyBuffers::FinTree *child = ret.add_children();
      *child = x.DNA();
    }
  }

  return ret;
}

FinTree::FinTree( const RemyBuffers::FinTree & dna )
  : _domain( dna.domain() ),
    _children(),
    _leaf()
{
  if ( dna.has_leaf() ) {
    assert( dna.children_size() == 0 );
    _leaf.emplace_back( dna.leaf() );
  } else {
    assert( dna.children_size() > 0 );
    for ( const auto &x : dna.children() ) {
      _children.emplace_back( x );
    }
  }
}
