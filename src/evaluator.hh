#ifndef EVALUATOR_HH
#define EVALUATOR_HH

#include <string>
#include <vector>

#include "random.hh"
#include "whiskertree.hh"
#include "network.hh"
#include "problem.pb.h"
#include "answer.pb.h"

using namespace std;

class Evaluator
{
public:
  class Outcome
  {
  public:
    double score;
    vector< pair< NetConfig, vector< pair< double, double > > > > throughputs_delays;
    WhiskerTree used_whiskers;

    Outcome() : score( 0 ), throughputs_delays(), used_whiskers() {}
  };

private:
  const unsigned int _prng_seed;

  vector< NetConfig > _configs;

public:
  Evaluator( const ConfigRange & range );
  
  ProblemBuffers::Problem serialize_problem( WhiskerTree & whiskers, const bool trace, const unsigned int carefulness );
  Evaluator::Outcome parse_problem_and_score( string input_filename );
  AnswerBuffers::Outcome serialize_answer( Outcome answer );
  Outcome parse_answer( string input_filename );

  Outcome score( WhiskerTree & run_whiskers, const bool trace = false, const unsigned int carefulness = 1 ) const;

  static Outcome score( WhiskerTree & run_whiskers, const unsigned int prng_seed, vector<NetConfig> configs, const bool trace = false, const unsigned int carefulness = 1 );
};

#endif
