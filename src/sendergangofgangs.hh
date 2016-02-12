#ifndef SENDERGANGOFGANGS_HH
#define SENDERGANGOFGANGS_HH

#include <vector>
#include <utility>
#include "receiver.hh"
#include "senderdatapoint.hh"

template <class Gang1Type, class Gang2Type>
class SenderGangofGangs
{
private:
  Gang1Type gang1_;
  Gang2Type gang2_;

public:
  SenderGangofGangs( const Gang1Type & gang1,
		     const Gang2Type & gang2 );

  unsigned int count_active_senders( void ) const;

  unsigned int count_senders( void ) const { return gang1_.count_senders() + gang2_.count_senders(); }

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

  const Gang1Type & gang1( void ) const { return gang1_; }
  const Gang2Type & gang2( void ) const { return gang2_; }

  Gang1Type & mutable_gang1( void ) { return gang1_; }
  Gang2Type & mutable_gang2( void ) { return gang2_; }
};

#endif
