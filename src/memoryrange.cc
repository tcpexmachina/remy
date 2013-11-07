#include <boost/functional/hash.hpp>

#include "memoryrange.hh"

using namespace std;
using namespace boost::accumulators;

std::vector< MemoryRange > MemoryRange::bisect( void ) const
{
  vector< MemoryRange > ret;

  /* decide which axis to bisect on */
  vector< Memory::DataType > variances( _acc.size() );
  transform( _acc.begin(), _acc.end(), variances.begin(), variance );

  unsigned int best_axis = distance( variances.begin(),
				     max_element( variances.begin(), variances.end() ) );

  assert( best_axis <= Memory::datasize );

  fprintf( stderr, "Best axis: %u (variance %f)\n", best_axis, variances.at( best_axis ) );

  /* bisect in that axis */
  auto ersatz_lower( _lower ), ersatz_upper( _upper );
  ersatz_lower.mutable_field( best_axis ) = ersatz_upper.mutable_field( best_axis ) = median( _acc[ best_axis ] );

  if ( _lower == ersatz_upper ) {
    /* try range midpoint instead */
    fprintf( stderr, "Warning, unable to bisect\n" );
    ersatz_lower.mutable_field( best_axis ) = ersatz_upper.mutable_field( best_axis ) = (_lower.field( best_axis ) + _upper.field( best_axis )) / 2;
  }

  if ( _lower == ersatz_upper ) {
    assert( !(ersatz_lower == _upper) );
    assert( _lower == ersatz_lower );
    fprintf( stderr, "Warning, still unable to bisect\n" );
    /* cannot double on this axis */
    ret.push_back( *this );
  } else {
    ret.emplace_back( _lower, ersatz_upper );
    ret.emplace_back( ersatz_lower, _upper );
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
  : _lower( dna.lower() ),
    _upper( dna.upper() ),
    _acc( Memory::datasize ),
    _count( 0 )
{
}

size_t hash_value( const MemoryRange & mr )
{
  size_t seed = 0;
  boost::hash_combine( seed, mr._lower );
  boost::hash_combine( seed, mr._upper );

  return seed;
}
