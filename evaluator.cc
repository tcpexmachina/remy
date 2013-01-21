#include "evaluator.hh"

Evaluator( const Whiskers & s_whiskers )
  : _prng( global_PRNG()() ),
    _whiskers( s_whiskers )
{
}

double Evaluator::score( const Whiskers & run_whiskers ) const
{
  PRNG run_prng( _prng );
  Network<Rat> network( run_whiskers, run_prng );
  network.tick( TICK_COUNT );  
  
}
