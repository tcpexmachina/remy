#include <iostream>
#include <cassert>
#include <limits>
#include <boost/accumulators/accumulators.hpp>
#include <boost/accumulators/statistics.hpp>
#include <boost/accumulators/statistics/tail_quantile.hpp>

#include "ratbreeder.hh"

using namespace boost::accumulators;
using namespace std;

typedef accumulator_set< double, stats< tag::tail_quantile <boost::accumulators::right > > >
  accumulator_t_right;

void RatBreeder::apply_best_split( WhiskerTree & whiskers, const unsigned int generation ) const
{
  const Evaluator eval( _options.config_range );
  auto outcome( eval.score( whiskers, true ) );

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

    printf( "Bisecting === %s === into === %s ===\n", my_whisker->str().c_str(), bisected_whisker.str().c_str() );
    assert( whiskers.replace( *my_whisker, bisected_whisker ) );
    break;
  }
}

Evaluator::Outcome RatBreeder::improve( WhiskerTree & whiskers )
{
  /* back up the original whiskertree */
  /* this is to ensure we don't regress */
  WhiskerTree input_whiskertree( whiskers );

  /* evaluate the whiskers we have */
  whiskers.reset_generation();
  unsigned int generation = 0;

  while ( generation < 5 ) {
    const Evaluator eval( _options.config_range );

    auto outcome( eval.score( whiskers ) );

    /* is there a whisker at this generation that we can improve? */
    auto most_used_whisker_ptr = outcome.used_whiskers.most_used( generation );

    /* if not, increase generation and promote all whiskers */
    if ( !most_used_whisker_ptr ) {
      generation++;
      whiskers.promote( generation );

      continue;
    }

    WhiskerImprover improver( eval, whiskers, _options.improver_options, outcome.score );

    Whisker whisker_to_improve = *most_used_whisker_ptr;

    double score_to_beat = outcome.score;

    while ( 1 ) {
      double new_score = improver.improve( whisker_to_improve );
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

    const auto result __attribute((unused)) = whiskers.replace( whisker_to_improve );
    assert( result );
  }

  /* Split most used whisker */
  apply_best_split( whiskers, generation );

  /* carefully evaluate what we have vs. the previous best */
  const Evaluator eval2( _options.config_range );
  const auto new_score = eval2.score( whiskers, false, 10 );
  const auto old_score = eval2.score( input_whiskertree, false, 10 );

  if ( old_score.score >= new_score.score ) {
    fprintf( stderr, "Regression, old=%f, new=%f\n", old_score.score, new_score.score );
    whiskers = input_whiskertree;
    return old_score;
  }

  return new_score;
}

WhiskerImprover::WhiskerImprover( const Evaluator & s_evaluator,
				  const WhiskerTree & rat,
          const WhiskerImproverOptions & options,
				  const double score_to_beat)
  : eval_( s_evaluator ),
    rat_( rat ),
    options_( options ),
    score_to_beat_( score_to_beat )
{}

void WhiskerImprover::evaluate_replacements(const vector<Whisker> &replacements,
    vector< pair< const Whisker &, future< pair< bool, double > > > > &scores,
    const double carefulness) {
  for ( const auto & test_replacement : replacements ) {
    if ( eval_cache_.find( test_replacement ) == eval_cache_.end() ) {
      /* need to fire off a new thread to evaluate */
      scores.emplace_back( test_replacement,
               async( launch::async, [] ( const Evaluator & e,
                              const Whisker & r,
                              const WhiskerTree & rat,
                              const double carefulness ) {
                    WhiskerTree replaced_whiskertree( rat );
                    const bool found_replacement __attribute((unused)) = replaced_whiskertree.replace( r );
                    assert( found_replacement );
                    return make_pair( true, e.score( replaced_whiskertree, false, carefulness ).score ); },
                  eval_, test_replacement, rat_, carefulness ) );
    } else {
      /* we already know the score */
      scores.emplace_back( test_replacement,
               async( launch::deferred, [] ( const double value ) {
                   return make_pair( false, value ); }, eval_cache_.at( test_replacement ) ) );
    }
  } 
}

double WhiskerImprover::improve( Whisker & whisker_to_improve )
{
  auto replacements( whisker_to_improve.next_generation( options_.optimize_window_increment,
                                                         options_.optimize_window_multiple,
                                                         options_.optimize_intersend ) );
  vector< pair< const Whisker &, future< pair< bool, double > > > > scores;

  /* Run for 5% simulation time to get estimates for the final score */
  evaluate_replacements(replacements, scores, 0.05);
  accumulator_t_right acc(
    tag::tail< boost::accumulators::right >::cache_size = scores.size() );
  vector<double> raw_scores;
  for ( auto & x : scores ) {
    const double score( x.second.get().second );
    acc(score);
    raw_scores.push_back(score);
  }
  /* Keep only the top OPTIMIZATION_FACTOR of the replacements */
  double cutoff = quantile(acc, quantile_probability = 1- OPT_FACTOR );
  vector<Whisker> top_replacements;
  for ( uint i = 0; i < scores.size(); i ++ ) {
    const Whisker & replacement( scores.at(i).first );
    const double score( raw_scores.at(i) );
    if ( score >= cutoff ) {
      top_replacements.push_back(replacement);
    }
  }

  /* find best replacement */
  scores.clear();
  evaluate_replacements(top_replacements, scores, 1);
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

  cout << "Chose " << whisker_to_improve.str() << endl;

  return score_to_beat_;
}
