#ifndef EVALUATOR_HH
#define EVALUATOR_HH

#include <string>
#include <vector>

#include "random.hh"
#include "whiskertree.hh"
#include "network.hh"
#include "problem.pb.h"
#include "answer.pb.h"
#include "configvector.hh"
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

    Outcome( const AnswerBuffers::Outcome & dna );

    AnswerBuffers::Outcome DNA( void ) const;
  };

private:
  const unsigned int _prng_seed;

  std::vector< NetConfig > _configs;

public:
  Evaluator( const ConfigVector & vector );
  
  ProblemBuffers::Problem DNA( const WhiskerTree & whiskers ) const;

  Outcome score( WhiskerTree & run_whiskers,
		const bool trace = false,
		const double carefulness = 1) const;

  static Evaluator::Outcome parse_problem_and_evaluate( const ProblemBuffers::Problem & problem );

  static Outcome score( WhiskerTree & run_whiskers,
			const unsigned int prng_seed,
			const std::vector<NetConfig> & configs,
			const bool trace,
			const unsigned int ticks_to_run );
};

#endif
