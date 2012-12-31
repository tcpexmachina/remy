#include <time.h>

#include "random.hh"

PRNG & global_PRNG( void )
{
  static PRNG generator( time( NULL ) );
  return generator;
}
