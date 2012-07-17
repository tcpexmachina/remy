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
  int _current_tick;

public:
  DumbSender( unsigned int s_id, const double s_rate );

  void advance( const int tick_to, Network & rec );
};

#endif
