#include "dna.pb.h"
#include <iostream>
#include <string>
#include <fstream>
using namespace std;

int main( int argc, char *argv[] )
{
  ifstream file;

  if ( argc != 2 ) {
    cerr << "This tool prints the contents of an input configuration file." << endl;
    cerr << "There must be exactly one argument, the name of the config file." << endl;
    exit( 1 );
  }

  string filename( argv[1] );
  file.open( filename );
  if ( !file.is_open() ) {
    cerr << "Could not open file " << filename << endl;
    exit( 1 );
  }

  RemyBuffers::ConfigRange configrange;
  if ( !configrange.ParseFromIstream( &file ) ) {
    cerr << "Could not parse file " << filename << endl;
    exit( 1 );
  }

  file.close();

  cout << configrange.DebugString() << endl;
  return 0;
}