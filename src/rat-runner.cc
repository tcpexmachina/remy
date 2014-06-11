#include <cstdio>
#include <vector>
#include <string>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "problem.pb.h"
#include "evaluator.hh"

using namespace std;

int main( int argc, char *argv[] )
{
  WhiskerTree whiskers;
  std::string problem_str = "";

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
    } else if ( arg.substr( 0, 8 ) == "problem=" ) {
      problem_str = arg.substr( 8 ).c_str();
      fprintf( stderr, "Setting problem_str to %s ms\n", problem_str.c_str() );
    }
  }

  assert( problem_str != "" );

  /* Setup and run problem_instance */
  int fd = open( problem_str.c_str(), O_RDONLY );
  if ( fd < 0 ) {
    perror( "open" );
    exit( 1 );
  }

  ProblemBuffers::Problem problem_instance;
  if ( !problem_instance.ParseFromFileDescriptor( fd ) ) {
    fprintf( stderr, "Could not parse %s.\n", problem_str.c_str() );
    exit( 1 );
  }

  /* Get whiskers */
  WhiskerTree run_whiskers( problem_instance.whiskers() );
  assert( problem_instance.scenarios().configs_size() > 0 );

  /* Get Scenarios into vector of config */
  std::vector<NetConfig> net_configs;
  for ( auto &x: problem_instance.scenarios().configs() ) {
    net_configs.emplace_back( x );
  }

  Evaluator eval( net_configs,
                  problem_instance.settings().prng_seed(),
                  problem_instance.settings().tick_count() );
  auto outcome = eval.score( run_whiskers );
  auto answer = outcome.DNA();
  printf( "%s\n", answer.DebugString().c_str() );
  printf( "Whiskers: %s\n", outcome.used_whiskers.str().c_str() );

  return 0;
}
