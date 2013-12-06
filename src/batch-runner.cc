#include <stdio.h>
#include <stdlib.h>
#include <vector>
#include <string>
#include <sstream>
#include <math.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <fstream>

#include <boost/accumulators/accumulators.hpp>
#include <boost/accumulators/statistics/mean.hpp>
#include <boost/accumulators/statistics/variance.hpp>

#include "configrange.hh"
#include "evaluator.hh"
#include "ratbreeder.hh"
#include "constants.hh"

using namespace std;
using namespace boost::accumulators;

WhiskerTree train_remy(std::vector<NetConfig> configs, int iterations) {

  WhiskerTree whiskers;

  RatBreeder breeder( configs );

  unsigned int run = 0;

  printf( "\n********** Training **********\n" );

  const NetConfig defaultnet;

  printf( "Optimizing for mean_on_duration = %f, mean_off_duration = %f\n",
	  defaultnet.mean_on_duration, defaultnet.mean_off_duration );

  printf( "Not saving output! Use remy.cc to save output.\n");

  for(int i = 0; i < iterations; i++) {
    auto outcome = breeder.improve( whiskers );
    printf( "run = %u, score = %f\n", run, outcome.score );

    for ( auto &run : outcome.throughputs_delays ) {
      printf( "===\nconfig: %s\n", run.first.str().c_str() );
      for ( auto &x : run.second ) {
	printf( "sender: [tp=%f, del=%f]\n", x.first / run.first.link_ppt, x.second / run.first.delay );
      }
    }

    fflush( NULL );
    run++;
  }

  return whiskers;
}


/*
  Test a configuration on the given set of whiskers.
  Return the normalized score, average delays, and average throughputs.
*/
void test_remy( WhiskerTree whiskers, std::vector<NetConfig> configs,
		vector<double> * avg_delays, vector<double> * avg_tps, 
		double * norm_score  ) {

  printf("\n*********** Testing ***********\n");

  Evaluator eval( whiskers, configs );
  auto outcome = eval.score( {}, false, 10 );
  printf( "score = %f\n", outcome.score );
  *norm_score = 0;
  
  for ( auto &run : outcome.throughputs_delays ) {
    printf( "===\nconfig: %s\n", run.first.str().c_str() );
    for ( auto &x : run.second ) {
      printf( "sender: [tp=%f, del=%f]\n", x.first / run.first.link_ppt, x.second / run.first.delay );
      avg_tps->push_back(x.first/run.first.link_ppt);
      avg_delays->push_back(x.second/run.first.delay);
      *norm_score += log2( x.first / run.first.link_ppt ) - log2( x.second / run.first.delay );
    }
  }

  printf( "normalized_score = %f\n", *norm_score );
}


