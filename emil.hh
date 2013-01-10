#ifndef EMIL_HH
#define EMIL_HH

#include <vector>

#include "packet.hh"
#include "poisson.hh"

class Emil
{
private:
  double _ewma;

  unsigned int _packets_sent, _packets_received;

  Poisson _fast_send_process, _slow_send_process;

  static constexpr double alpha = 0.9;

public:
  Emil( PRNG & prng );

  void packets_received( const std::vector< Packet > & packets );
  void dormant_tick( const unsigned int tickno ); /* do nothing */

  template <class NextHop>
  void send( const unsigned int id, NextHop & next, const unsigned int tickno );
};

#endif
