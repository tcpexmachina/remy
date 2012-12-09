#ifndef SENDERGANG_HH
#define SENDERGANG_HH

#include <queue>

#include "window-sender.hh"
#include "exponential.hh"

class SenderGang
{
private:
  std::priority_queue< std::pair< int, WindowSender > > _gang;

  Exponential _join_distribution;
  Exponential _flow_duration_distribution;
  const unsigned int _window_size;

  unsigned int _next_join_tick;

public:
  SenderGang( const double mean_interjoin_interval,
	      const double mean_flow_duration,
	      const unsigned int s_window_size );

  void tick( Network & net, Receiver & rec, const unsigned int tickno );
};

#endif
