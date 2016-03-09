#include <cstdio>
#include <string>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "fintree.hh"
#include "whiskertree.hh"
#include "network.cc"
#include "sendergangofgangs.hh"
#include "fish.hh"
#include "rat.hh"
#include "fish-templates.cc"
#include "aimd-templates.cc"
#include "graph.hh"
#include "fader-templates.cc"
#include "rat-templates.cc"
#include "bb100.hh"

using namespace std;

typedef SenderGang<Aimd, ExternalSwitchedSender<Aimd>> AimdGang;
typedef SenderGang<Fish, ExternalSwitchedSender<Fish>> FishGang;
typedef SenderGang<Rat, ExternalSwitchedSender<Rat>> RatGang;

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
void simulate(T & network, Graph & graph, GTKFader & fader)
{
  float t = 0.0;

  float upper_limit = fader.link_rate() * fader.rtt() * 1.2;

  while ( true ) {
    if ( fader.quit() ) {
      break;
    }

    fader.update( network );

    network.mutable_link().set_rate( fader.link_rate() );
    network.mutable_link().set_limit( fader.buffer_size() );
    network.mutable_delay().set_delay( fader.rtt() );

    double link_ppt = network.mutable_link().rate();
    double delay = network.mutable_delay().delay();

    char buf[ 256 ];
    snprintf( buf, 256, "link: %.1f Mbps, delay: %.0f ms, buffer: %.0f pkts, active = %d RemyCC & %d AIMD",
        link_ppt * 10, delay,
        fader.buffer_size(),
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
}


int main( int argc, char *argv[] )
{
  unsigned int num_senders = 4;
  const int RGB_base[6][3] = {
    {1, 0, 0}, {0, 1, 0}, {0, 0, 1}, {1, 1, 0}, {1, 0, 1}, {0, 1, 1}
  };

  bool is_poisson = false;

  RemyBuffers::WhiskerTree default_tree;
  if ( not default_tree.ParseFromArray( bigbertha_100x_dna_5, bigbertha_100x_dna_5_len ) ) {
    fprintf( stderr, "Could not parse default RemyCC DNA" );
    exit( 1 );
  }

  WhiskerTree whiskers( default_tree );
  FinTree fins;

  for ( int i = 1; i < argc && !is_poisson; i++ ) {
    string arg( argv[ i ] );
    if ( arg.substr( 0, 7) == "sender=" ) {
        string sender_type( arg.substr( 7 ) );
        if ( sender_type == "poisson" ) {
          is_poisson = true;
          fprintf( stderr, "Running poisson sender\n" );
        }
    }
  }
  for ( int i = 1; i < argc; i++ ) {
    string arg( argv[ i ] );
    if ( arg.substr( 0, 3 ) == "if=" ) {
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
    }
  }

  unsigned int num_lines = 2 * num_senders + 2;

  Graph graph( num_lines, 950, 600, "Ratatouille", 0, 1 );

  GTKFader fader( num_senders );

  NetConfig configuration = NetConfig().set_link_ppt( fader.link_rate() ).set_delay( fader.rtt() ).set_num_senders( num_senders );

  PRNG prng( 50 );
  unsigned int fish_prng_seed( prng() );

  graph.set_color( 0, 0, 0, 0, 1.0 );
  double m = 1.0;
  unsigned int i = 1;
  while ( i < num_lines ) {
    for ( unsigned int j = 0; j < 6 && i < num_lines; j ++ ) {
      graph.set_color(i, m * RGB_base[j][0], m * RGB_base[j][1], m * RGB_base[j][2], 0.8);
      i ++;
    }
    m /= 2;
  }

  if ( is_poisson ) {
    Network< FishGang, AimdGang > network( Fish( fins, fish_prng_seed, false ), Aimd(), prng, configuration );
    simulate<Network< FishGang, AimdGang >>(network, graph, fader);
  } else {
    Network< RatGang, AimdGang > network( Rat( whiskers, false ), Aimd(), prng, configuration );
    simulate<Network< RatGang, AimdGang >>(network, graph, fader);
  }

  return EXIT_SUCCESS;
}
