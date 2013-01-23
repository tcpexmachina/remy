#include <assert.h>
#include <math.h>
#include <algorithm>

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

Whisker::Whisker( const unsigned int s_window, const double s_intersend, const MemoryRange & s_domain )
  : _generation( 0 ),
    _window( s_window ),
    _intersend( s_intersend ),
    _domain( s_domain )
{
}

Whisker::Whisker( const Whisker & other )
  : _generation( other._generation ),
    _window( other._window ),
    _intersend( other._intersend ),
    _domain( other._domain )
{
}

vector< Whisker > Whisker::next_generation( void ) const
{
  vector< Whisker > ret_windows;

  /* generate all window sizes */
  Whisker copy( *this );
  copy._generation++;
  ret_windows.push_back( copy );

  for ( unsigned int i = 1; i <= MAX_WINDOW_INCR; i *= 8 ) {
    Whisker new_whisker( *this );
    new_whisker._generation++;

    if ( _window + i <= MAX_WINDOW ) {
      new_whisker._window = _window + i;
      ret_windows.push_back( new_whisker );
    }

    if ( _window > i ) {
      new_whisker._window = _window - i;
      ret_windows.push_back( new_whisker );
    }
  }

  /* generate all rates */
  vector< Whisker > ret;
  for ( auto &x : ret_windows ) {
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

  return ret;
}

void Whisker::promote( const unsigned int generation )
{
  _generation = max( _generation, generation );
}

string Whisker::str( void ) const
{
  char tmp[ 128 ];
  snprintf( tmp, 128, "{%s} gen=%u ct=%u => (win=%u, intersend=%.2f)",
	    _domain.str().c_str(), _generation, _domain.count(), _window, _intersend );
  return tmp;
}

