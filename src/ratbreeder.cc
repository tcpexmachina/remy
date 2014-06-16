#include <iostream>
#include <vector>
#include <cassert>
#include <limits>

#include "http_transmitter.hh"
#include "ratbreeder.hh"

using namespace std;

void RatBreeder::apply_best_split( WhiskerTree & whiskers, const unsigned int generation ) const
{
  const Evaluator eval( _net_configs );
  auto outcome( eval.score( whiskers, true ) );

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

RatBreeder::RatBreeder( const vector<NetConfig> & s_net_configs )
  : _net_configs( s_net_configs )
{}


Evaluator::Outcome RatBreeder::improve( WhiskerTree & whiskers )
{
  /* back up the original whiskertree */
  /* this is to ensure we don't regress */
  WhiskerTree input_whiskertree( whiskers );

  /* evaluate the whiskers we have */
  whiskers.reset_generation();
  unsigned int generation = 0;

  while ( generation < 5 ) {
    const Evaluator eval( _net_configs );

    auto outcome( eval.score( whiskers ) );

    /* is there a whisker at this generation that we can improve? */
    auto most_used_whisker_ptr = outcome.used_whiskers.most_used( generation );

    /* if not, increase generation and promote all whiskers */
    if ( !most_used_whisker_ptr ) {
      generation++;
      whiskers.promote( generation );

      continue;
    }

    WhiskerImprover improver( eval, whiskers, outcome.score );

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
  const Evaluator eval2( _net_configs );
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
				  const double score_to_beat )
  : eval_( s_evaluator ),
    rat_( rat ),
    score_to_beat_( score_to_beat )
{}

double WhiskerImprover::improve( Whisker & whisker_to_improve )
{
  auto replacements( whisker_to_improve.next_generation() );

  vector< tuple< const Whisker &, bool, string >> candidates;

  string http_server ( getenv( "PROBLEM_SERVER" ) );
  assert( http_server != "" );
  string http_host ( getenv( "HTTP_HOST" ) );
  assert( http_host != "" );
  HttpTransmitter http_request( http_server );
  /* find best replacement */
  for ( const auto & test_replacement : replacements ) {
    if ( eval_cache_.find( test_replacement ) == eval_cache_.end() ) {
      /* haven't evaluated yet, POST for evaluation */
      WhiskerTree replaced_whiskertree( rat_ );
      const bool found_replacement = replaced_whiskertree.replace( test_replacement );
      assert( found_replacement );
      auto problem_pb = eval_.bundle_up( replaced_whiskertree );
      string problem_str {};
      assert( problem_pb.SerializeToString( &problem_str ) );

      /* Set up POST and block for response */
      map<string, string> headers;
      headers[ "Host" ] = http_host;
      auto problem_id = http_request.make_post_request( problem_str, headers );
      assert( problem_id.size() == 32 ); /* Has to be an MD5 hash */
      candidates.emplace_back( test_replacement,
                               true,
                               problem_id );
    } else {
      /* we already know the score */
      candidates.emplace_back( test_replacement,
                               false,
                               "" );
    }
  }

  /* find the best one */
  for ( auto & x : candidates ) {
    const Whisker & test_replacement( get<0>( x ) );
    const bool was_new_evaluation( get<1>( x ) );
    double score;

    /* should we cache this result? */
    if ( was_new_evaluation ) {
      map<string, string> headers;
      headers[ "problem_id" ] = get<2>( x );
      headers[ "Host" ] = http_host;
      AnswerBuffers::Outcome answer_pb;
      assert( answer_pb.ParseFromString( http_request.make_get_request( headers ) ) );
      score  = answer_pb.score();
      eval_cache_.insert( make_pair( test_replacement, answer_pb.score() ) );
    } else {
      score  = eval_cache_.at( test_replacement );
    }

    if ( score > score_to_beat_ ) {
      score_to_beat_ = score;
      whisker_to_improve = test_replacement;
    }
  }

  return score_to_beat_;
}
