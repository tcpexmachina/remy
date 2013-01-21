#ifndef EVALUATOR_HH
#define EVALUATOR_HH

#include "random.hh"
#include "whiskertree.hh"

class Evaluator
{
public:
  class Outcome
  {
  public:
    double score;
    WhiskerTree used_whiskers;

    Outcome( const double s_score, const WhiskerTree & s_used_whiskers )
      : score( s_score ), used_whiskers( s_used_whiskers )
    {}
  };

private:
  const PRNG _prng;
  const WhiskerTree _whiskers;

public:
  Evaluator( const WhiskerTree & s_whiskers );
  Outcome score( const std::vector< Whisker > & replacements, const bool trace = false ) const;
};

#endif
