#include <assert.h>

#include "receiver.hh"

Receiver::Receiver()
  : _collector()
{
}

void Receiver::accept( Packet && p, const unsigned int tickno ) noexcept
{
  autosize( p.src );

  p.tick_received = tickno;

  _collector[ p.src ].push_back( std::move( p ) );
}

std::vector< Packet > Receiver::collect( const unsigned int src )
{
  auto ret( std::move( _collector[ src ] ) );
  assert( _collector[ src ].empty() );

  return ret;
}

void Receiver::autosize( const unsigned int index )
{
  if ( index >= _collector.size() ) {
    _collector.resize( index + 1 );
  }
}
