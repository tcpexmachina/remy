#ifndef SENDERGANG_HH
#define SENDERGANG_HH

#include <vector>
#include <tuple>

#include "exponential.hh"
#include "receiver.hh"
#include "utility.hh"

template <template< class NextHop > class SenderType, class NextHop>
class SenderGang
{
private:
  class SwitchedSender {
  public:
    unsigned int id;
    unsigned int next_switch_tick;
    bool sending;
    SenderType<NextHop> sender;
    Utility utility;

    SwitchedSender( const unsigned int s_id,
		    const unsigned int start_tick,
		    const SenderType<NextHop> & s_sender )
      : id( s_id ),
	next_switch_tick( start_tick ),
	sending( false ),
	sender( s_sender ),
	utility()
    {}

    void tick( NextHop & next, Receiver & rec,
	       const unsigned int tickno,
	       Exponential & start_distribution,
	       Exponential & stop_distribution );
  };

  std::vector< SwitchedSender > _gang;

  Exponential _start_distribution, _stop_distribution;

public:
  SenderGang( const double mean_on_duration,
	      const double mean_off_duration,
	      const unsigned int num_senders,
	      const SenderType<NextHop> & exemplar );

  void tick( NextHop & next, Receiver & rec, const unsigned int tickno );

  double utility( void ) const;
};

#endif
