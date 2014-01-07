#include <utility>

#include <boost/random/uniform_real_distribution.hpp>
#include <boost/random/uniform_int_distribution.hpp>

#include "configrange.hh"
#include "evaluator.hh"
#include "network.cc"
#include "rat-templates.cc"

const unsigned int TICK_COUNT = 1000000;

ZigZag& operator!(ZigZag& z)
{
  switch (z)
  {
    case ZigZag::ZIG: return z=ZigZag::ZAG;
    case ZigZag::ZAG: return z=ZigZag::ZIG;
    default: throw "";
  }
}

Evaluator::Evaluator( const WhiskerTree & s_whiskers1, const WhiskerTree & s_whiskers2, const ConfigRange & range )
  : _prng( global_PRNG()() ), /* freeze the PRNG seed for the life of this Evaluator */
    _whiskers1( s_whiskers1 ),
    _whiskers2( s_whiskers2 ),
    _configs()
{
  /* 0, 1, or 2 of each of the two sender classes */
  for ( int i = 0; i <= 2; i++ ) {
    for ( int j = 0; j <= 2; j++ ) {
      if ( i == 0 and j == 0 ) {
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

Evaluator::Outcome Evaluator::score( WhiskerTree & run_whiskers1, WhiskerTree & run_whiskers2,
				     const bool trace, const unsigned int carefulness ) const
{
  PRNG run_prng( _prng );

  run_whiskers1.reset_counts();
  run_whiskers2.reset_counts();

  /* run tests */
  Outcome the_outcome;
  for ( auto &x : _configs ) {
    /* run once */
    Network<Rat, Rat> network1( Rat( run_whiskers1, trace ), Rat( run_whiskers2, trace ), run_prng, x, make_pair( 0.1, 10.0 ) );
    network1.run_simulation( TICK_COUNT * carefulness );

    the_outcome.score += network1.senders().utility();
    the_outcome.throughputs_delays.emplace_back( x, network1.senders().throughputs_delays() );
  }

  the_outcome.used_whiskers = make_pair( run_whiskers1, run_whiskers2 );

  return the_outcome;
}

Evaluator::Outcome Evaluator::score( const std::vector< Whisker > & replacements, ZigZag tree_id,
				     const bool trace, const unsigned int carefulness ) const
{
  PRNG run_prng( _prng );

  WhiskerTree run_whiskers1( _whiskers1 );
  WhiskerTree run_whiskers2( _whiskers2 );
  for ( const auto &x : replacements ) {
    assert( ( tree_id == ZigZag::ZIG ? run_whiskers1 : run_whiskers2 ).replace( x ) );
  }

  return score( run_whiskers1, run_whiskers2, trace, carefulness );
}
