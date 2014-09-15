#ifndef FADER_HH
#define FADER_HH

#include <string>
#include <deque>
#include <array>
#include <atomic>

class GTKFader
{
  std::atomic<double> link_rate_ { 1.01 };
  std::atomic<double> time_increment_ { 1.0 / 60.0 };
  std::atomic<double> horizontal_size_ { 10 };
  std::atomic<double> buffer_size_ { 4820 };
  std::atomic<bool> autoscale_ { false };
  std::atomic<bool> autoscale_all_ { false };
  std::atomic<bool> quit_ { false };

public:
  GTKFader();

  template <class NetworkType>
  void update( NetworkType & network );

  double link_rate( void ) const { return link_rate_; }
  double time_increment( void ) const { return time_increment_; }
  double horizontal_size( void ) const { return horizontal_size_; }
  double buffer_size( void ) const { return buffer_size_; }
  bool autoscale( void ) { bool ret = autoscale_; autoscale_ = false; return ret; }
  bool autoscale_all( void ) { bool ret = autoscale_all_; autoscale_all_ = false; return ret; }
  bool quit( void ) const { return quit_; }
};

#endif /* FADER_HH */
