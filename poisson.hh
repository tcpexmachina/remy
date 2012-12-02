#ifndef POISSON_HH
#define POISSON_HH

#include <time.h>
#include <boost/random/mersenne_twister.hpp>
#include <boost/random/poisson_distribution.hpp>

class Poisson
{
private:
  static boost::random::mt19937 & get_generator( void )
  {
    static boost::random::mt19937 generator( time( NULL ) );
    return generator;
  }
  boost::random::poisson_distribution<> distribution;

public:
  Poisson( double rate ) : distribution( rate ) {}
  
  int sample( void ) { return distribution( get_generator() ); }
};

#endif
