#include <cassert>
#include <cmath>
#include <algorithm>
#include <boost/functional/hash.hpp>

#include "whisker.hh"

using namespace std;

vector< Whisker > Whisker::bisect( void ) const
{
  vector< Whisker > ret;
  for ( auto &x : _domain.bisect() ) {
    Whisker new_whisker( *this );
    new_whisker._domain = x;
    ret.push_back( new_whisker );
  }
  return ret;
}

Whisker::Whisker( const double s_intersend_increment, const double s_intersend_multiple, const MemoryRange & s_domain )
  : _generation( 0 ),
    _intersend_increment( s_intersend_increment ),
    _intersend_multiple( s_intersend_multiple ),
    _domain( s_domain )
{
}

Whisker::Whisker( const Whisker & other )
  : _generation( other._generation ),
    _intersend_increment( other._intersend_increment ),
    _intersend_multiple( other._intersend_multiple ),
    _domain( other._domain )
{
}

template < typename T >
bool Whisker::OptimizationSetting< T >::eligible_value( const T & value ) const
{
  return value >= min_value and value <= max_value;
}

template < typename T >
vector< T > Whisker::OptimizationSetting< T >::alternatives( const T & value ) const
{
  assert( eligible_value( value ) );

  vector< T > ret( 1, value );

  for ( T proposed_change = min_change;
	proposed_change <= max_change;
	proposed_change *= multiplier ) {
    /* explore positive change */
    const T proposed_value_up = value + proposed_change;
    const T proposed_value_down = value - proposed_change;

    if ( eligible_value( proposed_value_up ) ) {
      ret.push_back( proposed_value_up );
    }

    if ( eligible_value( proposed_value_down ) ) {
      ret.push_back( proposed_value_down );
    }
  }

  return ret;
}

vector< Whisker > Whisker::next_generation( void ) const
{
  vector< Whisker > ret;

  for ( const auto & alt_increment : get_optimizer().intersend_increment.alternatives( _intersend_increment ) ) {
    for ( const auto & alt_multiple : get_optimizer().intersend_multiple.alternatives( _intersend_multiple ) ) {
      Whisker new_whisker { *this };
      new_whisker._generation++;
      
      new_whisker._intersend_increment = alt_increment;
      new_whisker._intersend_multiple = alt_multiple;
      
      new_whisker.round();
      
      ret.push_back( new_whisker );
    }
  }

  return ret;
}

void Whisker::promote( const unsigned int generation )
{
  _generation = max( _generation, generation );
}

string Whisker::str( const unsigned int total ) const
{
  char tmp[ 256 ];
  snprintf( tmp, 256, "{%s} gen=%u usage=%.4f => (intersend=%f + %f * intersend)",
	    _domain.str().c_str(), _generation, double( _domain.count() ) / double( total ), _intersend_increment, _intersend_multiple );
  return tmp;
}

RemyBuffers::Whisker Whisker::DNA( void ) const
{
  RemyBuffers::Whisker ret;

  ret.set_intersend_increment( _intersend_increment );
  ret.set_intersend_multiple( _intersend_multiple );
  ret.mutable_domain()->CopyFrom( _domain.DNA() );

  return ret;
}

Whisker::Whisker( const RemyBuffers::Whisker & dna )
  : _generation( 0 ),
    _intersend_increment( dna.intersend_increment() ),
    _intersend_multiple( dna.intersend_multiple() ),
    _domain( dna.domain() )
{
}

void Whisker::round( void )
{
  _intersend_increment = (1.0/10000.0) * int( 10000 * _intersend_increment );
  _intersend_multiple = (1.0/10000.0) * int( 10000 * _intersend_multiple );
}

size_t hash_value( const Whisker & whisker )
{
  size_t seed = 0;
  boost::hash_combine( seed, whisker._intersend_increment );
  boost::hash_combine( seed, whisker._intersend_multiple );
  boost::hash_combine( seed, whisker._domain );

  return seed;
}
