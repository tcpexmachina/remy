#ifndef EXPONENTIAL_HH
#define EXPONENTIAL_HH

#include <boost/random/exponential_distribution.hpp>

#include "random.hh"

class Exponential
{
private:
  boost::random::exponential_distribution<> distribution;

public:
  Exponential( const double & rate ) : distribution( rate ) {}
  
  double sample( PRNG & prng ) { return distribution( prng ); }
};

#endif
