#ifndef NETWORK_HH
#define NETWORK_HH

#include "peekable_queue.hh"

#include "receiver.hh"
#include "packet.hh"
#include "poisson.hh"

class Network
{
private:
  std::queue< Packet > _buffer;
  Poisson _egress_process;

  unsigned int _used_pdos, _underflows;

public:
  Network( const double s_rate );

  void accept( Packet && p ) noexcept;

  void tick( Receiver & rec, const unsigned int tickno );

  unsigned int used_pdos( void ) const { return _used_pdos; }
  unsigned int underflows( void ) const { return _underflows; }
};

#endif
