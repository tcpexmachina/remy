#include <cstdio>
#include <vector>
#include <string>
#include <limits>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "evaluator.hh"
using namespace std;

template <typename T>
void print_tree(T & tree) 
{
  if ( tree.has_config() ) {
    printf( "Prior assumptions:\n%s\n\n", tree.config().DebugString().c_str() );
  }

  if ( tree.has_optimizer() ) {
    printf( "Remy optimization settings:\n%s\n\n", tree.optimizer().DebugString().c_str() );
  }
}

template <typename T>
void parse_outcome( T & outcome ) 
{
  printf( "score = %f\n", outcome.score );
  double norm_score = 0;

  for ( auto &run : outcome.throughputs_delays ) {
    printf( "===\nconfig: %s\n", run.first.str().c_str() );
    for ( auto &x : run.second ) {
      printf( "sender: [tp=%f, del=%f]\n", x.first / run.first.link_ppt, x.second / run.first.delay );
      norm_score += log2( x.first / run.first.link_ppt ) - log2( x.second / run.first.delay );
    }
  }

  printf( "normalized_score = %f\n", norm_score );

  printf( "Rules: %s\n", outcome.used_actions.str().c_str() );
}

int main( int argc, char *argv[] )
{
  WhiskerTree whiskers;
  FinTree fins;
  bool is_poisson = false;
  unsigned int num_senders = 2;
  double link_ppt = 1.0;
  double delay = 100.0;
  double mean_on_duration = 5000.0;
  double mean_off_duration = 5000.0;
  double buffer_size = numeric_limits<unsigned int>::max();
  unsigned int simulation_ticks = 1000000;
  string config_filename;

  for ( int i = 1; i < argc; i++ ) {
    string arg( argv[ i ] );
    if ( arg.substr( 0, 7) == "sender=" ) {
        string sender_type( arg.substr( 7 ) );
        if ( sender_type == "poisson" ) {
          is_poisson = true;
          fprintf( stderr, "Running poisson sender\n" );
        }
    } else if ( arg.substr( 0, 3 ) == "if=" ) {
      string filename( arg.substr( 3 ) );
      int fd = open( filename.c_str(), O_RDONLY );
      if ( fd < 0 ) {
        perror( "open" );
        exit( 1 );
      }

      if ( is_poisson ) {
        RemyBuffers::FinTree tree;
        if ( !tree.ParseFromFileDescriptor( fd ) ) {
          fprintf( stderr, "Could not parse %s.\n", filename.c_str() );
          exit( 1 );
        }
        fins = FinTree( tree );
        print_tree< RemyBuffers::FinTree >(tree);
      } else {
        RemyBuffers::WhiskerTree tree;
        if ( !tree.ParseFromFileDescriptor( fd ) ) {
          fprintf( stderr, "Could not parse %s.\n", filename.c_str() );
          exit( 1 );
        }
        whiskers = WhiskerTree( tree );  
        print_tree< RemyBuffers::WhiskerTree >(tree);
      }

      if ( close( fd ) < 0 ) {
        perror( "close" );
        exit( 1 );
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
    } else if ( arg.substr( 0, 4 ) == "buf=" ) {
      if (arg.substr( 4 ) == "inf") {
        buffer_size = numeric_limits<unsigned int>::max();
      } else {
        buffer_size = atoi( arg.substr( 4 ).c_str() );
      }
    }

 }
  ConfigRange configuration_range;
  configuration_range.configs.push_back(NetConfig().set_link_ppt(link_ppt).set_delay(delay).set_num_senders(num_senders).set_on_duration(mean_on_duration).set_off_duration(mean_off_duration).set_buffer_size(buffer_size));
  configuration_range.simulation_ticks = simulation_ticks;
  if ( is_poisson ) {
    Evaluator< FinTree > eval( configuration_range );
    auto outcome = eval.score( fins, false, 10 );
    parse_outcome< Evaluator< FinTree >::Outcome > ( outcome );
  } else {
    Evaluator< WhiskerTree > eval( configuration_range );
    auto outcome = eval.score( whiskers, false, 10 );
    parse_outcome< Evaluator< WhiskerTree >::Outcome > ( outcome );
  }



  return 0;
}
