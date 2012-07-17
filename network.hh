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
  int _current_tick;
  Poisson _egress_process;

public:
  Network( const double s_rate );

  void send( const Packet & p );
  void advance( const int tick_to, Receiver & rec );
};

#endif
