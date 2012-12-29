#ifndef SENDERGANG_HH
#define SENDERGANG_HH

#include <vector>
#include <tuple>

#include "exponential.hh"
#include "receiver.hh"

template <class SenderType, class NextHop>
class SenderGang
{
private:
  std::vector< std::tuple< unsigned int, SenderType > > _gang;

  Exponential _start_distribution, _stop_distribution;

public:
  SenderGang( const double mean_on_duration,
	      const double mean_off_duration,
	      const unsigned int num_senders,
	      const SenderType & exemplar );

  void tick( NextHop & next, Receiver & rec, const unsigned int tickno );

  double utility( void ) const;
};

#endif
