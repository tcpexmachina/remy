#ifndef RAT_HH
#define RAT_HH

#include <vector>
#include <string>

#include "packet.hh"
#include "whiskertree.hh"
#include "memory.hh"

class Rat
{
private:
  WhiskerTree _whiskers;
  Memory _memory;

  unsigned int _packets_sent, _packets_received;

  bool _track;

  double _internal_tick;

public:
  Rat( const WhiskerTree & s_whiskers, const bool s_track=false );

  void packets_received( const std::vector< Packet > & packets );
  void dormant_tick( const unsigned int tickno ); /* do nothing */

  template <class NextHop>
  void send( const unsigned int id, NextHop & next, const unsigned int tickno );

  const WhiskerTree & whiskers( void ) const { return _whiskers; }
};

#endif
