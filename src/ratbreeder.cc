#include <cstdio>
#include <vector>
#include <cassert>
#include <future>
#include <unordered_map>
#include <boost/functional/hash.hpp>

#include "ratbreeder.hh"

using namespace std;

void RatBreeder::apply_best_split( WhiskerTree & whiskers, const unsigned int generation )
{
  const Evaluator eval( whiskers, _range );
  auto outcome( eval.score( {}, true ) );

  while ( 1 ) {
    auto my_whisker( outcome.used_whiskers.most_used( generation ) );
    assert( my_whisker );

    WhiskerTree bisected_whisker( *my_whisker, true );

    if ( bisected_whisker.num_children() == 1 ) {
      //      printf( "Got unbisectable whisker! %s\n", my_whisker->str().c_str() );
      auto mutable_whisker( *my_whisker );
      mutable_whisker.promote( generation + 1 );
      assert( outcome.used_whiskers.replace( mutable_whisker ) );
      continue;
    }

    //    printf( "Bisecting === %s === into === %s ===\n", my_whisker->str().c_str(), bisected_whisker.str().c_str() );
    assert( whiskers.replace( *my_whisker, bisected_whisker ) );
    break;
  }
}

Evaluator::Outcome RatBreeder::improve( WhiskerTree & whiskers )
{
  /* evaluate the whiskers we have */
  whiskers.reset_generation();
  unsigned int generation = 0;

  while ( 1 ) {
    const Evaluator eval( whiskers, _range );
    unordered_map< Whisker, double, boost::hash< Whisker > > evalcache;

    auto outcome( eval.score( {} ) );

    //    printf( "gen %d, score = %.12f\n", generation, outcome.score );

    /*
    for ( auto &run : outcome.throughputs_delays ) {
      for ( auto &x : run ) {
	printf( "sender: [tp=%f, del=%f]\n", x.first, x.second );
      }
    }
    */

    //    printf( "Whiskers at generation %u: %s\n\n", generation, outcome.used_whiskers.str().c_str() );

    /* is there a whisker at this generation that we can improve? */
    auto my_whisker( outcome.used_whiskers.most_used( generation ) );

    /* if not, increase generation and promote all whiskers */
    if ( !my_whisker ) {
      generation++;
      //      printf( "Advancing to generation %d\n", generation );
      whiskers.promote( generation );

      if ( (generation % 4) == 0 ) {
	//	printf( "Splitting most popular whisker.\n" );
	//	printf( "Whiskers before split at generation %u: %s\n\n", generation, whiskers.str().c_str() );
	apply_best_split( whiskers, generation );
	//	printf( "Whiskers after split at generation %u: %s\n\n", generation, whiskers.str().c_str() );
	generation++;
	whiskers.promote( generation );
	break;
      }

      continue;
    }

    Whisker differential_whisker( *my_whisker );
    unsigned int diffgen = my_whisker->generation();

    /* otherwise, get all nexgen alternatives for that whisker and replace each in turn */
    while ( 1 ) {
      auto replacements( differential_whisker.next_generation() );
      double best_score = -INT_MAX;
      const Whisker *best_whisker = nullptr;

      //      printf( "Evaluating %lu replacements for %s.\n", replacements.size(), differential_whisker.str().c_str() );

      vector< pair< Whisker &, future< double > > > scores;
      vector< pair< Whisker &, double > > memoized_scores;

      /* find best case (using same randseed) */
      for ( auto &test_replacement : replacements ) {
	//	printf( "Evaluating %s... ", test_replacement.str().c_str() );
	if ( evalcache.find( test_replacement ) == evalcache.end() ) {
	  scores.emplace_back( test_replacement, async( launch::async, [] (const Evaluator &e, const Whisker &r) { return e.score( { r } ).score; }, eval, test_replacement ) );
	} else {
	  memoized_scores.emplace_back( test_replacement, evalcache.at( test_replacement ) );
	}
      }

      for ( auto &x : memoized_scores ) {
	const double score( x.second );
	//	printf( "score = %f\n", score );
	if ( score > best_score ) {
	  best_whisker = &x.first;
	  best_score = score;
	}
      }

      for ( auto &x : scores ) {
	const double score( x.second.get() );
	evalcache.insert( make_pair( x.first, score ) );
	//	printf( "score = %f\n", score );
	if ( score > best_score ) {
	  best_whisker = &x.first;
	  best_score = score;
	}
      }

      assert( best_whisker );

      /* replace with best nexgen choice and repeat */
      if ( best_score > outcome.score ) {
	//	printf( "Replacing with whisker that scored %.12f => %.12f (+%.12f)\n", outcome.score, best_score,
	//		best_score - outcome.score );
	//	printf( "=> %s\n", best_whisker->str().c_str() );
	assert( whiskers.replace( *best_whisker ) );
	differential_whisker = *best_whisker;
	differential_whisker.demote( diffgen );
	outcome.score = best_score;
      } else {
	assert( whiskers.replace( *best_whisker ) );
	//	printf( "Done with search.\n" );
	break;
      }
    }
  }

  /* carefully evaluate and return score */
  const Evaluator eval2( whiskers, _range );
  return eval2.score( {}, false, 10 );  
}
