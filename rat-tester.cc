#include <stdio.h>
#include <vector>

#include "evaluator.hh"

using namespace std;

void apply_best_split( WhiskerTree & whiskers, const unsigned int generation )
{
  Evaluator eval( whiskers );
  auto outcome( eval.score( {}, true ) );

  while ( 1 ) {
    auto my_whisker( outcome.used_whiskers.most_used( generation ) );
    assert( my_whisker );

    WhiskerTree bisected_whisker( *my_whisker, true );

    if ( bisected_whisker.num_children() == 1 ) {
      printf( "Got unbisectable whisker! %s\n", my_whisker->str().c_str() );
      auto mutable_whisker( *my_whisker );
      mutable_whisker.promote( generation + 1 );
      assert( outcome.used_whiskers.replace( mutable_whisker ) );
      continue;
    }

    printf( "Turning %s into %s\n", my_whisker->str().c_str(), bisected_whisker.str().c_str() );
    assert( whiskers.replace( *my_whisker, bisected_whisker ) );
    break;
  }
}

int main( void )
{
  WhiskerTree whiskers;

  unsigned int generation = 0;

  while ( 1 ) {
    /* evaluate the whiskers we have */
    const Evaluator eval( whiskers );

    auto outcome( eval.score( {} ) );

    printf( "gen %d, score = %.12f\n", generation, outcome.score );

    for ( auto &run : outcome.throughputs_delays ) {
      for ( auto &x : run ) {
	printf( "sender: [tp=%f, del=%f]\n", x.first, x.second );
      }
    }

    printf( "whiskers: %s\n\n", outcome.used_whiskers.str().c_str() );

    /* is there a whisker at this generation that we can improve? */
    auto my_whisker( outcome.used_whiskers.most_used( generation ) );

    /* if not, increase generation and promote all whiskers */
    if ( !my_whisker ) {
      printf( "Advancing to generation %d\n", generation );
      generation++;
      whiskers.promote( generation );

      if ( (generation % 4) == 0 ) {
	printf( "Splitting most popular whisker.\n" );
	apply_best_split( whiskers, generation );
	generation++;
	whiskers.promote( generation );
      }
      
      continue;
    }

    /* otherwise, get all nexgen alternatives for that whisker and replace each in turn */
    auto replacements( my_whisker->next_generation() );
    double best_score = -INT_MAX;
    const Whisker *best_whisker = nullptr;

    printf( "Evaluating %lu replacements.\n", replacements.size() );

    /* find best case (using same randseed) */
    for ( const auto &test_replacement : replacements ) {
      const double score( eval.score( { test_replacement } ).score );
      if ( score > best_score ) {
	best_whisker = &test_replacement;
	best_score = score;
      }
    }

    assert( best_whisker );

    /* replace with best nexgen choice and repeat */
    if ( best_score > outcome.score ) {
      printf( "Replacing with whisker that scored %.12f => %.12f (+%.12f)\n", outcome.score, best_score,
	      best_score - outcome.score );
    }
    assert( whiskers.replace( *best_whisker ) );
  }

  return 0;
}
