#include <stdio.h>
#include <vector>

#include "rat-templates.cc"
#include "random.hh"
#include "network.cc"

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

double evaluate( const Rat::Whiskers whiskers, const unsigned int seed )
{
  PRNG my_generator( seed );
  Network<Rat> network( whiskers, my_generator );

  for ( unsigned int i = 0; i < 1000000; i++ ) {
    network.tick();
  }

  return network.senders().utility();
}

int main( void )
{
  Rat::Whiskers whiskers;

  //  unsigned int generation = 0;

  while ( 1 ) {
    /* evaluate the whiskers we have */
    double score = evaluate( whiskers, global_PRNG()() );
    printf( "score: %f\n", score );

    /* is there a whisker at this generation that we can improve? */
  }

  return 0;
}
