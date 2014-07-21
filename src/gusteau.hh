#ifndef GUSTEAU_HH
#define GUSTEAU_HH

#include <cassert>
#include <vector>
#include <string>
#include <limits>

#include "packet.hh"
#include "memory.hh"

class Gusteau
{
private:
  Memory _memory;

  int _packets_sent, _packets_received;

  double _last_send_time;
  double _max_receive_ratio;

  int _the_window;
  double _intersend_time;

  unsigned int _flow_id;
  int _largest_ack;
  int _flow_start;

public:
  Gusteau( void );

  void packets_received( const std::vector< Packet > & packets );
  void reset( const double & tickno ); /* start new flow */

  template <class NextHop>
  void send( const unsigned int id, NextHop & next, const double & tickno,
	     const unsigned int packets_sent_cap = std::numeric_limits<unsigned int>::max() );

  Gusteau & operator=( const Gusteau & ) { assert( false ); return *this; }

  double next_event_time( const double & tickno ) const;
};

#endif
