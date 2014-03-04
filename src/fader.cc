#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#include <system_error>
#include <cmath>

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

  initialize();

  auto output = physical_values_;

  for ( auto & x : physical_values_ ) {
    x = 255;
  }

  write( output );
}

void Fader::write( const decltype(physical_values_) & output )
{
  for ( unsigned int i = 0; i < physical_values_.size(); i++ ) {
    if ( output.at( i ) != physical_values_.at( i ) ) {
      array< uint8_t, 3 > write_buffer = { 176, uint8_t( i ), output.at( i ) };

      ssize_t bytes_written = ::write( fd_, &write_buffer, write_buffer.size() );      
      if ( bytes_written != 3 ) {
	throw runtime_error( "could not write to MIDI device" );
      }
    }
  }

  physical_values_ = output;
}

void Fader::compute_internal_state( void )
{
  link_rate_ = 0.316227766016838 * pow( 100.0, physical_values_.at( 81 ) / 127.0 );
  time_increment_ = (pow( 1.05, physical_values_.at( 88 ) ) - 1) / 100;
  horizontal_size_ = pow( 1.07, physical_values_.at( 87 ) / 1.5 );
  autoscale_ = physical_values_.at( 89 );
  buffer_size_ = (pow( 1.03, physical_values_.at( 83 ) ) - 1) * 500;
}

void Fader::rationalize( decltype(physical_values_) & output ) const
{
  for ( uint8_t i = 0; i < 128; i++ ) {
    if ( link_rate_ <= 0.316227766016838 * pow( 100.0, i / 127.0 ) ) {
      output.at( 81 ) = i;
      break;
    }
  }

  for ( uint8_t i = 0; i < 128; i++ ) {
    if ( time_increment_ <= (pow( 1.05, i ) - 1) / 100 ) {
      output.at( 88 ) = i;
      break;
    }
  }

  for ( uint8_t i = 0; i < 128; i++ ) {
    if ( horizontal_size_ <= pow( 1.07, i / 1.5 ) ) {
      output.at( 87 ) = i;
      break;
    }
  }

  for ( uint8_t i = 0; i < 128; i++ ) {
    if ( buffer_size_ <= (pow( 1.03, i ) - 1) * 500 ) {
      output.at( 83 ) = i;
      break;
    }
  }

  for ( uint8_t i = 0; i < 128; i++ ) {
    output.at( 89 ) = autoscale_ ? 127 : 0;
  }
}

void Fader::initialize( void )
{
  link_rate_ = 3.16;
  time_increment_ = 0.01;
  horizontal_size_ = 10;
  buffer_size_ = link_rate_ * 150 * 10;
  autoscale_ = false;

  rationalize( physical_values_ );
  compute_internal_state();
}
