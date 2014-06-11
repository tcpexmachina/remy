#ifndef EVALUATOR_HH
#define EVALUATOR_HH

#include <vector>

#include "random.hh"
#include "whiskertree.hh"
#include "network.hh"
#include "answer.pb.h"

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
    AnswerBuffers::Outcome DNA( void ) const;
  };

private:
  const PRNG _prng;

  unsigned int _ticks;

  std::vector< NetConfig > _configs;

public:
  Evaluator( const ConfigRange & range );
  Evaluator( const std::vector<NetConfig> & s_configs,
             const unsigned int prng_seed = global_PRNG()(),
             const unsigned int ticks = TICK_COUNT);
  Outcome score( WhiskerTree & run_whiskers, const bool trace = false, const unsigned int carefulness = 1 ) const;

  static const unsigned int TICK_COUNT = 1000000;
};

#endif
