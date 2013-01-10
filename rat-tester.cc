#include <stdio.h>
#include <vector>

#include "rat-templates.cc"
#include "random.hh"
#include "network.cc"

using namespace std;

void summarize( const Network<Rat> & network )
{
  printf( "util=%9.5f", network.senders().utility() );

  auto tds = network.senders().throughputs_delays();
  for ( auto &x : tds ) {
    printf( "    [ tp=%.4f del=%.4f ]", x.first, x.second );
  }
  printf( "\n\n" );

  auto senders = network.senders().senders();
  for ( auto &x : senders ) {
    printf( "Whiskers:" );
    for ( auto &y : x->whiskers().whiskers() ) {
      printf( " %s", y.summary().c_str() );
    }
    printf( "\n\n" );
  }
}

int main( void )
{
  unsigned int TICK_COUNT = 1000000;

  Rat::Whiskers whiskers;

  unsigned int generation = 0;

  while ( 1 ) {
    /* evaluate the whiskers we have */
    const unsigned int random_seed = global_PRNG()();

    /* run with existing whiskers */
    Network<Rat> network( whiskers, random_seed );
    network.tick( TICK_COUNT );
    printf( "gen %d, score = %.12f\n", generation, network.senders().utility() );

    summarize( network );

    /* is there a whisker at this generation that we can improve? */
    auto my_sender( network.senders().senders()[ 0 ] );
    auto my_whisker( my_sender->whiskers().most_used( generation ) );

    /* if not, increase generation and promote all whiskers */
    if ( !my_whisker ) {
      generation++;
      printf( "Advancing to generation %d\n", generation );
      whiskers.promote( generation );
      continue;
    }

    /* otherwise, get all nexgen alternatives for that whisker and replace each in turn */
    auto replacements( my_whisker->next_generation() );
    double best_score = -INT_MAX;
    const Rat::Whisker *best_whisker = nullptr;

    /* find best case (using same randseed) */
    for ( const auto &test_replacement : replacements ) {
      auto new_whiskers( whiskers );
      new_whiskers.replace( test_replacement );
      Network<Rat> test_network( new_whiskers, random_seed );
      test_network.tick( TICK_COUNT );

      const double score( test_network.senders().utility() );
      if ( score > best_score ) {
	best_whisker = &test_replacement;
	best_score = score;
      }
    }

    assert( best_whisker );

    /* replace with best nexgen choice and repeat */
    printf( "Replacing with whisker that scored %.12f\n", best_score );
    whiskers.replace( *best_whisker );
  }

  return 0;
}
