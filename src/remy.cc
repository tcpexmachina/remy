#include <cstdio>
#include <vector>
#include <string>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <math.h>

#include "ratbreeder.hh"

using namespace std;

int main( int argc, char *argv[] )
{
  WhiskerTree whiskers;
  string output_filename;
  RatBreederOptions options;

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

    } else if ( arg.substr( 0, 4 ) == "opt=" ) {
      options.improver_options.optimize_window_increment = false;
      options.improver_options.optimize_window_multiple = false;
      options.improver_options.optimize_intersend = false;
      for (char & c : arg.substr( 4 )) {
        if (c == 'b') options.improver_options.optimize_window_increment = true;
        else if (c == 'm') options.improver_options.optimize_window_multiple = true;
        else if (c == 'r') options.improver_options.optimize_intersend = true;
        else {
          fprintf( stderr, "Invalid optimize option: %c\n", c );
          exit( 1 );
        }
      }
    }
  }

  options.config_range.link_packets_per_ms = make_pair( sqrt(0.1), sqrt(1000.0) ); /* 3.17 Mbps to 317 Mbps */
  options.config_range.rtt_ms = make_pair( 150, 150 ); /* ms */
  options.config_range.max_senders = 2;
  options.config_range.mean_on_duration = 1000;
  options.config_range.mean_off_duration = 1000;

  //  options.config_range.lo_only = true;
  RatBreeder breeder( options );

  unsigned int run = 0;

  printf( "#######################\n" );
  printf( "Optimizing for link packets_per_ms in [%f, %f]\n",
          options.config_range.link_packets_per_ms.first,
          options.config_range.link_packets_per_ms.second );
  printf( "Optimizing for rtt_ms in [%f, %f]\n",
          options.config_range.rtt_ms.first,
          options.config_range.rtt_ms.second );
  printf( "Optimizing for num_senders = 1-%d\n",
          options.config_range.max_senders );
  printf( "Optimizing for mean_on_duration = %f, mean_off_duration = %f\n",
          options.config_range.mean_on_duration, options.config_range.mean_off_duration );
  printf( "Optimizing window increment: %d, window multiple: %d, intersend: %d\n",
          options.improver_options.optimize_window_increment, options.improver_options.optimize_window_multiple,
          options.improver_options.optimize_intersend);

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
      remycc.mutable_config()->CopyFrom( options.config_range.DNA() );
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
