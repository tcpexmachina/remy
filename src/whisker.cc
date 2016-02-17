#include <cassert>
#include <cmath>
#include <algorithm>
#include <boost/functional/hash.hpp>

#include "whisker.hh"

using namespace std;

Whisker::Whisker( const unsigned int s_window_increment, const double s_window_multiple, const double s_intersend, const MemoryRange & s_domain )
  : Action(s_domain),
    _window_increment( s_window_increment ),
    _window_multiple( s_window_multiple ),
    _intersend( s_intersend )
{
}

Whisker::Whisker( const Whisker & other )
  : Action( other ),
    _window_increment( other._window_increment ),
    _window_multiple( other._window_multiple ),
    _intersend( other._intersend )
{
}

Whisker::Whisker( const RemyBuffers::Whisker & dna )
  : Action( dna.domain() ), 
    _window_increment( dna.window_increment() ),
    _window_multiple( dna.window_multiple() ),
    _intersend( dna.intersend() )
{
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

vector< Whisker > Whisker::next_generation( bool optimize_window_increment, bool optimize_window_multiple, bool optimize_intersend ) const
{
  vector< Whisker> ret;

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

string Whisker::str( const unsigned int total ) const
{
  char tmp[ 256 ];
  snprintf( tmp, 256, "{%s} gen=%u usage=%.4f => (win=%d + %f * win, intersend=%f)",
	    _domain.str().c_str(), _generation, double( _domain.count() ) / double( total ), _window_increment, _window_multiple, _intersend );
  return tmp;
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
