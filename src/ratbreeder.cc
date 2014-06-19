#include <iostream>
#include <vector>
#include <cassert>
#include <limits>
#include <unistd.h>
#include <time.h>

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

    struct timespec tp_start, tp_end;
    clock_gettime( CLOCK_MONOTONIC_RAW, &tp_start );
    auto outcome( eval.score( whiskers ) );
    clock_gettime( CLOCK_MONOTONIC_RAW, &tp_end );
    uint32_t sleep_time_us = ( ( tp_end.tv_sec * 1000000 + tp_end.tv_nsec / 1000 ) -
                               ( tp_start.tv_sec * 1000000 + tp_start.tv_nsec / 1000 ));
    printf("One evaluation on one core took about %u usec\n", sleep_time_us);

    /* is there a whisker at this generation that we can improve? */
    auto most_used_whisker_ptr = outcome.used_whiskers.most_used( generation );

    /* if not, increase generation and promote all whiskers */
    if ( !most_used_whisker_ptr ) {
      generation++;
      whiskers.promote( generation );

      continue;
    }

    WhiskerImprover improver( eval, whiskers, outcome.score, sleep_time_us );

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
				  const double score_to_beat,
				  const uint32_t s_sleep_time_us )
  : eval_( s_evaluator ),
    rat_( rat ),
    score_to_beat_( score_to_beat ),
    http_server_( string( getenv( "PROBLEM_SERVER" ) ) + "/problem" ),
    http_host_( getenv( "HTTP_HOST" ) ),
    sleep_time_us_( s_sleep_time_us )
{
  assert( http_server_ != "" );
  assert( http_host_ != "" );
}

double WhiskerImprover::improve( Whisker & whisker_to_improve )
{
  auto replacements( whisker_to_improve.next_generation() );

  vector< tuple< Whisker, bool, string >> candidates;

  HttpTransmitter http_transmitter( http_server_ );

  /* Scatter/Map: Fire off evaluations for those that haven't been evaluated yet */
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
      headers[ "Host" ] = http_host_;
      auto http_response = http_transmitter.make_post_request( problem_str, headers );
      string problemid = get<0>( http_response );
      long status_code = get<1>( http_response );
      assert( problemid.size() == 32 ); /* Has to be an MD5 hash */
      assert( status_code == 200 ); /* Has to be 200 OK */
      //printf( "Posted problem with id %s\n", problemid.c_str() );
      candidates.emplace_back( test_replacement,
                               true,
                               problemid );
    } else {
      /* we already know the score */
      candidates.emplace_back( test_replacement,
                               false,
                               "" );
    }
  }

  /* Gather/Reduce: Keep asking PROBLEM_SERVER unless we get all answers */
  double score = score_to_beat_;
  while ( not candidates.empty() ) {
    /* wait a while, possibly for stragglers to finish */
    usleep( 2 * sleep_time_us_ );
    vector<tuple<Whisker, bool, string>>::iterator candidate_iterator = candidates.begin();
    while ( candidate_iterator != candidates.end() ) {
      if( evaluate_if_done( *candidate_iterator, http_transmitter ) ) {
        const Whisker & test_replacement( get<0>( *candidate_iterator ) );
        score  = eval_cache_.at( test_replacement );
        if ( score > score_to_beat_ ) {
          score_to_beat_ = score;
          whisker_to_improve = test_replacement;
        }
        candidate_iterator = candidates.erase( candidate_iterator );
      } else {
        candidate_iterator++;
        continue;
      }
    }
  }

  return score_to_beat_;
}

/* Do we have the answer now? */
bool WhiskerImprover::evaluate_if_done( std::tuple< Whisker, bool, string > & candidate,
                                        HttpTransmitter & http_transmitter )
{
    const Whisker & test_replacement( get<0>( candidate ) );
    const bool was_new_evaluation( get<1>( candidate ) );

    /* should we cache this result? */
    if ( was_new_evaluation ) {
      map<string, string> headers;
      headers[ "problemid" ] = get<2>( candidate );
      headers[ "Host" ] = http_host_;
      AnswerBuffers::Outcome answer_pb;
      auto http_response = http_transmitter.make_get_request( headers );
      string answer_pb_str = get<0>( http_response );
      long status_code = get<1>( http_response );

      /* Either processing or done */
      assert( status_code == 200 or status_code == 202 );

      if ( status_code == 200 ) {
        /* Make sure we received a valid protobuf */
        assert( answer_pb.ParseFromString( answer_pb_str ) );
        //printf( "Received answer for id %s\n", headers[ "problemid" ].c_str() );

        /* Insert into eval_cache_, but check that it doesn't already exist */
        assert( eval_cache_.find( test_replacement ) == eval_cache_.end() );
        eval_cache_.insert( make_pair( test_replacement, answer_pb.score() ) );
        get<1>( candidate ) = true;
        return true;
      } else {
        /* No dice, try again */
        return false;
      }
    } else {
      /* test_replacement must exist in cache */
      assert( eval_cache_.find( test_replacement ) != eval_cache_.end() );
      return true;
    }
}
