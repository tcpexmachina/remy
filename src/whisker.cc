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

Whisker::Whisker( const unsigned int s_window_increment, const double s_window_multiple, const double s_intersend, const MemoryRange & s_domain )
  : _generation( 0 ),
    _window_increment( s_window_increment ),
    _window_multiple( s_window_multiple ),
    _intersend( s_intersend ),
    _domain( s_domain )
{
}

Whisker::Whisker( const Whisker & other )
  : _generation( other._generation ),
    _window_increment( other._window_increment ),
    _window_multiple( other._window_multiple ),
    _intersend( other._intersend ),
    _domain( other._domain )
{
}

template < typename T >
bool Whisker::OptimizationSetting< T >::eligible_value( const T & value ) const
{
  return value >= min_value and value <= max_value;
}

template < typename T >
vector< T > Whisker::OptimizationSetting< T >::alternatives( const T & value, bool active ) const
{
  if ( !eligible_value( value ) ) {
    printf("Ineligible value: %s is not between %s and %s\n", to_string( value ).c_str(), to_string( min_value ).c_str(), to_string( max_value ).c_str());
    assert(false);
  }

  vector< T > ret( 1, value );

  /* If this axis isn't active, return only the current value. */
  if (!active) return ret;

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

vector< Whisker > Whisker::next_generation( bool optimize_window_increment, bool optimize_window_multiple, bool optimize_intersend ) const
{
  vector< Whisker > ret;

  auto window_increment_alternatives = get_optimizer().window_increment.alternatives( _window_increment, optimize_window_increment );
  auto window_multiple_alternatives = get_optimizer().window_multiple.alternatives( _window_multiple, optimize_window_multiple );
  auto intersend_alternatives = get_optimizer().intersend.alternatives( _intersend, optimize_intersend );

  printf("Alternatives: window increment %u to %u, window multiple %f to %f, intersend %f to %f\n",
         *(min_element(window_increment_alternatives.begin(), window_increment_alternatives.end())),
         *(max_element(window_increment_alternatives.begin(), window_increment_alternatives.end())),
         *(min_element(window_multiple_alternatives.begin(), window_multiple_alternatives.end())),
         *(max_element(window_multiple_alternatives.begin(), window_multiple_alternatives.end())),
         *(min_element(intersend_alternatives.begin(), intersend_alternatives.end())),
         *(max_element(intersend_alternatives.begin(), intersend_alternatives.end()))
  );

  for ( const auto & alt_window : window_increment_alternatives ) {
    for ( const auto & alt_multiple : window_multiple_alternatives ) {
      for ( const auto & alt_intersend : intersend_alternatives ) {
        Whisker new_whisker { *this };
        new_whisker._generation++;

	new_whisker._window_increment = alt_window;
	new_whisker._window_multiple = alt_multiple;
	new_whisker._intersend = alt_intersend;

	new_whisker.round();

	ret.push_back( new_whisker );
      }
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
  snprintf( tmp, 256, "{%s} gen=%u usage=%.4f => (win=%d + %f * win, intersend=%f)",
	    _domain.str().c_str(), _generation, double( _domain.count() ) / double( total ), _window_increment, _window_multiple, _intersend );
  return tmp;
}

RemyBuffers::Whisker Whisker::DNA( void ) const
{
  RemyBuffers::Whisker ret;

  ret.set_window_increment( _window_increment );
  ret.set_window_multiple( _window_multiple );
  ret.set_intersend( _intersend );
  ret.mutable_domain()->CopyFrom( _domain.DNA() );

  return ret;
}

Whisker::Whisker( const RemyBuffers::Whisker & dna )
  : _generation( 0 ),
    _window_increment( dna.window_increment() ),
    _window_multiple( dna.window_multiple() ),
    _intersend( dna.intersend() ),
    _domain( dna.domain() )
{
}

void Whisker::round( void )
{
  _window_multiple = (1.0/10000.0) * int( 10000 * _window_multiple );
  _intersend = (1.0/10000.0) * int( 10000 * _intersend );
}

size_t hash_value( const Whisker & whisker )
{
  size_t seed = 0;
  boost::hash_combine( seed, whisker._window_increment );
  boost::hash_combine( seed, whisker._window_multiple );
  boost::hash_combine( seed, whisker._intersend );
  boost::hash_combine( seed, whisker._domain );

  return seed;
}
