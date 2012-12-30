#ifndef RAT_HH
#define RAT_HH

#include <vector>

#include "packet.hh"

template <class NextHop>
class Rat
{
public:
  class Whiskers {
  public:
    Whiskers() {}

    void packet_sent( const Packet & packet __attribute((unused)) ) {}
    void packets_received( const std::vector< Packet > & packets __attribute((unused)) ) {}
    unsigned int window( const unsigned int tickno __attribute((unused)) ) const { return 100; }
  };

private:
  Whiskers _whiskers;
  unsigned int _packets_sent, _packets_received;

public:
  Rat( const Whiskers & s_whiskers );

  void packets_received( const std::vector< Packet > & packets );
  void send( const unsigned int id, NextHop & next, const unsigned int tickno );
  void dormant_tick( const unsigned int tickno ); /* do nothing */
};

#endif
