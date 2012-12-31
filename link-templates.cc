#include <utility>

#include "link.hh"

template <class NextHop>
void Link::tick( NextHop & next, const unsigned int tickno )
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
