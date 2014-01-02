#include <iostream>
#include <vector>
#include <cassert>
#include <future>
#include <limits>

#include "ratbreeder.hh"

using namespace std;

void RatBreeder::apply_best_split( WhiskerTree & whiskers1, WhiskerTree & whiskers2, const unsigned int generation, const ZigZag & tree_to_split ) const
{
  const Evaluator eval( whiskers1, whiskers2, _range );
  auto outcome( eval.score( {}, tree_to_split, true ) );

  while ( 1 ) {
    auto my_whisker( outcome.get_used_whiskers( tree_to_split ).most_used( generation ) );
    assert( my_whisker );

    WhiskerTree bisected_whisker( *my_whisker, true );

    if ( bisected_whisker.num_children() == 1 ) {
      //      printf( "Got unbisectable whisker! %s\n", my_whisker->str().c_str() );
      auto mutable_whisker( *my_whisker );
      mutable_whisker.promote( generation + 1 );
      assert( outcome.get_used_whiskers( tree_to_split ).replace( mutable_whisker ) );
      continue;
    }

    //    printf( "Bisecting === %s === into === %s ===\n", my_whisker->str().c_str(), bisected_whisker.str().c_str() );
    assert( (tree_to_split == ZigZag::ZIG ? whiskers1 : whiskers2).replace( *my_whisker, bisected_whisker ) );
    break;
  }
}

Evaluator::Outcome RatBreeder::improve( WhiskerTree & whiskers1, WhiskerTree & whiskers2 )
{
  /* back up the original whiskertree */
  /* this is to ensure we don't regress */
  WhiskerTree input_whiskertree1( whiskers1 );
  WhiskerTree input_whiskertree2( whiskers2 );

  /* evaluate the whiskers we have */
  whiskers1.reset_generation();
  whiskers2.reset_generation();
  unsigned int generation = 0;

  while ( generation < 5 ) {
    ZigZag tree_to_improve = ZigZag::ZIG; /* ZIG indicates tree 1, ZAG indicates tree 2 */
    while ( true ) {
      const Evaluator eval( whiskers1, whiskers2, _range );
      auto outcome( eval.score( {}, tree_to_improve ) );

      /* is there a whisker at this generation, for this whisker tree, that we can improve? */
      auto most_used_whisker_ptr = outcome.get_used_whiskers( tree_to_improve ).most_used( generation );

      /* if not, check the other whisker tree */
      if ( !most_used_whisker_ptr ) {
        tree_to_improve = not tree_to_improve;
        printf("Ran out of whiskers on tree, moving to other tree %d\n", tree_to_improve == ZigZag::ZIG ? 1 : 2);
        most_used_whisker_ptr = outcome.get_used_whiskers( tree_to_improve ).most_used( generation );
        /* If it's still null, we are done, promote */
        if ( !most_used_whisker_ptr ) {
          printf("Ran out of whiskers on both trees\n");
          assert( outcome.get_used_whiskers( ZigZag::ZIG ).most_used( generation ) == nullptr and
                  outcome.get_used_whiskers( ZigZag::ZAG ).most_used( generation ) == nullptr );
          generation++;
          whiskers1.promote( generation );
          whiskers2.promote( generation );
          break;
        }
      }

      assert ( most_used_whisker_ptr );
      WhiskerImprover improver( eval, outcome.score );

      Whisker whisker_to_improve = *most_used_whisker_ptr;

      printf( "Generation %d, improving whisker on tree %d\n", generation, tree_to_improve == ZigZag::ZIG ? 1 : 2 );

      double score_to_beat = outcome.score;
      while ( 1 ) {
        double new_score = improver.improve( whisker_to_improve, tree_to_improve );
        assert( new_score >= score_to_beat );
        if ( new_score == score_to_beat ) {
	  cerr << "Ending search." << endl;
	  break;
        } else {
	  cerr << "Score jumps from " << score_to_beat << " to " << new_score << endl;
	  score_to_beat = new_score;
        }
      }
      whisker_to_improve.demote( generation + 1 );

      const auto result = (tree_to_improve == ZigZag::ZIG ? whiskers1 : whiskers2).replace( whisker_to_improve );
      assert( result );

      /* zig-zag across the trees */
      tree_to_improve = not tree_to_improve;
    }
  }

  /* Split most used whisker for both trees */
  apply_best_split( whiskers1, whiskers2, generation, ZigZag::ZIG );
  apply_best_split( whiskers1, whiskers2, generation, ZigZag::ZAG );

  /* carefully evaluate what we have vs. the previous best */
  const Evaluator eval2( {}, {}, _range );
  const auto new_score = eval2.score( whiskers1, whiskers2, false, 10 );
  const auto old_score = eval2.score( input_whiskertree1, input_whiskertree2, false, 10 );

  if ( old_score.score >= new_score.score ) {
    fprintf( stderr, "Regression, old=%f, new=%f\n", old_score.score, new_score.score );
    whiskers1 = input_whiskertree1;
    whiskers2 = input_whiskertree2;
    return old_score;
  }

  return new_score;
}

WhiskerImprover::WhiskerImprover( const Evaluator & s_evaluator, const double score_to_beat )
  : eval_( s_evaluator ),
    score_to_beat_( score_to_beat )
{}

double WhiskerImprover::improve( Whisker & whisker_to_improve, const ZigZag & tree_to_improve )
{
  auto replacements( whisker_to_improve.next_generation() );

  vector< pair< const Whisker &, future< pair< bool, double > > > > scores;

  /* find best replacement */
  for ( const auto & test_replacement : replacements ) {
    if ( eval_cache_.find( test_replacement ) == eval_cache_.end() ) {
      /* need to fire off a new thread to evaluate */
      scores.emplace_back( test_replacement,
			   async( launch::async, [&tree_to_improve] ( const Evaluator & e, const Whisker & r ) {
			       return make_pair( true, e.score( { r }, tree_to_improve ).score ); }, eval_, test_replacement ) );
    } else {
      /* we already know the score */
      scores.emplace_back( test_replacement,
			   async( launch::deferred, [] ( const double value ) {
			       return make_pair( false, value ); }, eval_cache_.at( test_replacement ) ) );
    }
  }

  /* find the best one */
  for ( auto & x : scores ) {
    const Whisker & replacement( x.first );
    const auto outcome( x.second.get() );
    const bool was_new_evaluation( outcome.first );
    const double score( outcome.second );

    /* should we cache this result? */
    if ( was_new_evaluation ) {
      eval_cache_.insert( make_pair( replacement, score ) );
    }

    if ( score > score_to_beat_ ) {
      score_to_beat_ = score;
      whisker_to_improve = replacement;
    }
  }

  return score_to_beat_;
}
