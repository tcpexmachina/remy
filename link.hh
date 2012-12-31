#ifndef LINK_HH
#define LINK_HH

#include <queue>

#include "packet.hh"
#include "poisson.hh"

class Link
{
private:
  std::queue< Packet > _buffer;
  Poisson _egress_process;

public:
  Link( const double s_rate, PRNG & s_prng );

  void accept( Packet && p ) noexcept;

  template <class NextHop>
  void tick( NextHop & next, const unsigned int tickno );
};

#endif
