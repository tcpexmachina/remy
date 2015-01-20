#ifndef SIMPLE_HH
#define SIMPLE_HH

#include <vector>
#include <string>
#include <limits>

#include "packet.hh"
#include "whiskertree.hh"
#include "memory.hh"

class Simple
{
private:
  Memory _memory;

  double _last_send_time;

  double _intersend_time;

  unsigned int _flow_id;
  int _largest_ack;

public:
  Simple( void );

  void packets_received( const std::vector< Packet > & packets );
  void reset( const double & tickno ); /* start new flow */

  template <class NextHop>
  void send( const unsigned int id, NextHop & next, const double & tickno,
	     const int packets_sent_cap = std::numeric_limits<int>::max() );

  Simple & operator=( const Simple & ) { assert( false ); return *this; }

  double next_event_time( const double & tickno ) const;

  const int & packets_sent( void ) const { return _memory.packets_sent(); }
};

#endif
