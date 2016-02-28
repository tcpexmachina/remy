#ifndef POISSON_HH
#define POISSON_HH

#include <cassert>

#include "random.hh"

class Poisson
{
private:
  std::poisson_distribution<> distribution;

  PRNG & prng;

public:
  Poisson( const double & rate, PRNG & s_prng ) : distribution( rate ), prng( s_prng ) {}

  int sample( void ) { return distribution( prng ); }

  Poisson & operator=( const Poisson & ) { assert( false ); return *this; }
};

#endif
