#ifndef RAT_HH
#define RAT_HH

#include <vector>
#include <string>
#include <limits>

#include "packet.hh"
#include "whiskertree.hh"
#include "memory.hh"

class Rat
{
private:
  const WhiskerTree & _whiskers;
  Memory _memory;
  std::vector< Memory::DataType > _initial_state;

  bool _track;

  double _last_send_time;

  double _intersend_time;

  unsigned int _flow_id;

public:
  Rat( WhiskerTree & s_whiskers, const bool s_track=false );

  void packets_received( const std::vector< Packet > & packets );
  void reset( const double & tickno ); /* start new flow */

  template <class NextHop>
  void send( const unsigned int id, NextHop & next, const double & tickno,
	     const int packets_sent_cap = std::numeric_limits<int>::max() );

  const WhiskerTree & whiskers( void ) const { return _whiskers; }

  Rat & operator=( const Rat & ) { assert( false ); return *this; }

  double next_event_time( const double & tickno ) const;

  const int & packets_sent( void ) const { return _memory.packets_sent(); }

  void set_initial_state( const std::vector< Memory::DataType > & data );
  const std::vector<double> get_state( const double & tickno );
};

#endif
