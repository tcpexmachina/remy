#ifndef IMAGE_HH
#define IMAGE_HH

#include <vector>
#include <cstdint>

typedef uint32_t Pixel;

class Image
{
  unsigned int width_, height_, stride_pixels_;

  std::vector<Pixel> pixels_;

public:
  Image( const unsigned int width,
	 const unsigned int height,
	 const unsigned int stride_pixels );

  std::pair<unsigned int, unsigned int> size( void ) const { return std::make_pair( width_, height_ ); }
  const std::vector<Pixel> & pixels( void ) const { return pixels_; }
  unsigned char * raw_pixels( void ) { return reinterpret_cast<unsigned char *>( &pixels_.front() ); }

  unsigned int stride_pixels( void ) const { return stride_pixels_; }
  unsigned int stride_bytes( void ) const { return stride_pixels_ * sizeof( Pixel ); }
  void clear( void );
};

#endif /* IMAGE_HH */
