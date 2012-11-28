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

public:
  DumbSender( unsigned int s_id, const double s_rate );

  void tick( Network & rec, const int tickno );
};

#endif
