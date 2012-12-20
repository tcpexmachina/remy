#ifndef NETWORK_HH
#define NETWORK_HH

#include <queue>

#include "receiver.hh"
#include "packet.hh"
#include "poisson.hh"

template <class NextHop>
class Network
{
private:
  std::queue< Packet > _buffer;
  Poisson _egress_process;

public:
  Network( const double s_rate );

  void accept( Packet && p ) noexcept;

  void tick( NextHop & next, const unsigned int tickno );
};

#endif
