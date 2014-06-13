#ifndef UTIL_HH
#define UTIL_HH

#include <unistd.h>
#include <fcntl.h>
#include <string>

template <class T>
void dump_to_file( const T & protobuf, const std::string file_name )
{
  assert( file_name != "" );
  char of[ 128 ];
  snprintf( of, 128, "%s", file_name.c_str() );
  fprintf( stderr, "Writing to \"%s\"... ", of );
  int fd = open( of, O_WRONLY | O_TRUNC | O_CREAT, S_IRUSR | S_IWUSR );
  if ( fd < 0 ) {
    perror( "open" );
    exit( 1 );
  }
  if ( not protobuf.SerializeToFileDescriptor( fd ) ) {
    fprintf( stderr, "Could not serialize protobuf.\n" );
    exit( 1 );
  }

  if ( close( fd ) < 0 ) {
    perror( "close" );
    exit( 1 );
  }
}

#endif // UTIL_HH
