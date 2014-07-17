#ifndef JITTER_HH
#define JITTER_HH

#include <queue>

#include "packet.hh"
#include "delay.hh"
#include "exponential.hh"

class Jitter
{
private:
  std::queue< Packet > _buffer;

  Exponential _scheduler;
  double _push_to_next_time;

public:
  Jitter( const double mean_jitter, PRNG & prng )
    : _buffer(),
      _scheduler( 1.0/mean_jitter, prng ),
      _push_to_next_time( _scheduler.sample() )
  {
  }

  void accept( const Packet & p, const double & tickno __attribute((unused)) ) noexcept {
    _buffer.push( p );
  }

  template <class NextHop>
  void tick( NextHop & next, const double & tickno );

  double next_event_time( const double & tickno __attribute((unused)) ) const { return _push_to_next_time; }
};

#endif
