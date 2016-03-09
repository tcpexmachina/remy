#ifndef SENDERGANG_HH
#define SENDERGANG_HH

#include <vector>

#include "exponential.hh"
#include "receiver.hh"
#include "utility.hh"
#include "senderdatapoint.hh"

template <class SenderType>
class SwitchedSender {
private:
  double internal_tick;

protected:
  double next_switch_tick;
  SenderType sender;

  /* is abstract base class */
  virtual void switcher( const double & tickno,
			 PRNG & prng,
			 Exponential & start_distribution,
			 Exponential & stop_distribution,
			 const unsigned int num_sending ) = 0;

  void accumulate_sending_time_until( const double & tickno, const unsigned int num_sending );

  void receive_feedback( Receiver & rec );

public:
  void switch_on( const double & tickno );
  void switch_off( const double & tickno, const unsigned int num_sending );

  double next_event_time( const double & tickno ) const;
  SenderDataPoint statistics_for_log( void ) const;
  Utility utility;
  bool sending;
  unsigned int id;

  SwitchedSender( const unsigned int s_id,
		  const double & start_tick,
		  const SenderType & s_sender )
    : internal_tick( 0 ),
      next_switch_tick( start_tick ),
      sender( s_sender ),
      utility(),
      sending( false ),
      id( s_id )
  {}

  virtual ~SwitchedSender() {}
};

template <class SenderType>
class TimeSwitchedSender : public SwitchedSender<SenderType> {
public:
  template <class NextHop>
  void tick( NextHop & next, Receiver & rec,
	     const double & tickno,
	     const unsigned int num_sending,
	     PRNG & prng,
	     Exponential & start_distribution );

  void switcher( const double & tickno,
		 PRNG & prng,
		 Exponential & start_distribution,
		 Exponential & stop_distribution,
		 const unsigned int num_sending ) override;

  using SwitchedSender<SenderType>::SwitchedSender;
};

template <class SenderType>
class ByteSwitchedSender : public SwitchedSender<SenderType> {
private:
  unsigned int packets_sent_cap_ { 0 };

public:
  template <class NextHop>
  void tick( NextHop & next, Receiver & rec,
	     const double & tickno,
	     const unsigned int num_sending,
	     PRNG & prng,
	     Exponential & start_distribution );

  void switcher( const double & tickno,
		 PRNG & prng,
		 Exponential & start_distribution,
		 Exponential & stop_distribution,
		 const unsigned int num_sending ) override;

  using SwitchedSender<SenderType>::SwitchedSender;
};

template <class SenderType>
class ExternalSwitchedSender : public SwitchedSender<SenderType> {
public:
  template <class NextHop>
  void tick( NextHop & next, Receiver & rec,
	     const double & tickno,
	     const unsigned int num_sending,
	     PRNG & prng,
	     Exponential & start_distribution );

  void switcher( const double &,
		 PRNG &,
		 Exponential &,
		 Exponential &,
		 const unsigned int ) override
  {
    SwitchedSender<SenderType>::next_switch_tick = std::numeric_limits<double>::max();
  } /* don't switch */

  using SwitchedSender<SenderType>::SwitchedSender;    
};

template <class SenderType, class SwitcherType>
class SenderGang
{
private:
  std::vector< SwitcherType > _gang;

  PRNG & _prng;

  Exponential _start_distribution, _stop_distribution;

public:
  typedef SenderType Sender;

  SenderGang( const double mean_on_duration,
	      const double mean_off_duration,
	      const unsigned int num_senders,
	      const SenderType & exemplar,
	      PRNG & s_prng,
	      const unsigned int id_range_begin = 0 );

  /* Create empty SenderGang */
  SenderGang();

  unsigned int count_active_senders( void ) const;
  size_t count_senders( void ) const { return _gang.size(); }
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
  std::vector< SenderDataPoint > statistics_for_log( void ) const;

  double next_event_time( const double & tickno ) const;

  SwitcherType & mutable_sender( const unsigned int num ) { return _gang.at( num ); }
  const SwitcherType & sender( const unsigned int num ) const { return _gang.at( num ); }
};

#endif
