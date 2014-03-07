#include <cstdio>
#include <string>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "whiskertree.hh"
#include "network.cc"
#include "sendergangofgangs.hh"
#include "rat.hh"
#include "aimd-templates.cc"
#include "graph.hh"
#include "fader-templates.cc"

using namespace std;

int main( int argc, char *argv[] )
{
  WhiskerTree whiskers;
  unsigned int num_senders = 2;
  double link_ppt;
  double delay = 150.0;
  string fader_filename;

  for ( int i = 1; i < argc; i++ ) {
    string arg( argv[ i ] );
    if ( arg.substr( 0, 6 ) == "fader=" ) {
      fader_filename = arg.substr( 6 );
    } else if ( arg.substr( 0, 3 ) == "if=" ) {
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
    }
  }

  Fader fader( fader_filename );

  link_ppt = fader.link_rate();

  NetConfig configuration = NetConfig().set_link_ppt( link_ppt ).set_delay( delay ).set_num_senders( num_senders );

  PRNG prng( 50 );
  Network<SenderGang<Rat, ExternalSwitchedSender<Rat>>,
	  SenderGang<Aimd, ExternalSwitchedSender<Aimd>>> network( Rat( whiskers, false ), Aimd(), prng, configuration );

  float upper_limit = link_ppt * delay * 1.2;

  Graph graph( 2 * num_senders + 2, 1024, 600, "Ratatouille", 0, upper_limit );

  graph.set_color( 0, 0, 0, 0, 1.0 );
  graph.set_color( 1, 1, 0.38, 0, 0.8 );
  graph.set_color( 2, 0, 0.2, 1, 0.8 );
  graph.set_color( 3, 1, 0, 0, 0.8 );
  graph.set_color( 4, 0.5, 0, 0.5, 0.8 );
  graph.set_color( 5, 0, 0.5, 0.5, 0.8 );
  graph.set_color( 6, 0.5, 0.5, 0.5, 0.8 );
  graph.set_color( 7, 0.2, 0.2, 0.5, 0.8 );
  graph.set_color( 8, 0.2, 0.5, 0.2, 0.8 );

  float t = 0.0;

  while ( 1 ) {
    fader.update( network );

    network.mutable_link().set_rate( fader.link_rate() );
    network.mutable_link().set_limit( fader.buffer_size() );

    link_ppt = network.mutable_link().rate();

    char buf[ 256 ];
    snprintf( buf, 256, "link: %.1f Mbps, delay: %.0f ms, buffer: %.0f kB, active = %d RemyCC & %d AIMD",
	      link_ppt * 10, delay,
	      fader.buffer_size() * 1500.0 / 1000.0,
	      network.mutable_senders().mutable_gang1().count_active_senders(),
	      network.mutable_senders().mutable_gang2().count_active_senders() );

    graph.set_info( buf );

    if ( fader.autoscale() ) {
      upper_limit = link_ppt * delay * 1.2;
    }

    if ( fader.autoscale_all() ) {
      upper_limit = (fader.buffer_size() + link_ppt * delay) * 1.05;
    }

    network.run_simulation_until( t * 1000.0 );

    const vector< unsigned int > packets_in_flight = network.packets_in_flight();

    float ideal_pif = link_ppt * delay;

    graph.add_data_point( 0, t, ideal_pif );
    if ( ideal_pif > upper_limit ) {
      upper_limit = ideal_pif * 1.1;
    }

    graph.add_data_point( packets_in_flight.size() + 1, t, fader.buffer_size() + link_ppt * delay );

    for ( unsigned int i = 0; i < packets_in_flight.size(); i++ ) {
      graph.add_data_point( i + 1, t, packets_in_flight[ i ] );

      if ( packets_in_flight[ i ] > upper_limit ) {
	upper_limit = packets_in_flight[ i ] * 1.1;
      }
    }

    graph.set_window( t, fader.horizontal_size() * 1.5 );

    if ( graph.blocking_draw( t, fader.horizontal_size(), 0, upper_limit ) ) {
      break;
    }

    t += fader.time_increment();
  }

  for ( auto &x : network.senders().throughputs_delays() ) {
    printf( "sender: [tp=%f, del=%f]\n", x.first, x.second );
  }

  return EXIT_SUCCESS;
}
