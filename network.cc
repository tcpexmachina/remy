#include <assert.h>
#include <stdlib.h>
#include <utility>

#include "network.hh"

Network::Network( const double s_rate )
  : _buffer(),
    _egress_process( s_rate ),
    _used_pdos( 0 ),
    _underflows( 0 )
{
}

void Network::accept( Packet && p ) noexcept
{
  _buffer.push( std::move( p ) );
}

void Network::tick( Receiver & rec, const unsigned int tickno )
{
  const int num = _egress_process.sample();

  for ( int i = 0; i < num; i++ ) {
    if ( _buffer.empty() ) {
      _underflows += num - i;
      break;
    }

    rec.accept( std::move( _buffer.front() ), tickno );
    _buffer.pop();
    _used_pdos++;
  }
}