int main( int argc, char *argv[] )
{
  WhiskerTree whiskers;
  string output_filename;
  unsigned int num_senders = 2;
  double link_ppt = 1.0;
  double delay = 100.0;

  for ( int i = 1; i < argc; i++ ) {
    string arg( argv[ i ] );
    if ( arg.substr( 0, 3 ) == "if=" ) {
      string filename( arg.substr( 3 ) );
      int fd = open( filename.c_str(), O_RDONLY );
      if ( fd < 0 ) {
	perror( "open" );
	exit( 1 );
      }

      RemyBuffers::WhiskerTree tree;
      if ( !tree.ParseFromFileDescriptor( fd ) ) {
	fprintf( stderr, "Could not parse %s.\n", filename.c_str() );
	exit( 1 );
      }
      whiskers = WhiskerTree( tree );

      if ( close( fd ) < 0 ) {
	perror( "close" );
	exit( 1 );
      }
    } else if ( arg.substr( 0, 3 ) == "of=" ) {
      output_filename = string( arg.substr( 3 ) );
    } else if ( arg.substr( 0, 5 ) == "nsrc=" ) {
      num_senders = atoi( arg.substr( 5 ).c_str() );
      fprintf( stderr, "Setting num_senders to %d\n", num_senders );
    } else if ( arg.substr( 0, 5 ) == "link=" ) {
      link_ppt = atof( arg.substr( 5 ).c_str() );
      fprintf( stderr, "Setting link packets per ms to %f\n", link_ppt );
    } else if ( arg.substr( 0, 4 ) == "rtt=" ) {
      delay = atof( arg.substr( 4 ).c_str() );
      fprintf( stderr, "Setting delay to %f ms\n", delay );
    } else if ( arg.substr( 0, 5 ) == "axes=" ) {
      // comma-separated values, no spaces
      g_constants.axis_values.clear();
      stringstream ss(arg.substr(5));
      string next;
      while(getline(ss, next, ',')){
	g_constants.axis_values.push_back(atoi(next.c_str()));
      }
    }
  }

  ConfigRange range;
  range.link_packets_per_ms = make_pair( link_ppt, 0 ); /* 10 Mbps to 20 Mbps */
  range.rtt_ms = make_pair( delay, 0 ); /* ms */
  range.max_senders = num_senders;
  range.lo_only = true;

  std::vector<NetConfig> configs;

  configs.push_back( NetConfig().set_link_ppt( range.link_packets_per_ms.first ).set_delay( range.rtt_ms.first ).set_num_senders( range.max_senders ) );

  configs.push_back( NetConfig().set_link_ppt( range.link_packets_per_ms.first ).set_delay( range.rtt_ms.first ).set_num_senders( 1 ) );

  configs.push_back( NetConfig().set_link_ppt( range.link_packets_per_ms.first ).set_delay( range.rtt_ms.first ).set_num_senders( 2 ).set_on_duration( 500 ).set_off_duration( 500 ) );

  printf( "Optimizing for link packets_per_ms in [%f, %f], ",
	  range.link_packets_per_ms.first,
	  range.link_packets_per_ms.second );
  printf( "rtt_ms in [%f, %f], ",
	  range.rtt_ms.first,
	  range.rtt_ms.second );
  printf( "num_senders = 1-%d.\n",
	  range.max_senders );

  ///// Train Remy /////

  WhiskerTree trained_whiskers;
  trained_whiskers = train_remy(configs, 15);


  ///// Test Remy /////
  
  vector<accumulator_set<double, stats<tag::variance> > > scores;
  vector<accumulator_set<double, stats<tag::variance> > > tps;
  vector<accumulator_set<double, stats<tag::variance> > > delays;
  for ( unsigned int i = 0; i < configs.size(); i++ ) {
    accumulator_set<double, stats<tag::variance> > score_acc;
    accumulator_set<double, stats<tag::variance> > tp_acc;
    accumulator_set<double, stats<tag::variance> > delay_acc;
    scores.push_back(score_acc);
    tps.push_back(tp_acc);
    delays.push_back(delay_acc);
  }

  for(int i = 0; i < 10; i++){
    for ( unsigned int j = 0; j < configs.size(); j++ ) {
      vector<NetConfig> single_config = { configs[j] };
      vector<double> avg_delays;
      vector<double> avg_throughputs;
      double norm_score;
      test_remy(trained_whiskers, single_config, & avg_delays, & avg_throughputs, 
		& norm_score);
      scores[j](norm_score);
      tps[j](avg_throughputs[0]);
      delays[j](avg_delays[0]);
    }
  }

  if ( !output_filename.empty() ) {
    string curr_outfile = output_filename;
    for( auto val : g_constants.axis_values) {
      curr_outfile += "." + to_string(val);
    }

    char of[ 128 ];
    snprintf( of, 128, "%s", curr_outfile.c_str());
    
    fprintf( stderr, "Writing to \"%s\"... ", of );
    FILE* outfile = fopen(of, "a");
  
    for ( unsigned int i = 0; i < configs.size(); i++ ) {
      printf("\n***** Stats for config %s\nNormalized score: mean %f, stdev %f\nThroughputs: mean %f, stdev %f\nDelays: mean %f, stdev %f\n\n", 
	     configs[i].str().c_str(), 
	     extract::mean(scores[i]), sqrt(extract::variance(scores[i])), 
	     extract::mean(tps[i]), sqrt(extract::variance(tps[i])), 
	     extract::mean(delays[i]), sqrt(extract::variance(delays[i])));
      
      fprintf(outfile, "\n***** Stats for config %s\nNormalized score: mean %f, stdev %f\nThroughputs: mean %f, stdev %f\nDelays: mean %f, stdev %f\n\n", 
	      configs[i].str().c_str(), 
	      extract::mean(scores[i]), sqrt(extract::variance(scores[i])), 
	      extract::mean(tps[i]), sqrt(extract::variance(tps[i])), 
	      extract::mean(delays[i]), sqrt(extract::variance(delays[i])));
    }
    
  }

  return 0;
}
