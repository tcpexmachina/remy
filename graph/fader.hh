#ifndef FADER_HH
#define FADER_HH

#include <memory>
#include <atomic>
#include <mutex>

class GTKFader
{
  std::atomic<double> link_rate_ { 1 };
  std::atomic<double> rtt_ { 150 };
  std::atomic<double> time_increment_ { 0.65 / 60.0 }; /* a little slower than real time by default */
  std::atomic<double> horizontal_size_ { 10 };
  std::atomic<double> buffer_size_ { 1000 };
  std::atomic<bool> autoscale_ { false };
  std::atomic<bool> autoscale_all_ { false };
  std::atomic<bool> quit_ { false };

  std::unique_ptr<std::atomic<bool>[]> remy_;
  std::unique_ptr<std::atomic<bool>[]> aimd_;

  std::mutex mutex_ {};

public:
  GTKFader( const unsigned int & num_senders );

  template <class NetworkType>
  void update( NetworkType & network );

  double link_rate( void ) const { return link_rate_; }
  double rtt( void ) const { return rtt_; }
  double time_increment( void ) const { return time_increment_; }
  double horizontal_size( void ) const { return horizontal_size_; }
  double buffer_size( void ) const { return buffer_size_; }
  bool autoscale( void ) { bool ret = autoscale_; autoscale_ = false; return ret; }
  bool autoscale_all( void ) { bool ret = autoscale_all_; autoscale_all_ = false; return ret; }
  bool quit( void ) const { return quit_; }
};

#endif /* FADER_HH */
