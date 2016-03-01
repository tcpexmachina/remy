#include <iostream>

#include "fishbreeder.hh"

using namespace std;

Evaluator< FinTree >::Outcome FishBreeder::improve( FinTree & fins )
{
  /* back up the original fintree */
  /* this is to ensure we don't regress */
  FinTree input_fintree( fins );

  /* evaluate the fins we have */
  fins.reset_generation();
  unsigned int generation = 0;

  while ( generation < 5 ) {
    const Evaluator< FinTree > eval( _options.config_range );

    auto outcome( eval.score( fins ) );

    /* is there a fin at this generation that we can improve? */
    auto most_used_fin_ptr = outcome.used_actions.most_used( generation );

    /* if not, increase generation and promote all fins */
    if ( !most_used_fin_ptr ) {
      generation++;
      fins.promote( generation );

      continue;
    }

    FinImprover improver( eval, fins, outcome.score );

    Fin fin_to_improve = *most_used_fin_ptr;

    double score_to_beat = outcome.score;

    while ( 1 ) {
      double new_score = improver.improve( fin_to_improve );
      assert( new_score >= score_to_beat );
      if ( new_score == score_to_beat ) {
        cerr << "Ending search." << endl;
        break;
      } else {
        cerr << "Score jumps from " << score_to_beat << " to " << new_score << endl;
        score_to_beat = new_score;
      }
    }

    fin_to_improve.demote( generation + 1 );

    const auto result __attribute((unused)) = fins.replace( fin_to_improve );
    assert( result );
  }

  /* Split most used whisker */
  apply_best_split( fins, generation );

  /* carefully evaluate what we have vs. the previous best */
  const Evaluator< FinTree > eval2( _options.config_range );
  const auto new_score = eval2.score( fins, false, 10 );
  const auto old_score = eval2.score( input_fintree, false, 10 );

  if ( old_score.score >= new_score.score ) {
    fprintf( stderr, "Regression, old=%f, new=%f\n", old_score.score, new_score.score );
    fins = input_fintree;
    return old_score;
  }

  return new_score;
}

vector< Fin > FinImprover::get_replacements( Fin & action_to_improve ) 
{
  return action_to_improve.next_generation();
}
