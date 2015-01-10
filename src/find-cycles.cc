#include <cstdio>
#include <string>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <algorithm>
#include <cmath>

#include "whiskertree.hh"
#include "network.hh"
#include "network.cc"
#include "sendergangofgangs.hh"
#include "rat.hh"
#include "aimd-templates.cc"
#include "figure.hh"

using namespace std;

int main( int argc, char *argv[] )
{
  WhiskerTree whiskers;
  unsigned int num_senders = 2;
  double link_ppt;
  double delay = 150.0;
  double sewma = 1.0;
  double rewma = 1.0;
  double rtt_ratio = 1.0;
  double slow_rewma = 1.0;
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

    } else if ( arg.substr( 0, 6 ) == "rewma=" ) {
      rewma = atof( arg.substr( 6 ).c_str() );
    } else if ( arg.substr( 0, 6 ) == "sewma=" ) {
      sewma = atof( arg.substr( 6 ).c_str() );
    } else if ( arg.substr( 0, 5 ) == "rttr=" ) {
      rtt_ratio = atof( arg.substr( 5 ).c_str() );
    } else if ( arg.substr( 0, 5 ) == "slow=" ) {
      slow_rewma = atof( arg.substr( 5 ).c_str() );
    } else if ( arg.substr( 0, 5 ) == "buff=" ) {
      initial_buffer = atoi( arg.substr( 5 ).c_str() );
    } 
  }

  link_ppt = 1.5;
  unsigned int buffer_size = link_ppt * 150 * 10;

  NetConfig configuration = NetConfig().set_link_ppt( link_ppt ).set_delay( delay ).set_num_senders( num_senders );

  PRNG prng( 50 );
  Network<SenderGang<Rat, ExternalSwitchedSender<Rat>>,
	  SenderGang<Aimd, ExternalSwitchedSender<Aimd>>> network( Rat( whiskers, false ), Aimd(), prng, configuration );
  
  network.mutable_senders().mutable_gang1().mutable_sender( 0 ).switch_on( network.tickno() );

  network.mutable_link().set_rate( link_ppt );
  network.mutable_link().set_limit( buffer_size );

  link_ppt = network.mutable_link().rate();

  std::vector< std::pair< StatePoint, int > > cycle;

  printf("Start configuration: sewma=%f, rewma=%f, rttr=%f, slowrewma=%f\n", sewma, rewma, rtt_ratio, 
         slow_rewma);

  network.run_simulation_with_config( 1.0, sewma, rewma, rtt_ratio, slow_rewma, 0 ); 
  network.run_simulation_until( 15000.0 );

  const auto & current_history = network.history();
    
  assert ( !current_history.empty() );
  
  bool cycle_found = false;
  for ( unsigned int i = 0; i < current_history.size()/2; i++ ) {
    auto & curr = current_history.at( i );

    bool found = false;
    for ( int j = cycle.size()-1; j >= 0 ; j-- ) {
      if ( (curr == cycle.at( j ).first) and (curr._tickno - cycle.at( j ).first._tickno > 50.0) ) {
        cycle.at( j ).second++;
        found = true;
        cycle_found = true;
        break;
      }
    }

    if ( !found and !cycle_found ) {
      cycle.emplace_back( curr, 1 );
    } else if ( !found ) {
      cycle_found = false;
      cycle.clear();
      cycle.emplace_back( curr, 1 );
    }
  }

  printf("Cycle:\n");
  for ( auto & pt : cycle ) {
    printf("\t%s, count %d\n", pt.first.str().c_str(), pt.second);
  }

  for ( auto &x : network.senders().throughputs_delays() ) {
    printf( "sender: [tp=%f, del=%f]\n", x.first, x.second );
  }

  return EXIT_SUCCESS;
}
