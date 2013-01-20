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
      auto ersatz_lower( x._lower.data() ), ersatz_upper( x._upper.data() );
      //      ersatz_lower[ i ] = ersatz_upper[ i ] = (x._lower.data()[ i ] + x._upper.data()[ i ]) / 2;
      ersatz_lower[ i ] = ersatz_upper[ i ] = median( _acc[ i ] );

      if ( x._lower.data() == ersatz_upper ) {
	/* try range midpoint instead */
	ersatz_lower[ i ] = ersatz_upper[ i ] = (x._lower.data()[ i ] + x._upper.data()[ i ]) / 2;
      }

      if ( x._lower.data() == ersatz_upper ) {
	assert( !(ersatz_lower == x._upper.data()) );
	assert( x._lower.data() == ersatz_lower );
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

  return true;
}

void MemoryRange::track( const Memory & query ) const
{
  /* log it */
  for ( unsigned int i = 0; i < Memory::datasize(); i++ ) {
    _acc[ i ]( query.data()[ i ] );
  }
}

bool MemoryRange::operator==( const MemoryRange & other ) const
{
  return (_lower == other._lower) && (_upper == other._upper); /* ignore median estimator for now */
}

string MemoryRange::str( void ) const
{
  char tmp[ 128 ];
  snprintf( tmp, 128, "(lo=<%s>, hi=<%s>)",
	    _lower.str().c_str(),
	    _upper.str().c_str() );
  return tmp;
}
