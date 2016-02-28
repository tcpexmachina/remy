#ifndef EXPONENTIAL_HH
#define EXPONENTIAL_HH

#include <random>

#include "random.hh"

class Exponential
{
private:
  std::exponential_distribution<> distribution;

public:
  Exponential( const double & rate ) : distribution( rate ) {}
  
  double sample( PRNG & prng ) { return distribution( prng ); }
};

#endif
