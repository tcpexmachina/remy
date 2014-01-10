#ifndef EVALUATOR_HH
#define EVALUATOR_HH

#include <string>
#include <vector>

#include "random.hh"
#include "whiskertree.hh"
#include "network.hh"
#include "problem.pb.h"
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
  };

private:
  const unsigned int _prng_seed;

  std::vector< NetConfig > _configs;

public:
  Evaluator( const ConfigRange & range );
  
  ProblemBuffers::Problem serialize_problem( WhiskerTree & whiskers, const bool trace = false, const unsigned int carefulness = 1 );

  Outcome score( WhiskerTree & run_whiskers, const bool trace = false, const unsigned int carefulness = 1 ) const;

  static Evaluator::Outcome parse_problem_and_score( ProblemBuffers::Problem problem );
  static AnswerBuffers::Outcome serialize_answer( Outcome answer );
  static Outcome parse_answer( AnswerBuffers::Outcome proto_outcome );
  static Outcome score( WhiskerTree & run_whiskers, const unsigned int prng_seed, std::vector<NetConfig> configs, const bool trace = false, const unsigned int carefulness = 1 );
};

#endif
