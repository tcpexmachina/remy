#ifndef RAT_HH
#define RAT_HH

#include <vector>
#include <string>

#include "packet.hh"
#include "whisker.hh"
#include "memory.hh"

class Rat
{
private:
  Whiskers _whiskers;
  Memory _memory;

  unsigned int _packets_sent, _packets_received;

  unsigned int _the_window;

public:
  Rat( const Whiskers & s_whiskers );

  void packets_received( const std::vector< Packet > & packets );
  void dormant_tick( const unsigned int tickno ); /* do nothing */

  template <class NextHop>
  void send( const unsigned int id, NextHop & next, const unsigned int tickno );

  const Whiskers & whiskers( void ) const { return _whiskers; }
};

#endif
