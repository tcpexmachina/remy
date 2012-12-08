#ifndef DUMBSENDER_HH
#define DUMBSENDER_HH

#include "poissonptprocess.hh"
#include "network.hh"

class DumbSender
{
private:
  unsigned int _id;
  int _packets_sent;
  PoissonPointProcess _sending_process;

  /* stats */
  uint64_t total_packets, total_delay;

public:
  DumbSender( unsigned int s_id, const double s_rate );

  void tick( Network & net, Receiver & rec, const int tickno );
};

#endif
