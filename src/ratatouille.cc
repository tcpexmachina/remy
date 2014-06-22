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
#include "figure.hh"
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

      printf( "Whiskertree: %s\n", whiskers.str().c_str());
    }
  }

  Fader fader( fader_filename );

  link_ppt = fader.link_rate();

  NetConfig configuration = NetConfig().set_link_ppt( link_ppt ).set_delay( delay ).set_num_senders( num_senders );

  PRNG prng( 50 );
  Network<SenderGang<Rat, ExternalSwitchedSender<Rat>>,
	  SenderGang<Aimd, ExternalSwitchedSender<Aimd>>> network( Rat( whiskers, false ), Aimd(), prng, configuration );

  float upper_limit = link_ppt * delay * 1.2;

  // packets in flight graph //
  Figure fig( 1024, 768, "Ratatouille" );
  unsigned int sg_id = fig.add_subgraph( 2 * num_senders + 4, 
                                         "time (s)", "packets in flight", 
                                         0, upper_limit );
  unsigned int rtt_id = fig.add_subgraph( num_senders+1, "time (s)", "rttr", 
                                          0, 2 );
  unsigned int rewma_id = fig.add_subgraph( num_senders+1, "time (s)", "rewma",
                                            0, 2 );
  unsigned int sewma_id = fig.add_subgraph( num_senders+1, "time (s)", "sewma",
                                            0, 2 );
  unsigned int slow_id = fig.add_subgraph( num_senders+1, "time (s)", "slow_rewma", 
                                           0, 2 );

  fig.set_line_color( sg_id, 0, 0, 0, 0, 1.0 );
  fig.set_line_color( sg_id, 1, 1, 0.38, 0, 0.8 );
  fig.set_line_color( sg_id, 2, 0, 0.2, 1, 0.8 );
  fig.set_line_color( sg_id, 3, 1, 0, 0, 0.8 );
  fig.set_line_color( sg_id, 4, 0.5, 0, 0.5, 0.8 );
  fig.set_line_color( sg_id, 5, 0, 0.5, 0.5, 0.8 );
  fig.set_line_color( sg_id, 6, 0.5, 0.5, 0.5, 0.8 );
  fig.set_line_color( sg_id, 7, 0.2, 0.2, 0.5, 0.8 );
  fig.set_line_color( sg_id, 8, 0.2, 0.5, 0.2, 0.8 );
  fig.set_line_color( sg_id, 9, 0.5, 0.2, 0.5, 0.8 );

  fig.set_line_color( rewma_id, 0, 0, 0, 0, 1.0 );
  fig.set_line_color( rewma_id, 1, 1, 0.38, 0, 0.8 );
  fig.set_line_color( rewma_id, 2, 0, 0.2, 1, 0.8 );
  fig.set_line_color( rewma_id, 3, 1, 0, 0, 0.8 );
  fig.set_line_color( rewma_id, 4, 0.5, 0, 0.5, 0.8 );

  fig.set_line_color( rtt_id, 0, 0, 0, 0, 1.0 );
  fig.set_line_color( rtt_id, 1, 1, 0.38, 0, 0.8 );
  fig.set_line_color( rtt_id, 2, 0, 0.2, 1, 0.8 );
  fig.set_line_color( rtt_id, 3, 1, 0, 0, 0.8 );
  fig.set_line_color( rtt_id, 4, 0.5, 0, 0.5, 0.8 );

  fig.set_line_color( sewma_id, 0, 0, 0, 0, 1.0 );
  fig.set_line_color( sewma_id, 1, 1, 0.38, 0, 0.8 );
  fig.set_line_color( sewma_id, 2, 0, 0.2, 1, 0.8 );
  fig.set_line_color( sewma_id, 3, 1, 0, 0, 0.8 );
  fig.set_line_color( sewma_id, 4, 0.5, 0, 0.5, 0.8 );

  fig.set_line_color( slow_id, 0, 0, 0, 0, 1.0 );
  fig.set_line_color( slow_id, 1, 1, 0.38, 0, 0.8 );
  fig.set_line_color( slow_id, 2, 0, 0.2, 1, 0.8 );
  fig.set_line_color( slow_id, 3, 1, 0, 0, 0.8 );
  fig.set_line_color( slow_id, 4, 0.5, 0, 0.5, 0.8 );

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

    //graph.set_info( buf );

    if ( fader.autoscale() ) {
      upper_limit = link_ppt * delay * 1.2;
    }

    if ( fader.autoscale_all() ) {
      upper_limit = (fader.buffer_size() + link_ppt * delay) * 1.05;
    }

    network.run_simulation_until( t * 1000.0 );

    const vector< unsigned int > packets_in_flight = network.packets_in_flight();

    float ideal_pif = link_ppt * delay;

    fig.add_data_point( sg_id, 0, t, ideal_pif );
    if ( ideal_pif > upper_limit ) {
      upper_limit = ideal_pif * 1.1;
    }

    fig.add_data_point( sg_id, packets_in_flight.size() + 1, t, fader.buffer_size() + link_ppt * delay );

    // total number of packets in flight
    fig.add_data_point( sg_id, packets_in_flight.size() + 2, t, 
                        std::accumulate(packets_in_flight.begin(),
                                        packets_in_flight.end(),
                                        0) );
    
    // current ewmas for sender 1
    for( unsigned int i = 0; i < network.mutable_senders().mutable_gang1().count_active_senders(); i++ ) {
      fig.add_data_point( rewma_id, i+1, t,
                          network.mutable_senders().mutable_gang1().
                          mutable_sender( i ).mutable_sender().
                          current_memory().field( 1 ) );
      fig.add_data_point( sewma_id, i+1, t,
                          network.mutable_senders().mutable_gang1().
                          mutable_sender( i ).mutable_sender().
                          current_memory().field( 0 ) );
      fig.add_data_point( rtt_id, i+1, t,
                          network.mutable_senders().mutable_gang1().
                          mutable_sender( i ).mutable_sender().
                          current_memory().field( 2 ) );
      fig.add_data_point( slow_id, i+1, t,
                          network.mutable_senders().mutable_gang1().
                          mutable_sender( i ).mutable_sender().
                          current_memory().field( 3 ) );
    }

    for ( unsigned int i = 0; i < packets_in_flight.size(); i++ ) {
      fig.add_data_point( sg_id, i + 1, t, packets_in_flight[ i ] );

      if ( packets_in_flight[ i ] > upper_limit ) {
	upper_limit = packets_in_flight[ i ] * 1.1;
      }
    }

    fig.set_subgraph_window( sg_id, t, fader.horizontal_size() * 1.5 );
    fig.set_subgraph_ylimits( sg_id, 0, upper_limit );

    if ( fig.blocking_draw( t, fader.horizontal_size() )) {
      break;
    }

    t += fader.time_increment();
  }

  for ( auto &x : network.senders().throughputs_delays() ) {
    printf( "sender: [tp=%f, del=%f]\n", x.first, x.second );
  }

  return EXIT_SUCCESS;
}
