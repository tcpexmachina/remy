#include <cmath>

#include "jitter.hh"

template <class NextHop>
void Jitter::tick( NextHop & next, const double & tickno )
{
  if( tickno >= _push_to_next_time ) {
    if( not _buffer.empty() ) {
      next.accept( _buffer.front(), tickno );
      _buffer.pop();
      if( (rand() % 5) == 1 ) {
        _push_to_next_time = tickno + 1.00001;
      } else {
        _push_to_next_time = tickno + 0.00001;
      }
    }
  }
}
