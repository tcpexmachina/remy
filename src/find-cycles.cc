#include <cstdio>
#include <vector>
#include <string>
#include <google/sparse_hash_set>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <boost/functional/hash.hpp>
#include <unordered_map>
#include <unordered_set>
#include <iomanip>
#include <limits>

#include "sendergangofgangs.hh"
#include "simple-templates.cc"
#include "rat.hh"
#include "network.hh"
#include "network.cc"
#include "whiskertree.hh"

using namespace std;

const double quantizer = 100000000;

typedef int64_t quantized_t;

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
void find_cycle_in_network( Network< SenderType1, SenderType2 > & network ) {
  Network<SenderType1, SenderType2> network_fast( network );
  
  while ( true ) {
    network.run_until_sender_event();

    network_fast.run_until_sender_event();
    network_fast.run_until_sender_event();
    
    if ( quantized_states_equal(network.get_state(), network_fast.get_state()) ) {
        break;
      }
  } 

  auto current_state = network.get_state();
  auto start_tp_del = network.senders().throughputs_delays();
  double current_tick = network.tickno();
  network.run_until_sender_event();
  while ( not quantized_states_equal( current_state, network.get_state()) ) {
    network.run_until_sender_event();
  }
  auto end_tp_del = network.senders().throughputs_delays();
  double tp_change = end_tp_del.at( 0 ).first - start_tp_del.at( 0 ).first;
  double del_change = end_tp_del.at( 0 ).second - start_tp_del.at( 0 ).second;
  cout << tp_change << " " << del_change / tp_change << endl;
  printf( "cycle len %f\n", network.tickno() - current_tick );
}

int main( int argc, char *argv[] )
{
  WhiskerTree whiskers;
  unsigned int num_senders = 1;
  double link_ppt = 1.0;
  double delay = 50.0;
  double mean_on_duration = 10000000.0;
  double mean_off_duration = 0.0;
  double imputed_delay __attribute((unused)) = 1.0;
  double rewma __attribute((unused)) = 1.0;
  unsigned int initial_buffer = 0;

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

    } else if ( arg.substr( 0, 5 ) == "nsrc=" ) {
      num_senders = atoi( arg.substr( 5 ).c_str() );
    } else if ( arg.substr( 0, 5 ) == "link=" ) {
      link_ppt = atof( arg.substr( 5 ).c_str() );
    } else if ( arg.substr( 0, 4 ) == "rtt=" ) {
      delay = atof( arg.substr( 4 ).c_str() );
    } else if ( arg.substr( 0, 4 ) == "del=" ) {
      imputed_delay = atof( arg.substr( 4 ).c_str() );
    } else if ( arg.substr( 0, 6 ) == "rewma=" ) {
      rewma = atof( arg.substr( 6 ).c_str() );
    } else if ( arg.substr( 0, 5 ) == "buff=" ) {
      initial_buffer = atoi( arg.substr( 5 ).c_str() );
    }
  }

  PRNG prng( 50 );
  NetConfig configuration = NetConfig().set_link_ppt( link_ppt ).set_delay( delay ).set_num_senders( num_senders ).set_on_duration( mean_on_duration ).set_off_duration( mean_off_duration ).set_start_buffer( initial_buffer ); /* always on */
  Network<Rat, Rat> network( Rat( whiskers ), prng, configuration );
  network.mutable_senders().mutable_gang1().mutable_sender( 0 ).mutable_sender().set_initial_state( std::vector< double > { imputed_delay, rewma } );

  find_cycle_in_network( network );

  return 0;
}
