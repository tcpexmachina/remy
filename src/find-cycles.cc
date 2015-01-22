#include <cstdio>
#include <vector>
#include <string>
#include <google/dense_hash_set>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "evaluator.hh"
#include "state.hh"

using namespace std;

int main( int argc, char *argv[] )
{
  WhiskerTree whiskers;
  unsigned int num_senders = 2;
  double link_ppt = 1.0;
  double delay = 100.0;
  double mean_on_duration = 5000.0;
  double mean_off_duration = 5000.0;

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
    }
  }

  ConfigRange configuration_range;
  configuration_range.link_packets_per_ms = make_pair( link_ppt, 0 ); /* 1 Mbps to 10 Mbps */
  configuration_range.rtt_ms = make_pair( delay, 0 ); /* ms */
  configuration_range.max_senders = num_senders;
  configuration_range.mean_on_duration = mean_on_duration;
  configuration_range.mean_off_duration = mean_off_duration;
  configuration_range.lo_only = true;
  
  google::dense_hash_set< State, State::StateHash > state_set;
  state_set.set_empty_key( State ( std::vector<double> {-1, -1, -1 } ));

  state_set.insert( State(std::vector<double>{ 1, 2.21, 1 }) );

  google::dense_hash_set< State, State::StateHash >::const_iterator it
    = state_set.find( State( std::vector< double > { 1, 2.21, 1 } ));
  cout << (it != state_set.end() ? "present" : "not present")
       << endl;
  return 0;
}
