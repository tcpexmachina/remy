#include <cstdio>
#include <unistd.h>
#include <fcntl.h>

#include "ratbreeder.hh"

using namespace std;

int main(int, const char* argv[]) {
  const Whisker paced( 25600, 100, stod( argv[ 1 ] ) , MemoryRange( Memory(), MAX_MEMORY() ) );
  const WhiskerTree whiskers( paced, false );
  if ( !whiskers.DNA().SerializeToFileDescriptor( STDOUT_FILENO ) ) {
    fprintf( stderr, "Could not serialize whiskers.\n" );
    exit( 1 );
  }
}
