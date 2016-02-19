#include <iostream>
#include <cstdio>
#include <vector>
#include <string>
#include <limits>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include "dna.pb.h"
using namespace std;

// Parses input arguments, create range protobuf for specific parameter
RemyBuffers::Range set_range_protobuf(int argc, char *argv[], string arg_name) {
  string min_string = "min_" + arg_name + "=";
  string max_string = "max_" + arg_name + "=";
  string incr_string = arg_name + "_incr=";
  int min_size = min_string.size();
  int max_size = max_string.size();
  int incr_size = incr_string.size();
  double min = -1;
  double max = -1;
  double incr = 0;
  for (int i=1; i < argc; i++) {
    string arg( argv[ i ] );
    if ( arg.substr( 0, min_size ) == min_string ) {
     min = atof( arg.substr( min_size ).c_str() );
     fprintf( stderr, "Setting min of %s to %f\n", arg_name.c_str(), min );
    } else if ( arg.substr( 0, max_size ) == max_string ) {
     max = atof( arg.substr( max_size ).c_str() );
     fprintf( stderr, "Setting max of %s to %f\n", arg_name.c_str(), max );
    } else if ( arg.substr( 0, incr_size ) == incr_string ) {
     incr = atof ( arg.substr( incr_size ).c_str() );
     fprintf( stderr, "Setting incr of %s to %f\n", arg_name.c_str(), incr );
    }
  }

  if ( (min == -1) || (max == -1) ) {
    fprintf( stderr, "Please provide min, max and incr for %s with %s, %s, %s\n", arg_name.c_str(), min_string.c_str(), max_string.c_str(), incr_string.c_str());
    exit(1);
  }

  if ( min < max ) {
    if ( incr == 0 ) {
      fprintf( stderr, "Please provide non-zero incr for %s\n", arg_name.c_str());
      exit(1);
    } else if (  incr > max - min  ) {
      fprintf( stderr, "Please provide valid incr for %s\n.", arg_name.c_str());
      exit(1);
    }
  } else if ( min > max ){
    fprintf( stderr, "Please provide valid min and max for %s\n.", arg_name.c_str());
    exit(1);
  }
    
  RemyBuffers::Range range;
  range.set_low(min);
  range.set_high(max);
  range.set_incr(incr);
  return range;
}

int main(int argc, char *argv[]) {
  string output_filename;

  for (int i = 1; i < argc; i++) {
    string arg( argv[ i ] );
    if ( arg.substr( 0, 3 ) == "of=") {
      output_filename = string( arg.substr( 3 ) ) + ".cfg";
    }
  }


  if (output_filename.empty()) {
    fprintf( stderr, "Provide of=file_name argument\n" );
    exit ( 1 );
  }

  // parse all other config range parameters
  RemyBuffers::Range link_ppt = set_range_protobuf(argc, argv, "link_ppt");
  RemyBuffers::Range rtt = set_range_protobuf(argc, argv, "rtt");
  RemyBuffers::Range num_senders = set_range_protobuf(argc, argv, "nsrc");
  RemyBuffers::Range mean_on_duration = set_range_protobuf(argc, argv, "on");
  RemyBuffers::Range mean_off_duration = set_range_protobuf(argc, argv, "off");
  RemyBuffers::Range bdp_mult = set_range_protobuf(argc, argv, "bdp_mult");
  // add these ranges to configrange protobuf

  // create a net config vector
  RemyBuffers::ConfigVector input_networks;
  for ( double link = link_ppt.low(); link <= link_ppt.high(); link += link_ppt.incr() ) {
    for ( double del = rtt.low(); del <= rtt.high(); del += rtt.incr() ) {
      for ( double senders = num_senders.low(); senders <= num_senders.high(); senders += num_senders.incr() ) {
        for (double on = mean_on_duration.low(); on <= mean_on_duration.high(); on += mean_on_duration.incr() ) {
          for (double off = mean_off_duration.low(); off <= mean_off_duration.high(); off += mean_off_duration.incr() ) {
            for (double bdp = bdp_mult.low(); bdp <= bdp_mult.high(); bdp += bdp_mult.incr() ) {
              RemyBuffers::NetConfig* net_config = input_networks.add_config();
              net_config->set_link_ppt(link);
              net_config->set_delay(del);
              net_config->set_num_senders(senders);
              net_config->set_mean_on_duration(on);
              net_config->set_mean_off_duration(off);
              net_config->set_buffer_size( link * del * bdp ); // buffer size is a multiple of the BDP*/
              if ( bdp_mult.low() == bdp_mult.high() ) { break; }
            }
            if ( mean_off_duration.low() == mean_off_duration.high() ) { break; }
          }
          if ( mean_on_duration.low() == mean_on_duration.high() ) { break; }
        }
        if ( num_senders.low() == num_senders.high() ) { break;}
      }
      if ( rtt.low() == rtt.high() ) { break; }
    }
    if ( link_ppt.low() == link_ppt.high() ) { break; }
  }

  
   

  // write to file
  char of[ 128 ];
  snprintf( of, 128, "%s", output_filename.c_str());
  int fd = open( of, O_WRONLY | O_TRUNC | O_CREAT, S_IRUSR | S_IWUSR );
  if ( fd < 0 ) {
    perror( "open" );
    exit( 1 );
  }
  

  if ( not input_networks.SerializeToFileDescriptor( fd ) ) {
    fprintf( stderr, "Could not serialize InputConfig parameters.\n" );
    exit( 1 );
  }

  if ( close( fd ) < 0 ) {
    perror( "close" );
    exit( 1 );
  }
  printf( "Wrote config range protobuf to \"%s\"\n", output_filename.c_str() );
  return 0;

}



