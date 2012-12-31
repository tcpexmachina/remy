#include <utility>

#include "link.hh"

Link::Link( const double s_rate, PRNG & s_prng )
  : _buffer(),
    _egress_process( s_rate, s_prng )
{
}

void Link::accept( Packet && p ) noexcept
{
  _buffer.push( std::move( p ) );
}
