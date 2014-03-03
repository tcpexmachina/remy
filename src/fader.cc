#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#include <system_error>
#include <array>
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

void Fader::update( void )
{
  /* data ready to read? */
  while ( true ) {
    array< uint8_t, 4096 > read_buffer;
    ssize_t bytes_read = read( fd_, &read_buffer, read_buffer.size() );

    if ( bytes_read > 0 ) {
      buffer_.insert( buffer_.end(), read_buffer.begin(), read_buffer.begin() + bytes_read );
    } else if ( bytes_read == 0 ) {
      throw runtime_error( "EOF from fader" );
    } else if ( errno == EAGAIN or errno == EWOULDBLOCK ) {
      break;
    } else {
      throw system_error( errno, system_category() );
    }
  }

  auto new_physical_values_ = physical_values_;

  /* process what we have */
  while ( buffer_.size() >= 3 ) {
    /* check channel */
    const uint8_t channel = buffer_.front();
    buffer_.pop_front();
    if ( channel != 176 ) {
      throw runtime_error( "unknown MIDI channel" );
    }

    /* get control */
    const uint8_t control = buffer_.front();
    buffer_.pop_front();

    /* get value */
    const uint8_t value = buffer_.front();
    buffer_.pop_front();

    if ( control >= physical_values_.size() ) {
      throw runtime_error( "unexpected MIDI control number" );
    }

    if ( value >= 128 ) {
      throw runtime_error( "unexpected MIDI control value" );
    }

    new_physical_values_.at( control ) = value;
  }

  /* process the changes */
  for ( unsigned int i = 0; i < physical_values_.size(); i++ ) {
    
  }

  physical_values_ = new_physical_values_;
}
