#include <utility>

#include "network.hh"

template <class NextHop>
Network<NextHop>::Network( const double s_rate )
  : _buffer(),
    _egress_process( s_rate )
{
}

template <class NextHop>
void Network<NextHop>::accept( Packet && p ) noexcept
{
  _buffer.push( std::move( p ) );
}

template <class NextHop>
void Network<NextHop>::tick( NextHop & next, const unsigned int tickno )
{
  const int num = _egress_process.sample();

  for ( int i = 0; i < num; i++ ) {
    if ( _buffer.empty() ) {
      break;
    }

    next.accept( std::move( _buffer.front() ), tickno );
    _buffer.pop();
  }
}
