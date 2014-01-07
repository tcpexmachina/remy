#include <boost/random/uniform_real_distribution.hpp>
#include <boost/random/uniform_int_distribution.hpp>

#include "configrange.hh"
#include "evaluator.hh"
#include "network.cc"
#include "rat-templates.cc"
#include "aimd-templates.cc"

const unsigned int TICK_COUNT = 1000000;

Evaluator::Evaluator( const ConfigRange & range )
  : _prng( global_PRNG()() ), /* freeze the PRNG seed for the life of this Evaluator */
    _configs()
{
  /* 0, 1, or 2 of each of the two sender classes */
  for ( int i = 0; i <= 2; i++ ) {
    for ( int j = 0; j <= 2; j++ ) {
      if ( i == 0 ) {
        /* We can only optimize whiskers, not Aimd */
        continue;
      } else {
        _configs.push_back( NetConfig().set_link_ppt( range.link_packets_per_ms.first )
                                       .set_delay( range.rtt_ms.first )
                                       .set_num_senders1( i )
                                       .set_num_senders2( j )
                                       .set_on_duration( range.mean_on_duration )
                                       .set_off_duration( range.mean_off_duration ) );
      }
    }
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
    Network<Rat, Aimd> network1( Rat( run_whiskers, trace ), Aimd(), run_prng, x );
    network1.run_simulation( TICK_COUNT * carefulness );

    the_outcome.score += network1.senders().utility();
    the_outcome.throughputs_delays.emplace_back( x, network1.senders().throughputs_delays() );
  }

  the_outcome.used_whiskers = run_whiskers;

  return the_outcome;
}
