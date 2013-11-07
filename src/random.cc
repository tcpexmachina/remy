#include <ctime>
#include <sys/types.h>
#include <unistd.h>

#include "random.hh"

PRNG & global_PRNG( void )
{
  static PRNG generator( 123 );
  return generator;
}
