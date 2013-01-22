#include "evaluator.hh"
#include "network.cc"
#include "rat-templates.cc"

const unsigned int TICK_COUNT = 100000;

Evaluator::Evaluator( const WhiskerTree & s_whiskers )
  : _prng( global_PRNG()() ),
    _whiskers( s_whiskers )
{
}

Evaluator::Outcome Evaluator::score( const std::vector< Whisker > & replacements, const bool trace ) const
{
  PRNG run_prng( _prng );

  WhiskerTree run_whiskers( _whiskers );
  for ( auto &x : replacements ) {
    assert( run_whiskers.replace( x ) );
  }

  run_whiskers.reset_counts();

  Network<Rat> network( Rat( run_whiskers, trace ), run_prng, NetConfig() );
  network.tick( TICK_COUNT );

  return Outcome( network.senders().utility(),
		  network.senders().throughputs_delays(),
		  run_whiskers );
}
