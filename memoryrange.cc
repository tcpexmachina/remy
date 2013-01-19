#include "memoryrange.hh"

using namespace std;
using namespace boost::accumulators;

std::vector< MemoryRange > MemoryRange::bisect( void ) const
{
  vector< MemoryRange > ret { *this };

  /* bisect in each axis */
  for ( unsigned int i = 0; i < Memory::datasize(); i++ ) {
    vector< MemoryRange > doubled;
    for ( auto &x : ret ) {
      /* make midpoint data vector on this axis */
      auto midpoint_data( x._lower.data() );
      midpoint_data[ i ] = ( x._lower.data()[ i ] + x._upper.data()[ i ] ) / 2;
      Memory midpoint( midpoint_data );
      doubled.emplace_back( _lower, midpoint );
      doubled.emplace_back( midpoint, _upper );
    }
    ret = doubled;
  }

  return ret;
}

Memory MemoryRange::range_median( void ) const
{
  auto median_data( _lower.data() );
  for ( unsigned int i = 0; i < Memory::datasize(); i++ ) {
    median_data[ i ] = (_lower.data()[ i ] + _upper.data()[ i ]) / 2;
  }
  return Memory( median_data );
}

Memory MemoryRange::query_median( void ) const
{
  auto ret( _lower.data() );
  for ( unsigned int i = 0; i < Memory::datasize(); i++ ) {
    ret[ i ] = median( _acc[ i ] );
  }

  return ret;
}

bool MemoryRange::contains( const Memory & query ) const
{
  for ( unsigned int i = 0; i < Memory::datasize(); i++ ) {
    if ( (query.data()[ i ] < _lower.data()[ i ])
	 || (query.data()[ i ] >= _upper.data()[ i ]) ) { /* half-open range */
      return false;
    }
  }

  /* log it */
  for ( unsigned int i = 0; i < Memory::datasize(); i++ ) {
    _acc[ i ]( query.data()[ i ] );
  }

  return true;
}

bool MemoryRange::operator==( const MemoryRange & other ) const
{
  return (_lower == other._lower) && (_upper == other._upper); /* ignore median estimator for now */
}
