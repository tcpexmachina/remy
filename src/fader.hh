#ifndef FADER_HH
#define FADER_HH

#include <string>
#include <deque>
#include <array>

class Fader
{
  int fd_;
  std::deque< uint8_t > buffer_;

  std::array< uint8_t, 94 > physical_values_;

  double time_increment_ = 0.01;
  double horizontal_size_ = 10;

public:
  Fader( const std::string & filename );

  template <class NetworkType>
  void update( NetworkType & network );

  double time_increment( void ) const { return time_increment_; }
  double horizontal_size( void ) const { return horizontal_size_; }
};

#endif /* FADER_HH */
