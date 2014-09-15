#ifndef FADER_HH
#define FADER_HH

#include <string>
#include <deque>
#include <array>
#include <atomic>

class GTKFader
{
  std::atomic<double> link_rate_ { 1.01 };
  double time_increment_ = 0.0107893;
  double horizontal_size_ = 10.4385;
  double buffer_size_ = 4820;
  bool autoscale_ = false;
  bool autoscale_all_ = false;

public:
  GTKFader();

  template <class NetworkType>
  void update( NetworkType & network );

  double link_rate( void ) const { return link_rate_; }
  double time_increment( void ) const { return time_increment_; }
  double horizontal_size( void ) const { return horizontal_size_; }
  unsigned int buffer_size( void ) const { return static_cast<unsigned int>( buffer_size_ ); }
  bool autoscale( void ) const { return autoscale_; }
  bool autoscale_all( void ) const { return autoscale_all_; }
};

#endif /* FADER_HH */
