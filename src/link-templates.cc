#include <utility>

#include "link.hh"

template <class NextHop>
void Link::tick( NextHop & next, const double & tickno )
{
  assert( _next_delivery_time >= tickno );

  while ( _next_delivery_time <= tickno ) {
    if ( !_buffer.empty() ) {
      next.accept( std::move( _buffer.front() ), tickno );
      _buffer.pop();
    }
    _next_delivery_time += 1.0 / _rate;
  }
}
