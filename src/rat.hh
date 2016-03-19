#ifndef RAT_HH
#define RAT_HH

#include <vector>
#include <string>
#include <limits>

#include "packet.hh"
#include "whiskertree.hh"
#include "memory.hh"
#include "simulationresults.pb.h"

class Rat
{
private:
  const WhiskerTree & _whiskers;
  Memory _memory;

  unsigned int _packets_sent, _packets_received;

  bool _track;

  double _last_send_time;

  int _the_window;
  double _intersend_time;

  unsigned int _flow_id;
  int _largest_ack;

public:
  Rat( WhiskerTree & s_whiskers, const bool s_track=false );

  void packets_received( const std::vector< Packet > & packets );
  void reset( const double & tickno ); /* start new flow */

  template <class NextHop>
  void send( const unsigned int id, NextHop & next, const double & tickno,
	     const unsigned int packets_sent_cap = std::numeric_limits<unsigned int>::max() );

  const WhiskerTree & whiskers( void ) const { return _whiskers; }

  Rat & operator=( const Rat & ) { assert( false ); return *this; }

  double next_event_time( const double & tickno ) const;

  const unsigned int & packets_sent( void ) const { return _packets_sent; }

  SimulationResultBuffers::SenderState state_DNA() const;

};

#endif
