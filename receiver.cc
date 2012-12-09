#include <assert.h>

#include "receiver.hh"

Receiver::Receiver( const int num_senders )
  : _collector( num_senders )
{
}

void Receiver::accept( Packet && p, const unsigned int tickno ) noexcept
{
  assert( p.src < _collector.size() );

  p.tick_received = tickno;

  _collector[ p.src ].push_back( std::move( p ) );
}

std::vector< Packet > Receiver::collect( const unsigned int src )
{
  assert( src < _collector.size() );

  auto ret( std::move( _collector[ src ] ) );
  assert( _collector[ src ].empty() );

  return ret;
}
