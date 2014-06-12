#include <vector>
#include <string>
#include <iterator>
#include <regex>
#include <fcntl.h>
#include <boost/algorithm/string.hpp>

#include "dna.pb.h"
#include "network.hh"

using namespace std;

/* Split string on tok, just an excuse to use yet another C++11 feature */
vector<string> split_string( const string & str, const string & tok )
{
  vector<string> ret;
  boost::split( ret, str, boost::is_any_of( tok ) );
  return ret;
}

template <class Type> 
vector<Type> parse_comma_separators( const string & str ) {}

template <>
vector<double> parse_comma_separators( const string & str )
{
  vector<double> values;
  auto split_strs = split_string( str, "," );
  for ( auto &x: split_strs ) {
    values.push_back( stod( x ) );
  }
  return values;
}

template <>
vector<int> parse_comma_separators( const string & str )
{
  vector<int> values;
  auto split_strs = split_string( str, "," );
  for ( auto &x: split_strs ) {
    values.push_back( stoi( x ) );
  }
  return values;
}

int main( int argc, char *argv[] )
{
  vector<int> num_senders {2};
  vector<double> link_ppt {1.0};
  vector<double> delays {100.0};
  vector<double> mean_on_duration {5000.0};
  vector<double> mean_off_duration {5000.0};
  string scenario_file;

  for ( int i = 1; i < argc; i++ ) {
    string arg( argv[ i ] );
    if ( arg.substr( 0, 5 ) == "nsrc=" ) {
      num_senders = parse_comma_separators<int>( arg.substr( 5 ).c_str() );
    } else if ( arg.substr( 0, 5 ) == "link=" ) {
      link_ppt = parse_comma_separators<double>( arg.substr( 5 ).c_str() );
    } else if ( arg.substr( 0, 4 ) == "rtt=" ) {
      delays = parse_comma_separators<double>( arg.substr( 4 ).c_str() );
    } else if ( arg.substr( 0, 3 ) == "on=" ) {
      mean_on_duration = parse_comma_separators<double>( arg.substr( 3 ).c_str() );
    } else if ( arg.substr( 0, 4 ) == "off=" ) {
      mean_off_duration = parse_comma_separators<double>( arg.substr( 4 ).c_str() );
    } else if ( arg.substr( 0, 14 ) == "scenario_file=" ) {
      scenario_file = arg.substr( 14 );
    }
  }

  std::vector<NetConfig> net_configs;
  for ( auto &senders: num_senders )
    for ( auto &link: link_ppt )
      for ( auto &delay: delays )
        for ( auto &on: mean_on_duration )
          for (auto &off: mean_off_duration )
            net_configs.push_back( NetConfig().set_num_senders( senders )
                                              .set_link_ppt( link )
                                              .set_delay( delay )
                                              .set_on_duration( on )
                                              .set_off_duration( off ) );

  RemyBuffers::Scenarios scenarios;
  for ( auto &config: net_configs )
    *( scenarios.add_configs() ) = config.DNA();

  cout << scenarios.DebugString();

  assert( scenario_file != "" );
  char of[ 128 ];
  snprintf( of, 128, "%s", scenario_file.c_str() );
  fprintf( stderr, "Writing to \"%s\"...\n", of );
  int fd = open( of, O_WRONLY | O_TRUNC | O_CREAT, S_IRUSR | S_IWUSR );
  if ( fd < 0 ) {
    perror( "open" );
    exit( 1 );
  }

  if ( not scenarios.SerializeToFileDescriptor( fd ) ) {
    fprintf( stderr, "Could not serialize scenario.\n" );
    exit( 1 );
  }

  if ( close( fd ) < 0 ) {
    perror( "close" );
    exit( 1 );
  }

  return 0;
}
