#include "dna.pb.h"
#include "configvector.hh"
#include <iostream>
#include <string>
#include <fstream>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
using namespace std;

int main( int argc, char *argv[] )
{
  
  if ( argc != 2 ) {
    cerr << "This tool prints the contents of an input configuration file." << endl;
    cerr << "There must be exactly one argument, the name of the config file." << endl;
    exit( 1 );
  }

  string config_filename( argv[1] );

  RemyBuffers::ConfigVector config_vector;
  int cfd = open( config_filename.c_str(), O_RDONLY );
  if ( cfd < 0 ) {
    perror( "Error with opening this config file.");
    exit( 1 );
   }
   if ( !config_vector.ParseFromFileDescriptor( cfd ) ) {
     fprintf( stderr, "Could not parse input config from file %s. \n", config_filename.c_str() );
     exit ( 1 );
   }
   if ( close( cfd ) < 0 ) {
     perror( "close" );
     exit( 1 );
   }

  // iterate through each config, print it
  ConfigVector input_vector;
  input_vector.ReadVector(config_vector);
  for (auto &net_config: input_vector.configs) {
    printf( "===\nconfig: %s\n", net_config.str().c_str() );
  }
  return 0;
}
