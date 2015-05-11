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

using namespace std;

const double END_TIME = 5000000.0;

typedef int64_t quantized_t;
const double quantizer = 10000000;

void usage_error( const string & program_name )
{
    throw Exception( "Usage", program_name + " " );
}

vector<quantized_t> all_down( const vector<double> & state ) {
  vector<quantized_t> ret;
  for ( const auto & x : state ) {
    ret.push_back( x * quantizer );
  }
  return ret;
}

vector<vector<quantized_t>> fuzz_state( const vector<quantized_t> & state_all_down ) {
  vector<vector<quantized_t>> ret { state_all_down };

  /* bisect in each axis */
  for ( unsigned int i = 0; i < state_all_down.size(); i++ ) {
    //if ( i == 4 or i == 9 or i == 10 or i == 11 or i == 12 or i == 13 ) continue;
    continue;
    //if ( not ( i == 2 or i == 5 ) ) continue; 

    vector<vector<quantized_t>> new_ret;

    for ( const auto & x : ret ) {
      /* make up version */
      auto x_upped = x;
      auto x_downed = x;
      x_upped.at( i )++;
      x_downed.at( i )--;
      new_ret.push_back( x );
      new_ret.push_back( x_upped );
      new_ret.push_back( x_downed );
    }

    ret = new_ret;
  }

  return ret;
}

bool quantized_states_equal( std::vector<double> state1,
                             std::vector<double> state2 ) 
{
  const vector<quantized_t> state1_quantized = all_down( state1 );
  const vector<quantized_t> state2_quantized = all_down( state2 );

  const vector<vector<quantized_t>> fuzzy_states_1 { fuzz_state( state1_quantized ) };

  for ( unsigned int i = 0; i < fuzzy_states_1.size(); i++ ) {
    if ( fuzzy_states_1.at( i ) == state2_quantized ) return true;
  }
  return false;
}

/*
  Floyd's algorithm: run the slow network one event at a time,
  and the fast network two events at a time. When the states match,
  the network has a cycle.
 */
template < class SenderType1, class SenderType2 >
std::tuple< double, double, std::vector< std::pair< double, double > > > 
find_cycle_in_network( Network< SenderType1, SenderType2 > & network,
                       bool verbose = false ) {
  Network<SenderType1, SenderType2> network_slow( network );
  Network<SenderType1, SenderType2> network_fast( network );
  
  bool found_cycle = false;
  /* Phase 1: find a cycle */
  while ( network_fast.tickno() < END_TIME ) {
    network.run_until_event();

    network_fast.run_until_event();
    network_fast.run_until_event();

    auto network_state = network.get_state();
    auto fast_network_state = network_fast.get_state();

    if ( verbose ) {
      cout << setw(8) << network.tickno();
      for ( unsigned int i = 0; i < network_state.size() - 1; i++ ) {
        cout << " " <<  setw(10) << network_state.at( i );
      }
      cout << " " << setw(20) << network_state.at( network_state.size() - 1 ) << endl;
    }

    if ( quantized_states_equal(network.get_state(), network_fast.get_state()) ) {
      found_cycle = true;
      break;
    }
  } 

  if ( not found_cycle ) {
    throw Exception( "find_cycles", "No cycle found");
  }

  /* Phase 2: find the beginning of the cycle */
  while ( true ) {
    network_slow.run_until_event();
    network_fast.run_until_event();
    if ( quantized_states_equal(network_slow.get_state(), network_fast.get_state()) ) {
      break;
    }
  }

  double convergence_time = network_slow.tickno();

  auto current_state = network_slow.get_state();
  auto start_tp_del = network_slow.senders().throughputs_delays();
  double current_tick = network_slow.tickno();
  network_slow.run_until_event();
  while ( not quantized_states_equal( current_state, network_slow.get_state()) ) {
    network_slow.run_until_event();
  }
  auto end_tp_del = network_slow.senders().throughputs_delays();

  std::vector< std::pair< double, double > > deltas;
  for( size_t i = 0; i < start_tp_del.size(); i++ ) {
    deltas.emplace_back( end_tp_del.at( i ).first -
                         start_tp_del.at( i ).first,
                         end_tp_del.at( i ).second -
                         start_tp_del.at( i ).second );
  }

  double cycle_len = network_slow.tickno() - current_tick;

  return std::tuple<double, double, std::vector< std::pair< double, double > > > { convergence_time, cycle_len, deltas };
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

  //Network<Rat, Rat> network( Rat( whiskers ), prng, configuration );
  Network<Aimd, Aimd> network( Aimd(), prng, configuration );

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

  auto statistics = find_cycle_in_network( network, verbose );
  double convergence_time = std::get< 0 >( statistics );
  double cycle_len = std::get< 1 >( statistics );
  auto deltas = std::get< 2 >( statistics );

  for ( size_t i = 0; i < deltas.size(); i++ ) {
    auto packets_received = deltas.at( i ).first;
    auto total_delay = deltas.at( i ).second;
    auto norm_avg_delay = ( total_delay / packets_received ) / delay;
    auto norm_avg_throughput = ( packets_received / cycle_len ) / link_ppt;
    
    cout << sender_offset << " " << 
      convergence_time << " " << 
      cycle_len << " " << 
      norm_avg_delay << " " <<
      norm_avg_throughput << " " <<
      log2( norm_avg_throughput ) - log2( norm_avg_delay ) << " "  << endl;
  }

  return 0;
}
