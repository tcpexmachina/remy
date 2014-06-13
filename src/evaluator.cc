#include <boost/random/uniform_real_distribution.hpp>
#include <boost/random/uniform_int_distribution.hpp>

#include "configrange.hh"
#include "evaluator.hh"
#include "network.cc"
#include "rat-templates.cc"


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

Evaluator::Evaluator( const ConfigRange & range )
  : _prng_seed( global_PRNG()() ), /* freeze the PRNG seed for the life of this Evaluator */
    _prng( _prng_seed ),
    _ticks( TICK_COUNT ),
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

ProblemBuffers::Problem Evaluator::bundle_up( const WhiskerTree & run_whiskers ) const
{
  ProblemBuffers::Problem problem;
  problem.mutable_settings()->set_prng_seed( _prng_seed );
  problem.mutable_settings()->set_tick_count( _ticks );
  for ( auto &x: _configs ) {
    auto tmp_config = problem.mutable_scenarios()->add_configs();
    *tmp_config = x.DNA();
  }
  problem.mutable_whiskers()->CopyFrom( run_whiskers.DNA() );
  return problem;
}

Evaluator::Evaluator( const std::vector<NetConfig> & s_configs,
                      const unsigned int s_prng_seed,
                      const unsigned int ticks )
  : _prng_seed( s_prng_seed ), /* freeze the PRNG seed for the life of this Evaluator */
    _prng( _prng_seed ),
    _ticks( ticks ),
    _configs( s_configs )
{}

Evaluator::Outcome Evaluator::score( WhiskerTree & run_whiskers,
				     const bool trace, const unsigned int carefulness ) const
{
  PRNG run_prng( _prng );

  run_whiskers.reset_counts();

  /* run tests */
  Outcome the_outcome;
  for ( auto &x : _configs ) {
    /* run once */
    Network<Rat, Rat> network1( Rat( run_whiskers, trace ), run_prng, x );
    network1.run_simulation( _ticks * carefulness );

    the_outcome.score += network1.senders().utility();
    the_outcome.throughputs_delays.emplace_back( x, network1.senders().throughputs_delays() );
  }

  the_outcome.used_whiskers = run_whiskers;

  return the_outcome;
}
