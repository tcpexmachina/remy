#include <time.h>

#include "random.hh"

boost::random::mt19937 & get_generator( void )
{
  static boost::random::mt19937 generator( time( NULL ) );
  return generator;
}
