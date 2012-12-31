#include <utility>

#include "network.hh"

Network::Network( const double s_rate, PRNG & s_prng )
  : _buffer(),
    _egress_process( s_rate, s_prng )
{
}

void Network::accept( Packet && p ) noexcept
{
  _buffer.push( std::move( p ) );
}
