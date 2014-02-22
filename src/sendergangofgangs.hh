#ifndef SENDERGANGOFGANGS_HH
#define SENDERGANGOFGANGS_HH

#include <vector>
#include <utility>
#include "receiver.hh"
#include "sendergang.hh"

template <class Sender1, class Sender2>
class SenderGangofGangs
{
private:
  SenderGang<Sender1> gang1_;
  SenderGang<Sender2> gang2_;

public:
  SenderGangofGangs( const SenderGang<Sender1> & gang1,
		     const SenderGang<Sender2> & gang2 );

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

  std::vector< int > packets_in_flight( void ) const;
};

#endif
