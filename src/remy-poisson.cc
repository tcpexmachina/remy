#include <cstdio>
#include <vector>
#include <string>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "fishbreeder.hh"
#include "dna.pb.h"
#include "configrange.hh"
using namespace std;

int main( int argc, char *argv[] )
{
  FinTree fins;
  string output_filename;
  BreederOptions options;
  RemyBuffers::ConfigRange input_config;
  string config_filename;

  for ( int i = 1; i < argc; i++ ) {
    string arg( argv[ i ] );
    if ( arg.substr( 0, 3 ) == "if=" ) {
      string filename( arg.substr( 3 ) );
      int fd = open( filename.c_str(), O_RDONLY );
      if ( fd < 0 ) {
	perror( "open" );
	exit( 1 );
      }

      RemyBuffers::FinTree tree;
      if ( !tree.ParseFromFileDescriptor( fd ) ) {
	fprintf( stderr, "Could not parse %s.\n", filename.c_str() );
	exit( 1 );
      }
      fins = FinTree( tree );

      if ( close( fd ) < 0 ) {
	perror( "close" );
	exit( 1 );
      }

    } else if ( arg.substr( 0, 3 ) == "of=" ) {
      output_filename = string( arg.substr( 3 ) );

    } else if ( arg.substr( 0, 3 ) == "cf=" ) {
      config_filename = string( arg.substr( 3 ) );
      int cfd = open( config_filename.c_str(), O_RDONLY );
      if ( cfd < 0 ) {
        perror( "open config file error");
        exit( 1 );
      }
      if ( !input_config.ParseFromFileDescriptor( cfd ) ) {
        fprintf( stderr, "Could not parse input config from file %s. \n", config_filename.c_str() );
        exit ( 1 );
      }
      if ( close( cfd ) < 0 ) {
        perror( "close" );
        exit( 1 );
      }
    }
  }

  if ( config_filename.empty() ) {
    fprintf( stderr, "An input configuration protobuf must be provided via the cf= option. \n");
    fprintf( stderr, "You can generate one using './configuration'. \n");
    exit ( 1 );
  }

  options.config_range = ConfigRange( input_config );

  FishBreeder breeder( options );

  unsigned int run = 0;

  printf( "#######################\n" );
  printf( "Evaluator simulations will run for %d ticks\n",
    options.config_range.simulation_ticks );
  printf( "Optimizing for link packets_per_ms in [%f, %f]\n",
	  options.config_range.link_ppt.low,
	  options.config_range.link_ppt.high );
  printf( "Optimizing for rtt_ms in [%f, %f]\n",
	  options.config_range.rtt.low,
	  options.config_range.rtt.high );
  printf( "Optimizing for num_senders in [%f, %f]\n",
	  options.config_range.num_senders.low, options.config_range.num_senders.high );
  printf( "Optimizing for mean_on_duration in [%f, %f], mean_off_duration in [ %f, %f]\n",
	  options.config_range.mean_on_duration.low, options.config_range.mean_on_duration.high, options.config_range.mean_off_duration.low, options.config_range.mean_off_duration.high );
  printf( "Optimizing for stochastic_loss_rate in [%f, %f]\n", options.config_range.stochastic_loss_rate.low, options.config_range.stochastic_loss_rate.high );
  if ( options.config_range.buffer_size.low != numeric_limits<unsigned int>::max() ) {
    printf( "Optimizing for buffer_size in [%f, %f]\n",
            options.config_range.buffer_size.low,
            options.config_range.buffer_size.high );
  } else {
    printf( "Optimizing for infinitely sized buffers. \n");
  }

  printf( "Initial rules (use if=FILENAME to read from disk): %s\n", fins.str().c_str() );
  printf( "#######################\n" );

  if ( !output_filename.empty() ) {
    printf( "Writing to \"%s.N\".\n", output_filename.c_str() );
  } else {
    printf( "Not saving output. Use the of=FILENAME argument to save the results.\n" );
  }

  RemyBuffers::ConfigVector training_configs;
  bool written = false;

  while ( 1 ) {
    auto outcome = breeder.improve( fins );
    printf( "run = %u, score = %f\n", run, outcome.score );

    printf( "fins: %s\n", fins.str().c_str() );

    for ( auto &run : outcome.throughputs_delays ) {
      if ( !(written) ) {
        for ( auto &run : outcome.throughputs_delays) {
          // record the config to the protobuf
          RemyBuffers::NetConfig* net_config = training_configs.add_config();
          *net_config = run.first.DNA();
          written = true;
      
        }
      }
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

      auto remycc = fins.DNA();
      remycc.mutable_config()->CopyFrom( options.config_range.DNA() );
      remycc.mutable_optimizer()->CopyFrom( Fin::get_optimizer().DNA() );
      remycc.mutable_configvector()->CopyFrom( training_configs );
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
