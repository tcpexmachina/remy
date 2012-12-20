#ifndef DELAY_HH
#define DELAY_HH

#include <queue>
#include <tuple>

#include "packet.hh"

template <class NextHop>
class Delay
{
private:
  std::queue< std::tuple< unsigned int, Packet > > _queue;
  const unsigned int _delay;

public:
  Delay( const unsigned int s_delay );
  
  void accept( Packet && p, const unsigned int tickno ) noexcept;
  void tick( NextHop & s_next, const unsigned int tickno );
};

#endif
