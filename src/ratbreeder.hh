#ifndef RATBREEDER_HH
#define RATBREEDER_HH

#include "configrange.hh"
#include "evaluator.hh"

class RatBreeder
{
private:
  ConfigRange _range;

  void apply_best_split( WhiskerTree & whiskers, const unsigned int generation );

public:
  RatBreeder( const ConfigRange & s_range ) : _range( s_range ) {}

  Evaluator::Outcome improve( WhiskerTree & whiskers );
};

#endif
