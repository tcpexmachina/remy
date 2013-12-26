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
  private:
    double internal_tick;

  protected:
    unsigned int id;
    double next_switch_tick;
    SenderType sender;

    /* is abstract base class */
    virtual void switcher( const double & tickno,
			   Exponential & start_distribution,
			   Exponential & stop_distribution,
			   const unsigned int num_sending ) = 0;

    void switch_on( const double & tickno );
    void switch_off( const double & tickno, const unsigned int num_sending );

    void accumulate_sending_time_until( const double & tickno, const unsigned int num_sending );

    void receive_feedback( Receiver & rec, const double & tickno );

  public:
    double next_event_time( const double & tickno ) const;
    Utility utility;
    bool sending;

    SwitchedSender( const unsigned int s_id,
		    const double & start_tick,
		    const SenderType & s_sender )
      : internal_tick( 0 ),
	id( s_id ),
	next_switch_tick( start_tick ),
	sender( s_sender ),
	utility(),
	sending( false )
    {}
  };

  class TimeSwitchedSender : public SwitchedSender {
  public:
    template <class NextHop>
    void tick( NextHop & next, Receiver & rec,
	       const double & tickno,
	       const unsigned int num_sending,
	       Exponential & start_distribution ) override;

    void switcher( const double & tickno,
		   Exponential & start_distribution,
		   Exponential & stop_distribution,
		   const unsigned int num_sending ) override;

    using SwitchedSender::SwitchedSender;
  };

  class ByteSwitchedSender : public SwitchedSender {
  private:
    unsigned int packets_sent_cap_ { 0 };

  public:
    template <class NextHop>
    void tick( NextHop & next, Receiver & rec,
	       const double & tickno,
	       const unsigned int num_sending,
	       Exponential & start_distribution ) override;

    void switcher( const double & tickno,
		   Exponential & start_distribution,
		   Exponential & stop_distribution,
		   const unsigned int num_sending ) override;

    using SwitchedSender::SwitchedSender;
  };

  std::vector< TimeSwitchedSender > _gang;

  Exponential _start_distribution, _stop_distribution;

public:
  SenderGang( const double mean_on_duration,
	      const double mean_off_duration,
	      const unsigned int num_senders,
	      const SenderType & exemplar,
	      PRNG & s_prng );

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
