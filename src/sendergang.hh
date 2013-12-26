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

public:
  SenderGang( const double mean_on_duration,
	      const double mean_off_duration,
	      const unsigned int num_senders,
	      const SenderType & exemplar,
	      PRNG & s_prng,
	      const unsigned int id_range_begin = 0 );

  unsigned int count_active_senders( void ) const;
  unsigned int count_senders( void ) const { return _gang.size(); }
  unsigned int id_of_first_sender( void ) const { return _gang.at( 0 ).id; }

  void switch_senders( const unsigned int num_sending, const double & tickno );

  template <class NextHop>
  void run_senders( NextHop & next, Receiver & rec,
		    const unsigned int num_sending,
		    const double & tickno );

  template <class NextHop>
  void tick( NextHop & next, Receiver & rec, const double & tickno );

  double utility( void ) const;

  std::vector< std::pair< double, double > > throughputs_delays( void ) const;

  double next_event_time( const double & tickno ) const;
};

template <class Sender1, class Sender2>
class SenderGangofGangs
{
private:
  SenderGang<Sender1> gang1_;
  SenderGang<Sender2> gang2_;

public:
  SenderGangofGangs( SenderGang<Sender1> && gang1,
		     SenderGang<Sender2> && gang2 );

  unsigned int count_active_senders( void ) const;

  void switch_senders( const unsigned int num_sending, const double & tickno );

  template <class NextHop>
  void run_senders( NextHop & next, Receiver & rec,
		    const unsigned int num_sending,
		    const double & tickno );

  template <class NextHop>
  void tick( NextHop & next, Receiver & rec, const double & tickno );

  double utility( void ) const;

  std::vector< std::pair< double, double > > throughputs_delays( void ) const;

  double next_event_time( const double & tickno ) const;
};

#endif
