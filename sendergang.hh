#ifndef SENDERGANG_HH
#define SENDERGANG_HH

#include <vector>
#include <tuple>

#include "window-sender.hh"
#include "exponential.hh"

template <class NextHop>
class SenderGang
{
private:
  std::vector< std::tuple< unsigned int, WindowSender<NextHop> > > _gang;

  Exponential _start_distribution, _stop_distribution;

public:
  SenderGang( const double mean_on_duration,
	      const double mean_off_duration,
	      const unsigned int num_senders,
	      const WindowSender<NextHop> & exemplar );

  void tick( NextHop & next, Receiver & rec, const unsigned int tickno );

  double utility( void ) const;
};

#endif
