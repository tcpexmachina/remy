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

  double link_rate_ = 3.16;
  double time_increment_ = 0.01;
  double horizontal_size_ = 10;
  double buffer_size_ = 5000;
  bool autoscale_ = false;

  void compute_internal_state( void );
  void rationalize( decltype(physical_values_) & output ) const;
  void write( const decltype(physical_values_) & output );

public:
  Fader( const std::string & filename );

  template <class NetworkType>
  void update( NetworkType & network );

  double link_rate( void ) const { return link_rate_; }
  double time_increment( void ) const { return time_increment_; }
  double horizontal_size( void ) const { return horizontal_size_; }
  unsigned int buffer_size( void ) const { return static_cast<unsigned int>( buffer_size_ ); }
  bool autoscale( void ) const { return autoscale_; }
};

#endif /* FADER_HH */
