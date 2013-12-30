#ifndef AIMD_HH
#define AIMD_HH

#include <cassert>
#include <vector>
#include <string>

#include "packet.hh"

class Aimd
{
private:
  int _packets_sent, _packets_received;

  /* _the_window is the congestion window */
  double _the_window;
  unsigned int _flow_id;
  static constexpr double INITIAL_WINDOW = 1.0; /* INITIAL WINDOW OF 1 */

  /* Largest ACK so far */
  int _largest_ack;

  /* Are we in Slow Start? */
  bool _slow_start;

public:
  Aimd();

  void packets_received( const std::vector< Packet > & packets );
  void reset( const double & tickno ); /* start new flow */

  template <class NextHop>
  void send( const unsigned int id, NextHop & next, const double & tickno );

  Aimd & operator=( const Aimd & ) { assert( false ); }

  double next_event_time( const double & tickno ) const;
};

#endif
