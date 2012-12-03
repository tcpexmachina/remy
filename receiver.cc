#include <assert.h>

#include "receiver.hh"

Receiver::Receiver( const int num_senders )
  : _collector( num_senders )
{
}

void Receiver::accept( Packet && p, const int tickno ) noexcept
{
  assert( p.src < _collector.size() );

  p.tick_received = tickno;

  _collector[ p.src ].push_back( std::move( p ) );
}

std::vector< Packet > Receiver::collect( const unsigned int src )
{
  assert( src < _collector.size() );

  std::vector< Packet > ret( _collector[ src ] );
  _collector[ src ].clear();

  return ret;
}
