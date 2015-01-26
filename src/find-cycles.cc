#include <cstdio>
#include <vector>
#include <string>
#include <google/dense_hash_map>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "sendergangofgangs.hh"
#include "simple-templates.cc"
#include "rat.hh"
#include "network.hh"
#include "network.cc"
#include "state.hh"
#include "whiskertree.hh"

using namespace std;

int main( int argc, char *argv[] )
{
  WhiskerTree whiskers;
  unsigned int num_senders = 1;
  double link_ppt = 2.0;
  double delay = 50.0;
  double mean_on_duration = 10000000.0;
  double mean_off_duration = 0.0;
  double imputed_delay = 1.0;
  double rewma = 1.0;
  unsigned int initial_buffer __attribute((unused));

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
    } else if ( arg.substr( 0, 4 ) == "del=" ) {
      imputed_delay = atof( arg.substr( 4 ).c_str() );
    } else if ( arg.substr( 0, 6 ) == "rewma=" ) {
      rewma = atof( arg.substr( 6 ).c_str() );
    } else if ( arg.substr( 0, 5 ) == "buff=" ) {
      initial_buffer = atoi( arg.substr( 5 ).c_str() );
    }
  }

  google::dense_hash_map< State, double, State::StateHash > state_set;
  state_set.set_empty_key( State ( std::vector<double> { -1 }, -1 ));
  
  PRNG prng( 50 );
  NetConfig configuration = NetConfig().set_link_ppt( link_ppt ).set_delay( delay ).set_num_senders( num_senders ).set_on_duration( mean_on_duration ).set_off_duration( mean_off_duration ); /* always on */
  Network<Rat, Rat> network( Rat( whiskers ), prng, configuration );
  network.mutable_senders().mutable_gang1().mutable_sender( 0 ).mutable_sender().set_initial_state( std::vector< double > { imputed_delay, rewma } );
  printf("Starting in initial state %f, %f\n", imputed_delay, rewma);

  double time = 0.0;
  double time_increment = 0.001;
  const double end_time = 100000000.0;
  State last_state;
  while ( time < end_time ) { 
    network.run_simulation_until( time );
    auto network_state = State( network.get_state( time ), time );

    if( network_state == last_state ) {
      /* Don't match if we haven't exited the state */
      last_state = network_state;
      time += time_increment;
      continue;
    }
    network_state.print_state();
    google::dense_hash_map< State, double, State::StateHash >::const_iterator match = state_set.find( network_state );
    if ( match != state_set.end() ) {
      cout << "Cycle length \t" << time - match->second << 
        " ms after \t" << match->second << " ms at time \t" << 
        time << endl;
      //break;
    }
    
    state_set[ network_state ] = time;
    last_state = network_state;
    time += time_increment;
  }

  return 0;
}
