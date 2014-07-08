#include <cstdio>
#include <string>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <algorithm>
#include <cmath>

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
                                         0, upper_limit, 1000.0 );
  unsigned int sewma_rewma_id = fig.add_subgraph( 3*(num_senders+1), 
                                                "sewma", "rewma", 
                                                0, 2, 2.0 );
  unsigned int slow_rttr_id = fig.add_subgraph( 3*(num_senders+1), 
                                                "slow_rewma", "rttr", 
                                                0, 2, 10.0 );

  std::vector< unsigned int > subfig_ids = { sewma_rewma_id, slow_rttr_id };

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

  for( const unsigned int x : subfig_ids ) {
    fig.set_line_color( x, 0, 0, 0, 0, 1.0 );
    fig.set_line_color( x, 1, 1, 0.38, 0, 0.8 );
    fig.set_line_color( x, 2, 0, 0.2, 1, 0.8 );
    fig.set_line_color( x, 3, 1, 0, 0, 0.8 );
    fig.set_line_color( x, 4, 0.5, 0, 0.5, 0.8 );
    fig.set_line_color( x, 5, 0, 0.5, 0.5, 0.8 );
    fig.set_line_color( x, 6, 0.5, 0.5, 0.5, 0.8 );
    fig.set_line_color( x, 7, 0.2, 0.2, 0.5, 0.8 );
    fig.set_line_color( x, 8, 0.2, 0.5, 0.2, 0.8 );
    fig.set_line_color( x, 9, 0.5, 0.2, 0.5, 0.8 );
  }

  float t = 0.0;

  std::vector< std::pair< double, double > > subfig_limits;
  for( unsigned int i = 0; i < subfig_ids.size(); i++ ) {
    subfig_limits.emplace_back( std::pair< double, double >( 0, 0 ) );
  }

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

    fig.set_subgraph_info( sg_id, buf );

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

    // current queue size
    fig.add_data_point( sg_id, packets_in_flight.size() + 2, t, 
                        network.mutable_link().buffer_size() );

    std::vector< std::pair< double, double > > new_subfig_limits( subfig_limits.size() );
    for( unsigned int i = 0; i < subfig_ids.size(); i++ ) {
      new_subfig_limits.emplace_back( std::pair< double, double >( 0, 0 ) );
    }

    // current ewmas
    for( unsigned int i = 0; i < network.mutable_senders().mutable_gang1().count_active_senders(); i++ ) {
      const Memory & sender_memory = network.mutable_senders().
        mutable_gang1().mutable_sender( i ).mutable_sender().current_memory();
      const MemoryRange & sender_domain( whiskers.
                                         use_whisker( sender_memory,
                                                      false ).domain() );
      
      fig.add_data_point( sewma_rewma_id, i+1,
                          sender_memory.field( 1 ), 
                          sender_memory.field( 0 ) );
      new_subfig_limits.at( 0 ).second = 
        max( min( sender_memory.field( 0 ) + 2, 
                  double(sender_domain.upper().field( 0 )) ),
             new_subfig_limits.at( 0 ).second )*0.8;
      fig.set_subgraph_xrange( sewma_rewma_id, std::pair<float, float>( 0.0, 1.5 ));

      fig.add_data_point( slow_rttr_id, i+1,
                          sender_memory.field( 3 ), 
                          sender_memory.field( 2 ) );
      new_subfig_limits.at( 1 ).second = 
        max( min( sender_memory.field( 2 ) + 2, 
                  double(sender_domain.upper().field( 2 )) ),
             new_subfig_limits.at( 2 ).second )*0.8;
      fig.set_subgraph_xrange( slow_rttr_id, std::pair<float, float>( 0.0, 1.0 ));
    }

    for( unsigned int i = 0; i < subfig_limits.size(); i++ ) {
      subfig_limits.at( i ).second = new_subfig_limits.at( i ).second +
        (subfig_limits.at( i ).second)*0.2;
    }

    for ( unsigned int i = 0; i < packets_in_flight.size(); i++ ) {
      fig.add_data_point( sg_id, i + 1, t, packets_in_flight[ i ] );

      if ( packets_in_flight[ i ] > upper_limit ) {
	upper_limit = packets_in_flight[ i ] * 1.1;
      }
    }

    fig.set_subgraph_ylimits( sg_id, 0, upper_limit );

    if( network.mutable_senders().mutable_gang1().count_active_senders() > 0 ) {
    for ( unsigned int i = 0; i < subfig_limits.size(); i++ ) 
      {
        fig.set_subgraph_ylimits( subfig_ids.at ( i ), 
                                  subfig_limits.at( i ).first, 
                                  subfig_limits.at( i ).second );
      }  
    }

    fig.set_subgraph_xrange( sg_id, std::pair<float, float>( (float)max( 0.0, (double)t-10 ), t ));

    if ( fig.blocking_draw()) {
      break;
    }

    t += fader.time_increment();
  }

  for ( auto &x : network.senders().throughputs_delays() ) {
    printf( "sender: [tp=%f, del=%f]\n", x.first, x.second );
  }

  return EXIT_SUCCESS;
}
