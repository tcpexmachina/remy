#ifndef DUMBSENDER_HH
#define DUMBSENDER_HH

#include "poisson.hh"
#include "network.hh"

class DumbSender
{
private:
  unsigned int _id;
  int _packets_sent;
  Poisson _sending_process;

  /* stats */
  uint64_t total_packets, total_delay;

public:
  DumbSender( const unsigned int s_id, const double s_rate );

  void tick( Network & net, Receiver & rec, const unsigned int tickno );
};

#endif
