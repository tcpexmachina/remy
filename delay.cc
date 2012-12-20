#include "delay.hh"

using namespace std;

template <class NextHop>
Delay<NextHop>::Delay( const unsigned int s_delay )
  : _queue(),
    _delay( s_delay )
{
}

template <class NextHop>
void Delay<NextHop>::accept( Packet && p, const unsigned int tickno ) noexcept
{
  _queue.emplace( tickno + _delay, move( p ) );
}

template <class NextHop>
void Delay<NextHop>::tick( NextHop & next, const unsigned int tickno )
{
  while ( (!_queue.empty()) && (get< 0 >( _queue.front() ) <= tickno) ) {
    next.accept( move( get< 1 >( _queue.front() ) ), tickno );
    _queue.pop();
  }
}
