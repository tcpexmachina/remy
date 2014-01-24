#include <boost/random/uniform_real_distribution.hpp>
#include <boost/random/uniform_int_distribution.hpp>

#include "configrange.hh"
#include "evaluator.hh"
#include "network.cc"
#include "rat-templates.cc"
#include "aimd-templates.cc"

const unsigned int TICK_COUNT = 100000;

Evaluator::Evaluator( const ConfigRange & range )
  : _prng( global_PRNG()() ), /* freeze the PRNG seed for the life of this Evaluator */
    _configs()
{
  const double steps = 3.0;

  const double link_speed_dynamic_range = range.link_packets_per_ms.second / range.link_packets_per_ms.first;

  const double multiplier = pow( link_speed_dynamic_range, 1.0 / steps );

  double link_speed = range.link_packets_per_ms.first;

  /* this approach only varies link speed, so make sure no uncertainty in rtt */
  assert( range.rtt_ms.first == range.rtt_ms.second );

  while ( link_speed <= (range.link_packets_per_ms.second * ( 1 + (multiplier-1) / 2 ) ) ) {
    /* Rat vs AIMD, 1 second on, 1 second off */
    _configs.push_back( NetConfig().set_link_ppt( link_speed )
                                   .set_delay( range.rtt_ms.first )
                                   .set_num_senders1( 1 )
                                   .set_num_senders2( 1 )
                                   .set_on_duration( 5000.0 )
                                   .set_off_duration( 5000.0 ) );

    /* Rat vs AIMD, always on */
    _configs.push_back( NetConfig().set_link_ppt( link_speed )
                                   .set_delay( range.rtt_ms.first )
                                   .set_num_senders1( 1 )
                                   .set_num_senders2( 1 )
                                   .set_on_duration( 5000.0 )
                                   .set_off_duration( 10.0 ) );

    /* Two rats, 1 second on, 1 second off */
    _configs.push_back( NetConfig().set_link_ppt( link_speed )
                                   .set_delay( range.rtt_ms.first )
                                   .set_num_senders1( 2 )
                                   .set_num_senders2( 0 )
                                   .set_on_duration( 5000.0 )
                                   .set_off_duration( 5000.0 ) );

    /* Two rats, always on */
    _configs.push_back( NetConfig().set_link_ppt( link_speed )
                                   .set_delay( range.rtt_ms.first )
                                   .set_num_senders1( 2 )
                                   .set_num_senders2( 0 )
                                   .set_on_duration( 5000.0 )
                                   .set_off_duration( 10.0 ) );

    /* Move on to the next */
    link_speed *= multiplier;
  }
}

Evaluator::Outcome Evaluator::score( WhiskerTree & run_whiskers,
				     const bool trace, const unsigned int carefulness ) const
{
  PRNG run_prng( _prng );

  run_whiskers.reset_counts();

  /* run tests */
  Outcome the_outcome;
  for ( auto &x : _configs ) {
    /* run once */
    Network<Rat, Aimd> network1( Rat( run_whiskers, trace ), Aimd(), run_prng, x, std::make_pair(0.1, 0.1) );
    network1.run_simulation( TICK_COUNT * carefulness );

    the_outcome.score += network1.senders().utility();
    the_outcome.throughputs_delays.emplace_back( x, network1.senders().throughputs_delays() );
  }

  the_outcome.used_whiskers = run_whiskers;

  return the_outcome;
}
