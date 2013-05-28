#include <assert.h>
#include <math.h>
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

vector< Whisker > Whisker::next_generation( void ) const
{
  /* generate all window increments */
  vector< Whisker > ret_windows;

  Whisker copy( *this );
  copy._generation++;
  ret_windows.push_back( copy );

  for ( unsigned int i = 1; i <= MAX_WINDOW_INCR; i *= 8 ) {
    Whisker new_whisker( *this );
    new_whisker._generation++;

    if ( _window_increment + i <= MAX_WINDOW ) {
      new_whisker._window_increment = _window_increment + i;
      ret_windows.push_back( new_whisker );
    }

    if ( _window_increment - i >= -MAX_WINDOW ) {
      new_whisker._window_increment = _window_increment - i;
      ret_windows.push_back( new_whisker );
    }
  }

  /* generate all window multiples */
  vector< Whisker > ret_multiples;

  for ( auto &x : ret_windows ) {
    Whisker multiple_copy( x );
    ret_multiples.push_back( multiple_copy );

    for ( double multiple_incr = MULTIPLE_INCR; multiple_incr <= MAX_MULTIPLE_INCR; multiple_incr *= 8.0 ) {
      Whisker new_whisker( x );

      if ( x._window_multiple + multiple_incr <= MAX_MULTIPLE ) {
	new_whisker._window_multiple = x._window_multiple + multiple_incr;
	ret_multiples.push_back( new_whisker );
      }

      if ( x._window_multiple - multiple_incr >= 0 ) {
	new_whisker._window_multiple = x._window_multiple - multiple_incr;
	ret_multiples.push_back( new_whisker );
      }
    }
  }

  /* generate all rates */
  vector< Whisker > ret;
  for ( auto &x : ret_multiples ) {
    Whisker intersend_copy( x );
    ret.push_back( intersend_copy );

    for ( double intersend_incr = INTERSEND_INCR; intersend_incr <= MAX_INTERSEND_INCR; intersend_incr *= 8.0 ) {
      Whisker new_whisker( x );

      if ( x._intersend + intersend_incr <= MAX_INTERSEND ) {
	new_whisker._intersend = x._intersend + intersend_incr;
	ret.push_back( new_whisker );
      }

      if ( x._intersend - intersend_incr >= MIN_INTERSEND ) {
	new_whisker._intersend = x._intersend - intersend_incr;
	ret.push_back( new_whisker );
      }
    }
  }

  for ( auto &x : ret ) {
    x.round();
  }

  return ret;
}

void Whisker::promote( const unsigned int generation )
{
  _generation = max( _generation, generation );
}

string Whisker::str( void ) const
{
  char tmp[ 256 ];
  snprintf( tmp, 256, "{%s} gen=%u ct=%u => (win=%d + %f * win, intersend=%f)",
	    _domain.str().c_str(), _generation, _domain.count(), _window_increment, _window_multiple, _intersend );
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
    _intersend( 10 * dna.intersend() ),
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
