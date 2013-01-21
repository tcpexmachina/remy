#ifndef EVALUATOR_HH
#define EVALUATOR_HH

#include "random.hh"

class Evaluator
{
private:
  const PRNG _prng;
  const Whiskers _whiskers;

  double score( const Whiskers & run_whiskers );

public:
  Evaluator( const Whiskers & s_whiskers );
  double score( void ) const;
  double score( const Whisker & replacement );
  const Whiskers & used_whiskers( void );
};

#endif
