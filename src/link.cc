#include <utility>

#include "link.hh"

Link::Link( const double s_rate )
  : _buffer(),
    _rate( s_rate ),
    _next_delivery_time( 0 )
{
}

double Link::next_event_time( const double & tickno ) const
{
  assert( _next_delivery_time >= tickno );

  if ( _buffer.empty() ) {
    return std::numeric_limits<double>::max();
  }

  return _next_delivery_time;
}
