#ifndef POISSONPTPROCESS_HH
#define POISSONPTPROCESS_HH

#include "exponential.hh"

class PoissonPointProcess
{
private:
  Exponential distribution;
  double time_until_occurrence;

public:
  PoissonPointProcess( double rate )
    : distribution( rate ),
      time_until_occurrence( distribution.sample() )
  {}
  
  int tick( void )
  {
    int ret( 0 );

    while ( time_until_occurrence < 1.0 ) {
      ret++;
      time_until_occurrence += distribution.sample();
    }

    /* advance by one tick */
    time_until_occurrence -= 1.0;

    assert( time_until_occurrence >= 0.0 );

    return ret;
  }
};

#endif
