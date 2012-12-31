#include <utility>

#include "network.hh"

Network::Network( const double s_rate )
  : _buffer(),
    _egress_process( s_rate )
{
}

void Network::accept( Packet && p ) noexcept
{
  _buffer.push( std::move( p ) );
}
