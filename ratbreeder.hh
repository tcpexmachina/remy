#ifndef RATBREEDER_HH
#define RATBREEDER_HH

#include "evaluator.hh"

class RatBreeder
{
private:
  void apply_best_split( WhiskerTree & whiskers, const unsigned int generation );

public:
  RatBreeder() {}

  Evaluator::Outcome improve( WhiskerTree & whiskers );
};

#endif
