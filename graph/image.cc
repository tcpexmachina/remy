#include <cstring>
#include <stdexcept>

#include "image.hh"

using namespace std;

Image::Image( const unsigned int width,
	      const unsigned int height,
	      const unsigned int stride_pixels )
  : width_( width ),
    height_( height ),
    stride_pixels_( stride_pixels ),
    pixels_()
{
  if ( stride_pixels_ < width ) {
    throw runtime_error( "invalid stride in Image constructor" );
  }
  pixels_.reserve( stride_pixels * height );
  for ( unsigned int y = 0; y < height; y++ ) {
    for ( unsigned int x = 0; x < stride_pixels; x++ ) {
      pixels_.emplace_back();
    }
  }
}

void Image::clear( void )
{
  memset( raw_pixels(), 255, pixels_.size() * sizeof( Pixel ) );
}
