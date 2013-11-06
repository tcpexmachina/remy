#include <utility>

#include "link.hh"

Link::Link( const double s_rate )
  : _buffer(),
    _rate( s_rate ),
    _next_delivery_time( 0 )
{
}
