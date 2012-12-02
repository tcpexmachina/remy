#ifndef NETWORK_HH
#define NETWORK_HH

#include <queue>

#include "receiver.hh"
#include "packet.hh"
#include "poisson.hh"

class Network
{
private:
  std::queue< Packet > _buffer;
  Poisson _egress_process;

public:
  Network( const double s_rate );

  void accept( const Packet && p );
  void tick( Receiver & rec, const int tickno );
};

#endif
