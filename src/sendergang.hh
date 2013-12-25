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
    double next_switch_tick;
    bool sending;
    SenderType sender;
    Utility utility;
    double internal_tick;

    SwitchedSender( const unsigned int s_id,
		    const double & start_tick,
		    const SenderType & s_sender )
      : id( s_id ),
	next_switch_tick( start_tick ),
	sending( false ),
	sender( s_sender ),
	utility(),
	internal_tick( 0 )
    {}

    template <class NextHop>
    void tick( NextHop & next, Receiver & rec,
	       const double & tickno,
	       const unsigned int num_sending );

    void switcher( const double & tickno,
		   Exponential & start_distribution,
		   Exponential & stop_distribution,
		   const unsigned int num_sending );

    double next_event_time( const double & tickno ) const;
  };

  std::vector< SwitchedSender > _gang;

  Exponential _start_distribution, _stop_distribution;

  unsigned int _num_sending;

public:
  SenderGang( const double mean_on_duration,
	      const double mean_off_duration,
	      const unsigned int num_senders,
	      const SenderType & exemplar,
	      PRNG & s_prng );

  unsigned int switch_senders( unsigned int old_num_sending, const double & tickno );

  template <class NextHop>
  void tick( NextHop & next, Receiver & rec, const double & tickno );

  double utility( void ) const;
  std::vector< std::pair< double, double > > throughputs_delays( void ) const;

  double next_event_time( const double & tickno ) const;
};

#endif
