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

  NetConfig run1;
  Network<Rat> network1( Rat( run_whiskers, trace ), run_prng, run1 );
  network1.tick( TICK_COUNT );

  NetConfig run2;
  run2.delay = 200;
  Network<Rat> network2( Rat( run_whiskers, trace ), run_prng, run2 );
  network2.tick( TICK_COUNT );

  return Outcome( 0.5 * (network1.senders().utility() + network2.senders().utility()),
		  { network1.senders().throughputs_delays(),
		      network2.senders().throughputs_delays() },
		  run_whiskers );
}
