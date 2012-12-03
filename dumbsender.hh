#ifndef DUMBSENDER_HH
#define DUMBSENDER_HH

#include "poisson.hh"
#include "network.hh"

class DumbSender
{
private:
  unsigned int _id;
  int _packets_sent;
  Poisson _sending_process, _switching_process;
  bool _active;

  /* stats */
  uint64_t total_packets, total_delay;

public:
  DumbSender( unsigned int s_id, const double s_rate, const double s_switch_rate );

  void tick( Network & net, Receiver & rec, const int tickno );
};

#endif
