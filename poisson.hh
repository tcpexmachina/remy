#ifndef POISSON_HH
#define POISSON_HH

#include <boost/random/poisson_distribution.hpp>

#include "random.hh"

class Poisson
{
private:
  boost::random::poisson_distribution<> distribution;

  PRNG & prng;

public:
  Poisson( const double & rate, PRNG & s_prng ) : distribution( rate ), prng( s_prng ) {}

  int sample( void ) { return distribution( prng ); }
};

#endif
