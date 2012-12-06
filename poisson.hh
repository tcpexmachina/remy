#ifndef POISSON_HH
#define POISSON_HH

#include <boost/random/poisson_distribution.hpp>

#include "random.hh"

class Poisson
{
private:
  boost::random::poisson_distribution<> distribution;

public:
  Poisson( double rate ) : distribution( rate ) {}
  
  int sample( void ) { return distribution( get_generator() ); }
};

#endif
