#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#include <system_error>
#include <iostream>

#include "fader.hh"

using namespace std;

Fader::Fader( const string & filename )
  : fd_( open( filename.c_str(), O_RDWR | O_NONBLOCK ) ),
    buffer_(),
    physical_values_( {{}} )
{
  if ( fd_ < 0 ) {
    throw system_error( errno, system_category() );
  }

  cerr << "opened " << filename << endl;
}
