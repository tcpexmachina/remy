#include <assert.h>

#include "receiver.hh"

Receiver::Receiver( const int num_senders )
  : _collector( num_senders )
{
}

void Receiver::accept( const Packet & p, const int tickno )
{
  assert( p.src < _collector.size() );

  Packet time_marked( p );
  time_marked.tick_received = tickno;

  _collector[ p.src ].push_back( time_marked );
}

std::vector< Packet > Receiver::collect( const unsigned int src )
{
  assert( src < _collector.size() );

  std::vector< Packet > ret( _collector[ src ] );
  _collector[ src ].clear();

  return ret;
}
