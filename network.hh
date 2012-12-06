#ifndef NETWORK_HH
#define NETWORK_HH

#include <queue>

#include "receiver.hh"
#include "packet.hh"
#include "poissonptprocess.hh"

class Network
{
private:
  std::queue< Packet > _buffer;
  PoissonPointProcess _egress_process;

public:
  Network( const double s_rate );

  void accept( Packet && p ) noexcept;

  void tick( Receiver & rec, const int tickno );
};

#endif
