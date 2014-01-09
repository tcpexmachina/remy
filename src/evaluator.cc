#include <fcntl.h>
#include <boost/random/uniform_real_distribution.hpp>
#include <boost/random/uniform_int_distribution.hpp>

#include "configrange.hh"
#include "evaluator.hh"
#include "network.cc"
#include "rat-templates.cc"

const unsigned int TICK_COUNT = 100000;

Evaluator::Evaluator( const ConfigRange & range )
  : _prng_seed( global_PRNG()() ), /* freeze the PRNG seed for the life of this Evaluator */
    _configs()
{
  /* first load "anchors" */
  _configs.push_back( NetConfig().set_link_ppt( range.link_packets_per_ms.first ).set_delay( range.rtt_ms.first ).set_num_senders( range.max_senders ).set_on_duration( range.mean_on_duration ).set_off_duration( range.mean_off_duration ) );

  if ( range.lo_only ) {
    return;
  }

  _configs.push_back( NetConfig().set_link_ppt( range.link_packets_per_ms.first ).set_delay( range.rtt_ms.second ).set_num_senders( range.max_senders ).set_on_duration( range.mean_on_duration ).set_off_duration( range.mean_off_duration ) );
  _configs.push_back( NetConfig().set_link_ppt( range.link_packets_per_ms.second ).set_delay( range.rtt_ms.first ).set_num_senders( range.max_senders ).set_on_duration( range.mean_on_duration ).set_off_duration( range.mean_off_duration ) );
  _configs.push_back( NetConfig().set_link_ppt( range.link_packets_per_ms.second ).set_delay( range.rtt_ms.second ).set_num_senders( range.max_senders ).set_on_duration( range.mean_on_duration ).set_off_duration( range.mean_off_duration ) );

  /* now load some random ones just for fun */
  for ( int i = 0; i < 12; i++ ) {
    boost::random::uniform_real_distribution<> link_speed( range.link_packets_per_ms.first, range.link_packets_per_ms.second );
    boost::random::uniform_real_distribution<> rtt( range.rtt_ms.first, range.rtt_ms.second );
    boost::random::uniform_int_distribution<> num_senders( 1, range.max_senders );

    _configs.push_back( NetConfig().set_link_ppt( link_speed( global_PRNG() ) ).set_delay( rtt( global_PRNG() ) ).set_num_senders( num_senders( global_PRNG() ) ).set_on_duration( range.mean_on_duration ).set_off_duration( range.mean_off_duration ) );
  }
}

ProblemBuffers::Problem Evaluator::serialize_problem( WhiskerTree & whiskers, const bool trace, const unsigned int carefulness ) {
  ProblemBuffers::Problem ret;

  auto remycc = whiskers.DNA();
  // remycc.mutable_config()->CopyFrom( configuration_range.DNA() ); // we don't need this for scoring
  remycc.mutable_optimizer()->CopyFrom( Whisker::get_optimizer().DNA() );

  ProblemBuffers::ProblemSettings settings;
  settings.set_trace( trace );
  settings.set_carefulness( carefulness );
  settings.set_prng_seed( _prng_seed );

  ret.mutable_settings()->CopyFrom( settings );

  for ( auto &x : _configs ) {
    AnswerBuffers::NetConfig *config = ret.add_configs();
    *config = x.DNA();
  }

  return ret;
}

Evaluator::Outcome Evaluator::parse_problem_and_score( string input_filename ) {

  int fd = open( input_filename.c_str(), O_RDONLY );
  if ( fd < 0 ) {
    perror( "open" );
    exit( 1 );
  }

  ProblemBuffers::Problem problem;
  if ( !problem.ParseFromFileDescriptor( fd ) ) {
    fprintf( stderr, "Could not parse %s.\n", input_filename.c_str() );
    exit( 1 );
  }
  
  vector<NetConfig> configs;
  for ( const auto &x : problem.configs() ) {
    _configs.emplace_back( NetConfig( x ) );
  }

  WhiskerTree run_whiskers = WhiskerTree( problem.whiskers() );

  return Evaluator::score( run_whiskers, problem.settings().prng_seed(), configs, problem.settings().trace(), problem.settings().carefulness() );
}

