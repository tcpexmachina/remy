#include "simulationresults.pb.h"
#include <iostream>
#include <string>
#include <fstream>
using namespace std;

int main( int argc, char *argv[] )
{
  ifstream file;

  if ( argc != 2 ) {
    cerr << "This tool prints the contents of a simulations data file." << endl;
    cerr << "There must be exactly one argument, the name of the simulations data file." << endl;
    exit( 1 );
  }

  string filename( argv[1] );
  file.open( filename );
  if ( !file.is_open() ) {
    cerr << "Could not open file " << filename << endl;
    exit( 1 );
  }

  SimulationResultBuffers::SimulationsData simulations_data;
  if ( !simulations_data.ParseFromIstream( &file ) ) {
    cerr << "Could not parse file " << filename << endl;
    exit( 1 );
  }

  file.close();

  cout << simulations_data.DebugString() << endl;
  return 0;
}