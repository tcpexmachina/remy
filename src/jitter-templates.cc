#include "jitter.hh"

template <class NextHop>
void Jitter::tick( NextHop & next, const double & tickno )
{
  if( tickno <= _push_to_next_time ) {
    while( not _buffer.empty() ) {
      next.accept( _buffer.front(), tickno );
      _buffer.pop();
    }

    _push_to_next_time = _scheduler.sample() + tickno;
  }
}