AnswerBuffers::Outcome Evaluator::serialize_answer( Outcome answer ) {
  AnswerBuffers::Outcome ret;
  
  auto used_whiskers = answer.used_whiskers.DNA();
  //  used_whiskers.mutable_config()->CopyFrom( configuration_range.DNA() );
  used_whiskers.mutable_optimizer()->CopyFrom( Whisker::get_optimizer().DNA() );

  ret.mutable_used_whiskers()->CopyFrom( used_whiskers );

  for ( auto &run : answer.throughputs_delays ) {
    AnswerBuffers::ThroughputsDelays *tp_del = ret.add_throughputs_delays();
    auto config = run.first.DNA();
    tp_del->mutable_config()->CopyFrom( config );

    for ( auto &x : run.second ) {
      AnswerBuffers::SenderResults *results = tp_del->add_results();
      results->set_throughput( x.first / run.first.link_ppt ); 
      results->set_delay( x.second / run.first.delay );
    }
  }

  ret.mutable_throughputs_delays();
  ret.set_score(answer.score);

  return ret;
}

Evaluator::Outcome Evaluator::parse_answer( string input_filename ) {

  int fd = open( input_filename.c_str(), O_RDONLY );
  if ( fd < 0 ) {
    perror( "open" );
    exit( 1 );
  }

  AnswerBuffers::Outcome proto_outcome;
  if ( !proto_outcome.ParseFromFileDescriptor( fd ) ) {
    fprintf( stderr, "Could not parse %s.\n", input_filename.c_str() );
    exit( 1 );
  }

  Evaluator::Outcome outcome;

  RemyBuffers::WhiskerTree proto_used_whiskers = proto_outcome.used_whiskers();
  WhiskerTree used_whiskers( proto_used_whiskers );
  outcome.used_whiskers = used_whiskers;

  outcome.score = proto_outcome.score();

  for ( const auto &x : proto_outcome.throughputs_delays() ) {
    vector< pair< double, double > > tp_del;
    for ( const auto &result : x.results() ) {
      tp_del.emplace_back( result.throughput(), result.delay() );
    }

    outcome.throughputs_delays.emplace_back( NetConfig( x.config() ), tp_del );
  }

  return outcome;
  
}

Evaluator::Outcome Evaluator::score( WhiskerTree & run_whiskers,
				     const bool trace, const unsigned int carefulness ) const
{
  PRNG run_prng( _prng_seed );

  run_whiskers.reset_counts();

  /* run tests */
  Outcome the_outcome;
  for ( auto &x : _configs ) {
    /* run once */
    Network<Rat, Rat> network1( Rat( run_whiskers, trace ), run_prng, x );
    network1.run_simulation( TICK_COUNT * carefulness );
    
    the_outcome.score += network1.senders().utility();
    the_outcome.throughputs_delays.emplace_back( x, network1.senders().throughputs_delays() );
  }

  the_outcome.used_whiskers = run_whiskers;

  return the_outcome;
}


static Evaluator::Outcome score( WhiskerTree & run_whiskers, const unsigned int prng_seed, vector<NetConfig> configs, const bool trace = false, const unsigned int carefulness = 1 ) {
  PRNG run_prng( prng_seed );

  run_whiskers.reset_counts();

  /* run tests */
  Evaluator::Outcome the_outcome;
  for ( auto &x : configs ) {
    /* run once */
    Network<Rat, Rat> network1( Rat( run_whiskers, trace ), run_prng, x );
    network1.run_simulation( TICK_COUNT * carefulness );
    
    the_outcome.score += network1.senders().utility();
    the_outcome.throughputs_delays.emplace_back( x, network1.senders().throughputs_delays() );
  }

  the_outcome.used_whiskers = run_whiskers;

  return the_outcome;
}
