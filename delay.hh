#ifndef DELAY_HH
#define DELAY_HH

#include <queue>
#include <tuple>

#include "packet.hh"

class Delay
{
private:
  std::queue< std::tuple< unsigned int, Packet > > _queue;
  const unsigned int _delay;

public:
  Delay( const unsigned int s_delay ) : _queue(), _delay( s_delay ) {}
 
  void accept( Packet && p, const unsigned int tickno ) noexcept
  {
    _queue.emplace( tickno + _delay, std::move( p ) );
  }

  template <class NextHop>
  void tick( NextHop & next, const unsigned int tickno )
  {
    while ( (!_queue.empty()) && (std::get< 0 >( _queue.front() ) <= tickno) ) {
      next.accept( std::move( std::get< 1 >( _queue.front() ) ), tickno );
      _queue.pop();
    }
  }
};

#endif
