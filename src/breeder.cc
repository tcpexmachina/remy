#include <cassert>
#include <limits>
#include <boost/accumulators/accumulators.hpp>
#include <boost/accumulators/statistics.hpp>
#include <boost/accumulators/statistics/tail_quantile.hpp>

#include "breeder.hh"

using namespace boost::accumulators;
using namespace std;

typedef accumulator_set< double, stats< tag::tail_quantile <boost::accumulators::right > > >
  accumulator_t_right;

template <typename T>
void Breeder< T >::apply_best_split( T & tree, const unsigned int generation ) const
{
  const Evaluator< T > eval( _options.config_range );
  auto outcome( eval.score( tree, true ) );

  while ( 1 ) {
    auto my_action( outcome.used_actions.most_used( generation ) );
    assert( my_action );

    T bisected_action( *my_action, true );

    if ( bisected_action.num_children() == 1 ) {
      printf( "Got unbisectable whisker! %s\n", my_action->str().c_str() );
      auto mutable_action ( *my_action );
      mutable_action.promote( generation + 1 );
      assert( outcome.used_actions.replace( mutable_action ) );
      continue;
    }

    printf( "Bisecting === %s === into === %s ===\n", my_action->str().c_str(), bisected_action.str().c_str() );
    assert( tree.replace( *my_action, bisected_action ) );
    break;
  }
}

template <typename T, typename A>
ActionImprover< T, A >::ActionImprover( const Evaluator< T > & s_evaluator,
				  const T & tree,
				  const double score_to_beat)
  : eval_( s_evaluator ),
    tree_( tree ),
    score_to_beat_( score_to_beat )
{}

template <typename T, typename A>
void ActionImprover< T, A >::evaluate_replacements(const vector<A> &replacements,
    vector< pair< const A &, future< pair< bool, double > > > > &scores,
    const double carefulness ) 
{
  for ( const auto & test_replacement : replacements ) {
    if ( eval_cache_.find( test_replacement ) == eval_cache_.end() ) {
      /* need to fire off a new thread to evaluate */
      scores.emplace_back( test_replacement,
                           async( launch::async, [] ( const Evaluator< T > & e,
                                                      const A & r,
                                                      const T & tree,
                                                      const double carefulness ) {
                                    T replaced_tree( tree );
                                    const bool found_replacement __attribute((unused)) = replaced_tree.replace( r );
                                    assert( found_replacement );
                                    return make_pair( true, e.score( replaced_tree, false, carefulness ).score ); },
                                  eval_, test_replacement, tree_, carefulness ) );
    } else {
      /* we already know the score */
      scores.emplace_back( test_replacement,
        async( launch::deferred, [] ( const double value ) {
               return make_pair( false, value ); }, eval_cache_.at( test_replacement ) ) );
    }
  } 
}

template <typename T, typename A>
vector<A> ActionImprover< T, A >::early_bail_out( const vector< A > &replacements,
        const double carefulness, const double quantile_to_keep )
{  
  vector< pair< const A &, future< pair< bool, double > > > > scores;
  evaluate_replacements( replacements, scores, carefulness );
  
  accumulator_t_right acc(
     tag::tail< boost::accumulators::right >::cache_size = scores.size() );
  vector<double> raw_scores;
  for ( auto & x : scores ) {
    const double score( x.second.get().second );
    acc( score );
    raw_scores.push_back( score );
  }
  
  /* Set the lower bound to be MAX_PERCENT_ERROR worse than the current best score */
  double lower_bound = std::min( score_to_beat_ * (1 + MAX_PERCENT_ERROR), 
        score_to_beat_ * (1 - MAX_PERCENT_ERROR) );
  /* Get the score at given quantile */
  double quantile_bound = quantile( acc, quantile_probability = 1 - quantile_to_keep );
  double cutoff = std::max( lower_bound, quantile_bound );

  /* Discard replacements below threshold */
  vector<A> top_replacements;
  for ( uint i = 0; i < scores.size(); i ++ ) {
    const A & replacement( scores.at( i ).first );
    const double score( raw_scores.at( i ) );
    if ( score >= cutoff ) {
      top_replacements.push_back( replacement );
    }
  }
  return top_replacements;
}

template <typename T, typename A>
double ActionImprover< T, A >::improve( A & action_to_improve )
{
  auto replacements = get_replacements( action_to_improve );
  vector< pair< const A &, future< pair< bool, double > > > > scores;

  /* Run for 10% simulation time to get estimates for the final score 
     and discard bad performing ones early on. */
  vector<A> top_replacements = early_bail_out( replacements, 0.1, 0.5 );

  /* find best replacement */
  evaluate_replacements( top_replacements, scores, 1 );
  for ( auto & x : scores ) {
     const A & replacement( x.first );
     const auto outcome( x.second.get() );
     const bool was_new_evaluation( outcome.first );
     const double score( outcome.second );

     /* should we cache this result? */
     if ( was_new_evaluation ) {
       eval_cache_.insert( make_pair( replacement, score ) );
     }

     if ( score > score_to_beat_ ) {
      score_to_beat_ = score;
      action_to_improve = replacement;
     }
  }

  cout << "Chose " << action_to_improve.str() << endl;

  return score_to_beat_;
}

template class ActionImprover< WhiskerTree, Whisker >;
template class ActionImprover< FinTree, Fin >;
template class Breeder< FinTree >;
template class Breeder< WhiskerTree >;
