#include "poisson.hh"

Poisson::Poisson( double rate )
  : generator(),
    distribution( rate )
{}

int Poisson::sample( void )
{
  return distribution( generator );
}
