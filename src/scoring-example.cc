#include <cstdio>
#include <vector>
#include <string>
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

  // save problem to file
  ProblemBuffers::Problem serialized_problem = eval.DNA( whiskers );
  string problem_filename = "problem_example.dna";

  char of_p[ 128 ];
  snprintf( of_p, 128, "%s", problem_filename.c_str() );
  fprintf( stderr, "Writing to \"%s\"... ", of_p );
  int fd = open( of_p, O_WRONLY | O_TRUNC | O_CREAT, S_IRUSR | S_IWUSR );
  if ( fd < 0 ) {
    perror( "open" );
    exit( 1 );
  }

  if ( not serialized_problem.SerializeToFileDescriptor( fd ) ) {
    fprintf( stderr, "Could not serialize problem.\n" );
    exit( 1 );
  }

  if ( close( fd ) < 0 ) {
    perror( "close" );
    exit( 1 );
  }

  cerr << "Successfully stored problem." << endl;

  // parse file and score
  fd = open( problem_filename.c_str(), O_RDONLY );
  if ( fd < 0 ) {
    perror( "open" );
    exit( 1 );
  }

  ProblemBuffers::Problem problem;
  if ( !problem.ParseFromFileDescriptor( fd ) ) {
    fprintf( stderr, "Could not parse %s.\n", problem_filename.c_str() );
    exit( 1 );
  }

  auto outcome = Evaluator::parse_problem_and_evaluate( problem );

  cerr << "Successfully scored problem." << endl;

  // save score to file
  string answer_filename = "answer_example.dna";

  AnswerBuffers::Outcome serialized_answer = outcome.DNA();
  char of_a[ 128 ];
  snprintf( of_a, 128, "%s", answer_filename.c_str() );
  fprintf( stderr, "Writing to \"%s\"... ", of_a );
  fd = open( of_a, O_WRONLY | O_TRUNC | O_CREAT, S_IRUSR | S_IWUSR );
  if ( fd < 0 ) {
    perror( "open" );
    exit( 1 );
  }

  if ( not serialized_answer.SerializeToFileDescriptor( fd ) ) {
    fprintf( stderr, "Could not serialize outcome.\n" );
    exit( 1 );
  }

  if ( close( fd ) < 0 ) {
    perror( "close" );
    exit( 1 );
  }

  cerr << "Successfully stored answer." << endl;

  // read score from file

  fd = open( answer_filename.c_str(), O_RDONLY );
  if ( fd < 0 ) {
    perror( "open" );
    exit( 1 );
  }

  AnswerBuffers::Outcome proto_outcome;
  if ( !proto_outcome.ParseFromFileDescriptor( fd ) ) {
    fprintf( stderr, "Could not parse %s.\n", answer_filename.c_str() );
    exit( 1 );
  }

  Evaluator::Outcome parsed_outcome( proto_outcome );
  printf( "score = %f\n", outcome.score );
  double norm_score = 0;

  for ( auto &run : parsed_outcome.throughputs_delays ) {
    printf( "===\nconfig: %s\n", run.first.str().c_str() );
    for ( auto &x : run.second ) {
      printf( "sender: [tp=%f, del=%f]\n", x.first / run.first.link_ppt, x.second / run.first.delay );
      norm_score += log2( x.first / run.first.link_ppt ) - log2( x.second / run.first.delay );
    }
  }

  printf( "normalized_score = %f\n", norm_score );

  printf( "Whiskers: %s\n", outcome.used_whiskers.str().c_str() );

  return 0;
}
