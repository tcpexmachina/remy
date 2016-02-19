#include <cstdio>
#include <vector>
#include <string>
#include <limits>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "evaluator.hh"
#include "configvector.hh"
using namespace std;

int main( int argc, char *argv[] )
{
  WhiskerTree whiskers;
  RemyBuffers::ConfigVector config_vector;
  ConfigVector input_vector;
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

      if ( tree.has_config() ) {
	printf( "Prior assumptions:\n%s\n\n", tree.config().DebugString().c_str() );
      }

      if ( tree.has_optimizer() ) {
	printf( "Remy optimization settings:\n%s\n\n", tree.optimizer().DebugString().c_str() );
      }

    } else if ( arg.substr(0, 3 ) == "cf=" ) {
        config_filename = string( arg.substr( 3 ) );
        int cfd = open( config_filename.c_str(), O_RDONLY );
        if ( cfd < 0 ) {
          perror( "open config file error");
          exit( 1 );
        }
        if ( !config_vector.ParseFromFileDescriptor( cfd ) ) {
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
  input_vector.ReadVector(config_vector);
  Evaluator eval( input_vector );
  auto outcome = eval.score( whiskers, false, 10 );
  printf( "score = %f\n", outcome.score );
  double norm_score = 0;

  for ( auto &run : outcome.throughputs_delays ) {
    printf( "===\nconfig: %s\n", run.first.str().c_str() );
    for ( auto &x : run.second ) {
      printf( "sender: [tp=%f, del=%f]\n", x.first / run.first.link_ppt, x.second / run.first.delay );
      norm_score += log2( x.first / run.first.link_ppt ) - log2( x.second / run.first.delay );
    }
  }


  printf( "Whiskers: %s\n", outcome.used_whiskers.str().c_str() );

  return 0;
}
