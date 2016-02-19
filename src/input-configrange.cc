#include <iostream>
#include <cstdio>
#include <vector>
#include <string>
#include <limits>
#include <fstream>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include "dna.pb.h"
using namespace std;

// Parses input arguments, create range protobuf for specific parameter.
// If mandatory is false and no input arguments are found, skips and returns false.
// If the input arugments are invalid, or if mandatory is true and no input arguments
// are found, calls exit(1).
// If all input arguments were found, updates the range and returns true.
bool update_config_with_range(RemyBuffers::Range * range, int argc,
    char *argv[], string arg_name, bool mandatory) {

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

  if ( (min == -1 ) && (max == -1) && !mandatory ) {
    fprintf( stderr, "No arguments found for %s\n", arg_name.c_str() );
    return false;
  }

  if ( (min == -1) || (max == -1) ) {
    fprintf( stderr, "Please provide min, max and incr for %s with %s, %s, %s\n", arg_name.c_str(), min_string.c_str(), max_string.c_str(), incr_string.c_str());
    exit(1);
  }

  // check for validity
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

  range->set_low(min);
  range->set_high(max);
  range->set_incr(incr);

  return true;
}

int main(int argc, char *argv[]) {
  string input_filename, output_filename;
  bool infinite_buffers = false;
  ifstream input_file;

  for (int i = 1; i < argc; i++) {
    string arg( argv[ i ] );
    if ( arg.substr( 0, 3 ) == "if=") {
      input_filename = string( arg.substr( 3 ) );
    }
    if ( arg.substr( 0, 3 ) == "of=") {
      output_filename = string( arg.substr( 3 ) ) + ".cfg";
    }
    if ( arg == "inf_buffers") {
      infinite_buffers = true;
    } 
  }

  if (output_filename.empty()) {
    fprintf( stderr, "Provide of=file_name argument\n" );
    exit ( 1 );
  }

  RemyBuffers::ConfigRange input_config;
  bool mandatory = true;

  // If there's an input file, read it and use it
  if (!input_filename.empty()) {
    mandatory = false;
    input_file.open( input_filename );
    if ( !input_file.is_open() ) {
      fprintf(stderr, "Could not open file %s\n", input_filename.c_str() );
      exit( 1 );
    }
    if ( !input_config.ParseFromIstream( &input_file )) {
      fprintf(stderr, "Could not parse file %s\n", input_filename.c_str() );
      exit( 1 );
    }
    input_file.close();
  }

  update_config_with_range(input_config.mutable_link_packets_per_ms(), argc, argv, "link_ppt", mandatory);
  update_config_with_range(input_config.mutable_rtt(), argc, argv, "rtt", mandatory);
  update_config_with_range(input_config.mutable_num_senders(), argc, argv, "nsrc", mandatory);
  update_config_with_range(input_config.mutable_mean_on_duration(), argc, argv, "on", mandatory);
  update_config_with_range(input_config.mutable_mean_off_duration(), argc, argv, "off", mandatory);

  if ( !(infinite_buffers) ) {
    update_config_with_range(input_config.mutable_buffer_size(), argc, argv, "buf_size", mandatory);
  } else {
    RemyBuffers::Range buffer_size;
    buffer_size.set_low( numeric_limits<unsigned int>::max() );
    buffer_size.set_high( numeric_limits<unsigned int>::max() );
    buffer_size.set_incr( 0 );
    input_config.mutable_buffer_size()->CopyFrom(buffer_size);
  }
  // turn the range into a vector of input points
  RemyBuffers::ConfigVector input_networks;
  for ( double link = input_config.link_packets_per_ms().low(); link <= input_config.link_packets_per_ms().high(); link += input_config.link_packets_per_ms().incr() ) {
    for ( double del = input_config.rtt().low(); del <= input_config.rtt().high(); del += input_config.rtt().incr() ) {
      for ( double senders = input_config.num_senders().low(); senders <= input_config.num_senders().high(); senders += input_config.num_senders().incr() ) {
        for (double on = input_config.mean_on_duration().low(); on <= input_config.mean_on_duration().high(); on += input_config.mean_on_duration().incr() ) {
          for (double off = input_config.mean_off_duration().low(); off <= input_config.mean_off_duration().high(); off += input_config.mean_off_duration().incr() ) {
            for (double buffer = input_config.buffer_size().low(); buffer <= input_config.buffer_size().high(); buffer += input_config.buffer_size().incr() ) {
              RemyBuffers::NetConfig* net_config = input_networks.add_config();
              net_config->set_link_ppt(link);
              net_config->set_delay(del);
              net_config->set_num_senders(senders);
              net_config->set_mean_on_duration(on);
              net_config->set_mean_off_duration(off);
              net_config->set_buffer_size( buffer ); // buffer size is a multiple of the BDP*/
              if ( input_config.buffer_size().low() == input_config.buffer_size().high() ) { break; }
            }
            if ( input_config.mean_off_duration().low() == input_config.mean_off_duration().high() ) { break; }
          }
          if ( input_config.mean_on_duration().low() == input_config.mean_on_duration().high() ) { break; }
        }
        if ( input_config.num_senders().low() == input_config.num_senders().high() ) { break;}
      }
      if ( input_config.rtt().low() == input_config.rtt().high() ) { break; }
    }
    if ( input_config.link_packets_per_ms().low() == input_config.link_packets_per_ms().high() ) { break; }
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
    fprintf( stderr, "Could not serialize InputNetworks parameters.\n" );
    exit( 1 );
  }

  if ( close( fd ) < 0 ) {
    perror( "close" );
    exit( 1 );
  }
  printf( "Wrote config vector protobuf to \"%s\"\n", output_filename.c_str() );
  return 0;

}



