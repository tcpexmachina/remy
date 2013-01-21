#ifndef EVALUATOR_HH
#define EVALUATOR_HH

#include "random.hh"
#include "whisker.hh"

class Evaluator
{
public:
  class Outcome
  {
  public:
    double score;
    Whiskers used_whiskers;

    Outcome( const double s_score, const Whiskers & s_used_whiskers )
      : score( s_score ), used_whiskers( s_used_whiskers )
    {}
  };

private:
  const PRNG _prng;
  const Whiskers _whiskers;

public:
  Evaluator( const Whiskers & s_whiskers );
  Outcome score( const std::vector< Whisker > & replacements, const bool trace = false ) const;
};

#endif
