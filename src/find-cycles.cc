#include <cstdio>
#include <cmath>
#include <vector>
#include <string>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <getopt.h>
#include <boost/functional/hash.hpp>
#include <unordered_map>
#include <unordered_set>
#include <iomanip>
#include <limits>

#include "exception.hh"
#include "sendergangofgangs.hh"
#include "rat.hh"
#include "aimd.hh"
#include "aimd-templates.cc"
#include "network.hh"
#include "network.cc"
#include "whiskertree.hh"
#include "utility.hh"
#include "cycle-finder.hh"
#include "cycle-finder.cc"

using namespace std;

void usage_error( const string & program_name )
{
    throw Exception( "Usage", program_name + " " );
}

int main( int argc, char *argv[] )
{
  WhiskerTree whiskers;
  unsigned int num_senders = 2;
  double link_ppt = 1.0;
  double delay = 50.0;
  double rewma __attribute((unused)) = 1.0;
  double sender_offset = 0.0;
  unsigned int initial_buffer = 0;
  bool aimd __attribute((unused)) = false;
  bool verbose = false;

  try {
    if ( argc < 3 ) {
      usage_error( argv[ 0 ] );
    }
  
    const option command_line_options[] = {
      { "infile",         required_argument, nullptr, 'i' },
      { "num_senders",    required_argument, nullptr, 'n' },
      { "link_ppt",       required_argument, nullptr, 'l' },
      { "rtt",            required_argument, nullptr, 'r' },
      { "initial_buffer", required_argument, nullptr, 'b' },
      { "offset",         required_argument, nullptr, 'o' },
      { "aimd",                 no_argument, nullptr, 'a' },
      { "verbose",              no_argument, nullptr, 'v' },
      { 0,                                0, nullptr,   0 }
    };

    while ( true ) {
      const int opt = getopt_long( argc, argv, "i:n:l:r:b:o:v", command_line_options, nullptr );
      if ( opt == -1 ) { /* end of options */
        break;
      }
    
      switch ( opt ) {
      case 'i':
        {
          string filename( optarg );
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
        }
        break;
      case 'n':
        num_senders = atoi( optarg );
        break;
      case 'l':
        link_ppt = atof( optarg );
        break;
      case 'r':
        delay = atof( optarg );
        break;
      case 'b':
        initial_buffer = atoi( optarg );
        break;
      case 'o':
        sender_offset = atof( optarg );
        break;
      case 'a':
        aimd = true;
        break;
      case 'v':
        verbose = true;
        break;
      case '?':
        usage_error( argv[ 0 ] );
        break;
      default:
        throw Exception( "getopt_long", "unexpected return value " + to_string( opt ) );
      }
    }
  } catch ( const Exception & e ) {
    e.perror();
    return EXIT_FAILURE;
  }

  PRNG prng( 50 );
  NetConfig configuration = NetConfig().
    set_link_ppt( link_ppt ).
    set_delay( delay ).
    set_num_senders( num_senders ).
    set_start_buffer( initial_buffer );

  Network<Rat, Rat> network( Rat( whiskers ), prng, configuration );
  //Network<Aimd, Aimd> network( Aimd(), prng, configuration );

  network.mutable_senders().mutable_gang1().mutable_sender( 0 ).switch_on( network.tickno() );
  
  if ( num_senders > 1 ) {
    network.run_simulation_until( network.tickno() + sender_offset );
    for( size_t i = 1; i < num_senders; i++ ) {
      network.mutable_senders().
        mutable_gang1().
        mutable_sender( i ).
        switch_on( network.tickno() );
    }
  }

  CycleFinder<Rat, Rat> initial_cycle( network, sender_offset );
  initial_cycle.run_until_cycle_found( verbose );
  initial_cycle.print_all_statistics();

  return 0;
  if ( num_senders <= 1 ) return 0;
  cout << endl;

  /* Now find cycles for each sender turning off. */
  double first_cycle_end = initial_cycle.cycle_start().tickno() + initial_cycle.cycle_len();

  /* Sender 1 turns off */
  Network<Rat, Rat> off1_network( network );
  for ( double offset = sender_offset; offset < first_cycle_end; offset += 1 ) {
    Network<Rat, Rat> off1_network_offset( off1_network );
    off1_network_offset.run_simulation_until( off1_network.tickno() + offset );

    off1_network_offset.mutable_senders().
      mutable_gang1().
      mutable_sender( 0 ).
      switch_off( off1_network_offset.tickno(),
                  network.mutable_senders().mutable_gang1().
                  count_active_senders() );
    
    try {
      CycleFinder<Rat, Rat> offset1( off1_network_offset, offset );
      offset1.run_until_cycle_found( verbose );
      offset1.print_all_statistics();
    } catch ( const Exception & e ) {
      cout << "Failed on offset " << offset << endl;
      continue;
    }
  }

  cout << endl;

  /* Sender 2 turns off */
  Network<Rat, Rat> off2_network( network );
  for ( double offset = sender_offset; offset < first_cycle_end; offset += 1 ) {
    Network<Rat, Rat> off2_network_offset( off2_network );
    off2_network_offset.run_simulation_until( off2_network.tickno() + offset );

    off2_network_offset.mutable_senders().
      mutable_gang1().
      mutable_sender( 1 ).
      switch_off( off2_network_offset.tickno(),
                  network.mutable_senders().mutable_gang1().
                  count_active_senders() );
    
    try {
      CycleFinder<Rat, Rat> offset2( off2_network_offset, offset );
      offset2.run_until_cycle_found();
      offset2.print_all_statistics();
    } catch ( const Exception & e ) {
      cout << "Failed on offset " << offset << endl;
      continue;
    }
  }

  return 0;
}
