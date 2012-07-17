#ifndef POISSON_HH
#define POISSON_HH

#include <boost/random/mersenne_twister.hpp>
#include <boost/random/poisson_distribution.hpp>

class Poisson
{
private:
  boost::random::mt19937 generator;
  boost::random::poisson_distribution<> distribution;

public:
  Poisson( double rate );
  
  int sample( void );
};

#endif
