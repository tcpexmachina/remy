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

const unsigned int TICK_COUNT = 500000;

void apply_best_split( Whiskers & whiskers )
{
  whiskers.reset_counts();
  
  const PRNG the_prng = PRNG( global_PRNG()() );

  /* run to get counts */
  PRNG run_prng( the_prng );
  Network<Rat> network( Rat( whiskers, true ), run_prng );
  network.tick( TICK_COUNT );

  auto my_sender( network.senders().senders()[ 0 ] );
  auto my_whisker( my_sender->whiskers().most_used( -1 ) );

  assert( my_whisker );

  Whiskers bisected_whisker( *my_whisker, true );

  assert( whiskers.replace( *my_whisker, bisected_whisker ) );
}

int main( void )
{
  Whiskers whiskers;

  unsigned int generation = 0;

  while ( 1 ) {
    if ( generation >= 128 ) {
      exit( 0 );
    }

    /* evaluate the whiskers we have */
    const PRNG the_prng = PRNG( global_PRNG()() );

    /* run with existing whiskers */
    PRNG run_prng( the_prng );
    Network<Rat> network( whiskers, run_prng );
    network.tick( TICK_COUNT );
    const double orig_score( network.senders().utility() );
    printf( "gen %d, score = %.12f\n", generation, orig_score );

    summarize( network, true );

    /* is there a whisker at this generation that we can improve? */
    auto my_sender( network.senders().senders()[ 0 ] );
    auto my_whisker( my_sender->whiskers().most_used( generation ) );

    /* if not, increase generation and promote all whiskers */
    if ( !my_whisker ) {
      printf( "Advancing to generation %d\n", generation );
      generation++;
      whiskers.promote( generation );

      if ( (generation % 4) == 0 ) {
	printf( "Splitting most popular whisker.\n" );
	apply_best_split( whiskers );
      }
      
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
      printf( "Replacing with whisker that scored %.12f => %.12f (+%.12f)\n", orig_score, best_score,
	      best_score - orig_score );
    }
    assert( whiskers.replace( *best_whisker ) );
  }

  return 0;
}
