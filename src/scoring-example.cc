#include <cstdio>
#include <vector>
#include <string>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <iostream>

#include "evaluator.hh"
#include "configrange.hh"
using namespace std;


int main( int argc, char *argv[] )
{
  WhiskerTree whiskers;
  unsigned int num_senders = 2;
  double link_ppt = 1.0;
  double delay = 100.0;
  double mean_on_duration = 5000.0;
  double mean_off_duration = 5000.0;
  double stochastic_loss_rate = 0;
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
    } else if ( arg.substr( 0, 5 ) == "nsrc=" ) {
      num_senders = atoi( arg.substr( 5 ).c_str() );
      fprintf( stderr, "Setting num_senders to %d\n", num_senders );
    } else if ( arg.substr( 0, 5 ) == "link=" ) {
      link_ppt = atof( arg.substr( 5 ).c_str() );
      fprintf( stderr, "Setting link packets per ms to %f\n", link_ppt );
    } else if ( arg.substr( 0, 4 ) == "rtt=" ) {
      delay = atof( arg.substr( 4 ).c_str() );
      fprintf( stderr, "Setting delay to %f ms\n", delay );
    } else if ( arg.substr( 0, 3 ) == "on=" ) {
      mean_on_duration = atof( arg.substr( 3 ).c_str() );
      fprintf( stderr, "Setting mean_on_duration to %f ms\n", mean_on_duration );
    } else if ( arg.substr( 0, 4 ) == "off=" ) {
      mean_off_duration = atof( arg.substr( 4 ).c_str() );
      fprintf( stderr, "Setting mean_off_duration to %f ms\n", mean_off_duration );
    } else if ( arg.substr(0, 6 ) == "sloss=" ) {
      stochastic_loss_rate = atof( arg.substr( 6 ).c_str() );
      fprintf( stderr, "Setting stochastic loss rate to %f\n", stochastic_loss_rate );
    }
  }

  ConfigRange configuration_range;
  configuration_range.link_ppt = Range( link_ppt,link_ppt, 0 ); /* 1 Mbps to 10 Mbps */
  configuration_range.rtt = Range( delay, delay, 0 ); /* ms */
  configuration_range.num_senders = Range(num_senders, num_senders, 0 );
  configuration_range.mean_on_duration = Range(mean_on_duration, mean_on_duration, 0);
  configuration_range.mean_off_duration = Range(mean_off_duration, mean_off_duration, 0);
  configuration_range.stochastic_loss_rate = Range(stochastic_loss_rate, stochastic_loss_rate, 0);
  Evaluator< WhiskerTree > eval( configuration_range );

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

  auto outcome = Evaluator< WhiskerTree >::parse_problem_and_evaluate( problem );

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

  Evaluator< WhiskerTree >::Outcome parsed_outcome( proto_outcome );
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

  printf( "Whiskers: %s\n", outcome.used_actions.str().c_str() );

  return 0;
}
