#include <fcntl.h>
#include <boost/random/uniform_real_distribution.hpp>
#include <boost/random/uniform_int_distribution.hpp>

#include "configrange.hh"
#include "evaluator.hh"
#include "network.cc"
#include "rat-templates.cc"

const unsigned int TICK_COUNT = 1000000;

Evaluator::Evaluator( const ConfigVector & vector )
  : _prng_seed( global_PRNG()() ), /* freeze the PRNG seed for the life of this Evaluator */
    _configs()
{
  // take the config vector given
  // loop through and add to the _configs var
  for ( auto &config : vector.configs ) {
    _configs.push_back( config );
  }
}

ProblemBuffers::Problem Evaluator::DNA( const WhiskerTree & whiskers ) const
{
  ProblemBuffers::Problem ret;

  ret.mutable_whiskers()->CopyFrom( whiskers.DNA() );

  ProblemBuffers::ProblemSettings settings;
  settings.set_prng_seed( _prng_seed );
  settings.set_tick_count( TICK_COUNT );

  ret.mutable_settings()->CopyFrom( settings );

  for ( auto &x : _configs ) {
    RemyBuffers::NetConfig *config = ret.add_configs();
    *config = x.DNA();
  }

  return ret;
}

Evaluator::Outcome Evaluator::parse_problem_and_evaluate( const ProblemBuffers::Problem & problem )
{
  vector<NetConfig> configs;
  for ( const auto &x : problem.configs() ) {
    configs.emplace_back( x );
  }

  WhiskerTree run_whiskers = WhiskerTree( problem.whiskers() );

  return Evaluator::score( run_whiskers, problem.settings().prng_seed(),
			   configs, false, problem.settings().tick_count() );
}

AnswerBuffers::Outcome Evaluator::Outcome::DNA( void ) const
{
  AnswerBuffers::Outcome ret;

  for ( const auto & run : throughputs_delays ) {
    AnswerBuffers::ThroughputsDelays *tp_del = ret.add_throughputs_delays();
    tp_del->mutable_config()->CopyFrom( run.first.DNA() );

    for ( const auto & x : run.second ) {
      AnswerBuffers::SenderResults *results = tp_del->add_results();
      results->set_throughput( x.first ); 
      results->set_delay( x.second );
    }
  }

  ret.set_score( score );

  return ret;
}

Evaluator::Outcome::Outcome( const AnswerBuffers::Outcome & dna )
  : score( dna.score() ),
    throughputs_delays(),
    used_whiskers()
{
  for ( const auto &x : dna.throughputs_delays() ) {
    vector< pair< double, double > > tp_del;
    for ( const auto &result : x.results() ) {
      tp_del.emplace_back( result.throughput(), result.delay() );
    }

    throughputs_delays.emplace_back( NetConfig( x.config() ), tp_del );
  }
}

Evaluator::Outcome Evaluator::score( WhiskerTree & run_whiskers,
				     const bool trace, const unsigned int carefulness ) const
{
  return score( run_whiskers, _prng_seed, _configs, trace, TICK_COUNT * carefulness );
}


Evaluator::Outcome Evaluator::score( WhiskerTree & run_whiskers,
				     const unsigned int prng_seed,
				     const vector<NetConfig> & configs,
				     const bool trace,
				     const unsigned int ticks_to_run )
{
  PRNG run_prng( prng_seed );

  run_whiskers.reset_counts();

  /* run tests */
  Evaluator::Outcome the_outcome;
  for ( auto &x : configs ) {
    /* run once */
    Network<Rat, Rat> network1( Rat( run_whiskers, trace ), run_prng, x );
    network1.run_simulation( ticks_to_run );
    
    the_outcome.score += network1.senders().utility();
    the_outcome.throughputs_delays.emplace_back( x, network1.senders().throughputs_delays() );
  }

  the_outcome.used_whiskers = run_whiskers;

  return the_outcome;
}
