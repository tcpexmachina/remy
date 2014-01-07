#ifndef EVALUATOR_HH
#define EVALUATOR_HH

#include <vector>

#include "random.hh"
#include "whiskertree.hh"
#include "network.hh"

class Evaluator
{
public:
  class Outcome
  {
  public:
    double score;
    std::vector< std::pair< NetConfig, std::vector< std::pair< double, double > > > > throughputs_delays;
    WhiskerTree used_whiskers;

    Outcome() : score( 0 ), throughputs_delays(), used_whiskers() {}
  };

private:
  const PRNG _prng;

  std::vector< NetConfig > _configs;

public:
  Evaluator( const ConfigRange & range );
  Outcome score( WhiskerTree & run_whiskers, const bool trace = false, const unsigned int carefulness = 1 ) const;
};

#endif
