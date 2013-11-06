#ifndef LINK_HH
#define LINK_HH

#include <queue>

#include "packet.hh"
#include "poisson.hh"

class Link
{
private:
  std::queue< Packet > _buffer;
  const double _rate;
  double _next_delivery_time;

public:
  Link( const double s_rate );

  void accept( Packet && p ) noexcept { _buffer.push( std::move( p ) ); }

  template <class NextHop>
  void tick( NextHop & next, const unsigned int tickno );
};

#endif
