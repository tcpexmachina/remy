#include <boost/functional/hash.hpp>

#include "memoryrange.hh"

using namespace std;
using namespace boost::accumulators;

std::vector< MemoryRange > MemoryRange::bisect( void ) const
{
  vector< MemoryRange > ret { *this };

  /* bisect in each active axis */
  for ( auto & i : _active_axis ) {
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
	doubled.emplace_back( x._lower, ersatz_upper, x._active_axis );
	doubled.emplace_back( ersatz_lower, x._upper, x._active_axis );
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
  for ( auto & i : _active_axis ) {
    median_data.mutable_field( i ) = (_lower.field( i ) + _upper.field( i )) / 2;
  }
  return median_data;
}

bool MemoryRange::contains( const Memory & query ) const
{
  for ( auto & i : _active_axis ) {
    if (!((query.field(i) >= _lower.field(i)) && (query.field(i) < _upper.field(i)))) { return false; }
  }
  return true;
}

void MemoryRange::track( const Memory & query ) const
{
  /* log it */
  for ( auto & i : _active_axis ) {
    _acc[ i ]( query.field( i ) );
  }
}

bool MemoryRange::operator==( const MemoryRange & other ) const
{
  for ( auto & i : _active_axis ) {
    if (!((_lower.field(i) == other._lower.field(i)) && (_upper.field(i) == other._upper.field(i)))) { return false; }
  }
  return true;
}

string MemoryRange::str( void ) const
{
  char tmp[ 256 ];
  strcpy( tmp, "(lo=< ");
  for ( auto & i : _active_axis ) {
    strcat( tmp, _lower.str( i ).c_str() );
  }
  strcat( tmp, ">, hi=< ");
  for ( auto & i : _active_axis ) {
    strcat( tmp, _upper.str( i ).c_str() );
  }
  strcat( tmp, ">)");
  return tmp;
}

RemyBuffers::MemoryRange MemoryRange::DNA( void ) const
{
  RemyBuffers::MemoryRange ret;

  ret.mutable_lower()->CopyFrom( _lower.DNA() );
  ret.mutable_upper()->CopyFrom( _upper.DNA() );
  for (auto & x : _active_axis) {
    ret.add_active_axis(x);
  }
  
  return ret;
}

MemoryRange::MemoryRange( const RemyBuffers::MemoryRange & dna )
  : _lower( true, dna.lower() ),
    _upper( false, dna.upper() ),
    _active_axis( ), 
    _acc( Memory::datasize ),
    _count( 0 )
{
  for (auto & x : dna.active_axis()) {
    _active_axis.push_back( (Axis) x );
  }
  /* Backward compatibility: old remys don't have active axis; set to default */
  if ( _active_axis.empty() ) {
    _active_axis = vector<Axis>( { RemyBuffers::MemoryRange::SEND_EWMA, RemyBuffers::MemoryRange::REC_EWMA, 
                                   RemyBuffers::MemoryRange::RTT_RATIO, RemyBuffers::MemoryRange::SLOW_REC_EWMA } );
  }
}

size_t hash_value( const MemoryRange & mr )
{
  size_t seed = 0;
  boost::hash_combine( seed, mr._lower );
  boost::hash_combine( seed, mr._upper );

  return seed;
}
