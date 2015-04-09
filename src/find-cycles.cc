#include <cstdio>
#include <vector>
#include <string>
#include <google/sparse_hash_map>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <boost/functional/hash.hpp>
#include <unordered_map>
#include <iomanip>

#include "sendergangofgangs.hh"
#include "simple-templates.cc"
#include "rat.hh"
#include "aimd.hh"
#include "aimd-templates.cc"
#include "network.hh"
#include "network.cc"
#include "whiskertree.hh"

using namespace std;

const double quantizer = 10000000;

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
    if ( i == 1 or i == 3 or i == 7 ) {
      continue;
    }

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

int main( int argc, char *argv[] )
{
  WhiskerTree whiskers;
  unsigned int num_senders = 1;
  double link_ppt = 1.0;
  double delay = 50.0;
  double mean_on_duration = 10000000.0;
  double mean_off_duration = 0.0;
  double imputed_delay __attribute((unused)) = 1.0;
  double rewma = 1.0;
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

  unordered_map< vector<quantized_t>, double, boost::hash<vector<quantized_t>> > state_set;
  
  PRNG prng( 50 );
  NetConfig configuration = NetConfig().set_link_ppt( link_ppt ).set_delay( delay ).set_num_senders( num_senders ).set_on_duration( mean_on_duration ).set_off_duration( mean_off_duration ).set_start_buffer( initial_buffer ); /* always on */
  //Network<Rat, Rat> network( Rat( whiskers ), prng, configuration );
  Network<Aimd, Aimd> network( Aimd(), prng, configuration );
  //network.mutable_senders().mutable_gang1().mutable_sender( 0 ).mutable_sender().set_initial_state( std::vector< double > { imputed_delay, rewma } );

  double time = 0.0;
  const double end_time = 100000000.0;
  vector<quantized_t> last_state;
  while ( time < end_time ) { 
    network.run_until_event();
    time = network.tickno();
    const vector<double> network_state_exact { network.get_state() };
    const vector<quantized_t> network_state = all_down( network_state_exact );

    if( network_state == last_state ) {
      /* Don't match if we haven't exited the state */
      continue;
    }

    cout << setw(8) << time;
    for ( unsigned int i = 0; i < network_state.size() - 1; i++ ) {
      cout << " " <<  setw(10) << network_state.at( i ) / quantizer;
    }
    cout << " " << setw(20) << network_state.at( network_state.size() - 1 ) / quantizer << " ";

    auto match = state_set.find( network_state );
    if ( match != state_set.end() ) {
      cout << initial_buffer << " " << rewma << " "  << 
        match->second << " " << time - match->second;
      //break;
    }

    cout << endl;

    const vector<vector<quantized_t>> fuzzy_states { fuzz_state( network_state ) };
    for ( const auto & x : fuzzy_states ) {
      state_set[ x ] = time;
    }

    last_state = network_state;
  }

  cout << endl;

  return 0;
}
