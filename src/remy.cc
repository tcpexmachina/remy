#include <cstdio>
#include <vector>
#include <string>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "ratbreeder.hh"

using namespace std;

int main( int argc, char *argv[] )
{
  WhiskerTree whiskers;
  string output_filename;

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
    }
  }

  //////// Initialize configurations to optimize for ////////

  ConfigRange range;
  range.link_packets_per_ms = make_pair( 1.0, 2.0 ); /* 10 Mbps to 20 Mbps */
  range.rtt_ms = make_pair( 100, 200 ); /* ms */
  range.max_senders = 16;
  range.mean_on_duration = 5000;
  range.mean_off_duration = 5000;
  //  range.lo_only = true;

  std::vector<NetConfig> configs;

  /* first load "anchors": the extreme points of the configuration range */
  configs.push_back( NetConfig().set_link_ppt( range.link_packets_per_ms.first ).set_delay( range.rtt_ms.first ).set_num_senders( range.max_senders ).set_on_duration( range.mean_on_duration ).set_off_duration( range.mean_off_duration ) );

  configs.push_back( NetConfig().set_link_ppt( range.link_packets_per_ms.first ).set_delay( range.rtt_ms.second ).set_num_senders( range.max_senders ).set_on_duration( range.mean_on_duration ).set_off_duration( range.mean_off_duration ) );
  configs.push_back( NetConfig().set_link_ppt( range.link_packets_per_ms.second ).set_delay( range.rtt_ms.first ).set_num_senders( range.max_senders ).set_on_duration( range.mean_on_duration ).set_off_duration( range.mean_off_duration ) );
  configs.push_back( NetConfig().set_link_ppt( range.link_packets_per_ms.second ).set_delay( range.rtt_ms.second ).set_num_senders( range.max_senders ).set_on_duration( range.mean_on_duration ).set_off_duration( range.mean_off_duration ) );

  /* now add some randomly generated configurations from the range*/
  for ( int i = 0; i < 12; i++ ) {
    configs.push_back( NetConfig( range )); // each will be initialized differently at evaluation time
  }

  RatBreeder breeder( configs );



  //////// Run the optimizer ////////

  unsigned int run = 0;

  printf( "#######################\n" );
  printf( "Optimizing for link packets_per_ms in [%f, %f]\n",
	  range.link_packets_per_ms.first,
	  range.link_packets_per_ms.second );
  printf( "Optimizing for rtt_ms in [%f, %f]\n",
	  range.rtt_ms.first,
	  range.rtt_ms.second );
  printf( "Optimizing for num_senders = 1-%d\n",
	  range.max_senders );
  printf( "Optimizing for mean_on_duration = %f, mean_off_duration = %f\n",
	  range.mean_on_duration, range.mean_off_duration );

  printf( "Initial rules (use if=FILENAME to read from disk): %s\n", whiskers.str().c_str() );
  printf( "#######################\n" );

  if ( !output_filename.empty() ) {
    printf( "Writing to \"%s.N\".\n", output_filename.c_str() );
  } else {
    printf( "Not saving output. Use the of=FILENAME argument to save the results.\n" );
  }

  while ( 1 ) {
    auto outcome = breeder.improve( whiskers );
    printf( "run = %u, score = %f\n", run, outcome.score );

    printf( "whiskers: %s\n", whiskers.str().c_str() );

    for ( auto &run : outcome.throughputs_delays ) {
      printf( "===\nconfig: %s\n", run.first.str().c_str() );
      for ( auto &x : run.second ) {
	printf( "sender: [tp=%f, del=%f]\n", x.first / run.first.link_ppt, x.second / run.first.delay );
      }
    }

    if ( !output_filename.empty() ) {
      char of[ 128 ];
      snprintf( of, 128, "%s.%d", output_filename.c_str(), run );
      fprintf( stderr, "Writing to \"%s\"... ", of );
      int fd = open( of, O_WRONLY | O_TRUNC | O_CREAT, S_IRUSR | S_IWUSR );
      if ( fd < 0 ) {
	perror( "open" );
	exit( 1 );
      }

      auto remycc = whiskers.DNA();
      remycc.mutable_config()->CopyFrom( range.DNA() );
      remycc.mutable_optimizer()->CopyFrom( Whisker::get_optimizer().DNA() );

      if ( not remycc.SerializeToFileDescriptor( fd ) ) {
	fprintf( stderr, "Could not serialize RemyCC.\n" );
	exit( 1 );
      }

      if ( close( fd ) < 0 ) {
	perror( "close" );
	exit( 1 );
      }

      fprintf( stderr, "done.\n" );
    }

    fflush( NULL );
    run++;
  }

  return 0;
}
