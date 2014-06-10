#include <cstdio>
#include <vector>
#include <string>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "dna.pb.h"
#include "ratbreeder.hh"

using namespace std;

int main( int argc, char *argv[] )
{
  WhiskerTree whiskers;
  RemyBuffers::Scenarios scenarios;
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
    } else if ( arg.substr( 0, 10 ) == "scenarios=" ) {
      string scenarios_file = string( arg.substr( 10 ) );
      int fd = open( scenarios_file.c_str(), O_RDONLY );
      if ( fd < 0 ) {
        perror( "scenarios_file open" );
        exit( 1 );
      }

      if ( !scenarios.ParseFromFileDescriptor( fd ) ) {
	fprintf( stderr, "Could not parse %s.\n", scenarios_file.c_str() );
	exit( 1 );
      }

      if ( close( fd ) < 0 ) {
	perror( "close" );
	exit( 1 );
      }
    }
  }

  assert( scenarios.configs_size() > 0 );
  std::vector<NetConfig> net_configs;
  for ( auto &x: scenarios.configs() ) {
    net_configs.emplace_back( x );
  }

  RatBreeder breeder( net_configs );

  unsigned int run = 0;

  cout << "######scenarios are####\n";
  cout << scenarios.DebugString();
  cout << "#######################\n";

  assert ( !output_filename.empty() );
  printf( "Writing to \"%s.N\".\n", output_filename.c_str() );

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

    /* Write output file */
    char of[ 128 ];
    snprintf( of, 128, "%s.%d", output_filename.c_str(), run );
    fprintf( stderr, "Writing to \"%s\"... ", of );
    int fd = open( of, O_WRONLY | O_TRUNC | O_CREAT, S_IRUSR | S_IWUSR );
    if ( fd < 0 ) {
      perror( "open" );
      exit( 1 );
    }

    auto remycc = whiskers.DNA();
    remycc.mutable_scenarios()->CopyFrom( scenarios );
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

    fflush( NULL );
    run++;
  }
  return 0;
}
