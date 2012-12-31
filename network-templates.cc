#include <utility>

#include "network.hh"

template <class NextHop>
void Network::tick( NextHop & next, const unsigned int tickno )
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
