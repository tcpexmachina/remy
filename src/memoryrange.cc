#include <boost/functional/hash.hpp>

#include "memoryrange.hh"

using namespace std;
using namespace boost::accumulators;

std::vector< MemoryRange > MemoryRange::bisect( void ) const
{
  vector< MemoryRange > ret { *this };

  /* bisect in each axis */
  for ( unsigned int i = 0; i < Memory::datasize; i++ ) {
    vector< MemoryRange > doubled;
    for ( const auto &x : ret ) {
      auto ersatz_lower( x._lower ), ersatz_upper( x._upper );
      ersatz_lower.mutable_field( i ) = ersatz_upper.mutable_field( i ) = median( _acc[ i ] );

      if ( x._lower == ersatz_upper ) {
	/* try range midpoint instead */
	ersatz_lower.mutable_field( i ) = ersatz_upper.mutable_field( i ) = (x._lower.field( i ) + x._upper.field( i )) / 2;
      }

      if ( x._lower == ersatz_upper ) {
	assert( !(ersatz_lower == x._upper) );
	assert( x._lower == ersatz_lower );
	/* cannot double on this axis */
	doubled.push_back( x );
      } else {
	doubled.emplace_back( x._lower, ersatz_upper );
	doubled.emplace_back( ersatz_lower, x._upper );
      }
    }

    ret = doubled;
  }

  assert( !ret.empty());

  return ret;
}

Memory MemoryRange::range_median( void ) const
{
  Memory median_data( _lower );
  for ( unsigned int i = 0; i < Memory::datasize; i++ ) {
    median_data.mutable_field( i ) = (_lower.field( i ) + _upper.field( i )) / 2;
  }
  return median_data;
}

bool MemoryRange::contains( const Memory & query ) const
{
  return (query >= _lower) && (query < _upper);
}

void MemoryRange::track( const Memory & query ) const
{
  /* log it */
  for ( unsigned int i = 0; i < Memory::datasize; i++ ) {
    _acc[ i ]( query.field( i ) );
  }
}

bool MemoryRange::operator==( const MemoryRange & other ) const
{
  return (_lower == other._lower) && (_upper == other._upper); /* ignore median estimator for now */
}

string MemoryRange::str( void ) const
{
  char tmp[ 256 ];
  snprintf( tmp, 256, "(lo=<%s>, hi=<%s>)",
	    _lower.str().c_str(),
	    _upper.str().c_str() );
  return tmp;
}

RemyBuffers::MemoryRange MemoryRange::DNA( void ) const
{
  RemyBuffers::MemoryRange ret;

  ret.mutable_lower()->CopyFrom( _lower.DNA() );
  ret.mutable_upper()->CopyFrom( _upper.DNA() );

  return ret;
}

MemoryRange::MemoryRange( const RemyBuffers::MemoryRange & dna )
  : _lower( true, dna.lower() ),
    _upper( false, dna.upper() ),
    _acc( Memory::datasize ),
    _count( 0 )
{}

size_t hash_value( const MemoryRange & mr )
{
  size_t seed = 0;
  boost::hash_combine( seed, mr._lower );
  boost::hash_combine( seed, mr._upper );

  return seed;
}
