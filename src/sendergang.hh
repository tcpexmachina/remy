#ifndef SENDERGANG_HH
#define SENDERGANG_HH

#include <vector>

#include "exponential.hh"
#include "receiver.hh"
#include "utility.hh"

template <class SenderType>
class SenderGang
{
private:
  class SwitchedSender {
  public:
    unsigned int id;
    unsigned int next_switch_tick;
    bool sending;
    SenderType sender;
    Utility utility;

    SwitchedSender( const unsigned int s_id,
		    const unsigned int start_tick,
		    const SenderType & s_sender )
      : id( s_id ),
	next_switch_tick( start_tick ),
	sending( false ),
	sender( s_sender ),
	utility()
    {}

    void switcher( const unsigned int tickno,
		   Exponential & start_distribution,
		   Exponential & stop_distribution );

    template <class NextHop>
    void tick( NextHop & next, Receiver & rec,
	       const unsigned int tickno  );
  };

  std::vector< SwitchedSender > _gang;

  Exponential _start_distribution, _stop_distribution;

public:
  SenderGang( const double mean_on_duration,
	      const double mean_off_duration,
	      const unsigned int num_senders,
	      const SenderType & exemplar,
	      PRNG & s_prng );

  template <class NextHop>
  void tick( NextHop & next, Receiver & rec, const unsigned int tickno );

  double utility( void ) const;
  std::vector< std::pair< double, double > > throughputs_delays( void ) const;

  const std::vector< const SenderType * > senders( void ) const;
};

#endif
