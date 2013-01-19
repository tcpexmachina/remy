#include <stdio.h>
#include <vector>

#include "rat-templates.cc"
#include "random.hh"
#include "network.cc"

using namespace std;

void summarize( const Network<Rat> & network, const bool all=false )
{
  //  printf( "util=%9.5f", network.senders().utility() );

  auto tds = network.senders().throughputs_delays();
  for ( auto &x : tds ) {
    printf( "    [ tp=%.4f del=%.4f ]", x.first, x.second );
  }
  printf( "\n\n" );

  if ( all ) {
    printf( "\n\n" );
    auto senders = network.senders().senders();
    for ( auto &x : senders ) {
      printf( "Whiskers:" );
      printf( " %s", x->whiskers().str().c_str() );
      printf( "\n\n" );
    }
  }
}

int main( void )
{
  const unsigned int TICK_COUNT = 500000;

  Whiskers whiskers;

  unsigned int generation = 0;

  while ( 1 ) {
    /* evaluate the whiskers we have */
    const PRNG the_prng = PRNG( global_PRNG()() );

    /* run with existing whiskers */
    PRNG run_prng( the_prng );
    Network<Rat> network( whiskers, run_prng );
    network.tick( TICK_COUNT );
    const double orig_score( network.senders().utility() );
    //    printf( "gen %d, score = %.12f\n", generation, orig_score );

    summarize( network );

    /* is there a whisker at this generation that we can improve? */
    auto my_sender( network.senders().senders()[ 0 ] );
    auto my_whisker( my_sender->whiskers().most_used( generation ) );

    /* if not, increase generation and promote all whiskers */
    if ( !my_whisker ) {
      generation++;
      printf( "Advancing to generation %d\n", generation );
      whiskers.promote( generation );
      summarize( network, true );
      continue;
    }

    /* otherwise, get all nexgen alternatives for that whisker and replace each in turn */
    auto replacements( my_whisker->next_generation() );
    double best_score = -INT_MAX;
    const Whisker *best_whisker = nullptr;

    /* find best case (using same randseed) */
    for ( const auto &test_replacement : replacements ) {
      auto new_whiskers( whiskers );
      assert( new_whiskers.replace( test_replacement ) );
      PRNG new_run_prng( the_prng );
      Network<Rat> test_network( new_whiskers, new_run_prng );
      test_network.tick( TICK_COUNT );

      const double score( test_network.senders().utility() );
      if ( score > best_score ) {
	best_whisker = &test_replacement;
	best_score = score;
      }
    }

    assert( best_whisker );

    /* replace with best nexgen choice and repeat */
    if ( best_score > orig_score ) {
      printf( "Replacing with whisker that scored %.12f => %.12f (+%.12f) ", orig_score, best_score,
	      best_score - orig_score );
      summarize( network );
      printf( "\n" );
    }
    assert( whiskers.replace( *best_whisker ) );
  }

  return 0;
}
