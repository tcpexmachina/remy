#ifndef RAT_HH
#define RAT_HH

#include <vector>

#include "packet.hh"

class Rat
{
private:
  class Memory {
  public:
    double last_delay;

    void packet_sent( const Packet & packet __attribute((unused)) ) {}
    void packets_received( const std::vector< Packet > & packets __attribute((unused)) ) {}
    void advance_to( const unsigned int tickno __attribute((unused)) ) {}
  };

public:
  class Whisker {
  private:
    unsigned int _generation;
    unsigned int _window;
    unsigned int _count;

  public:
    Whisker();
    unsigned int window( void ) const { return _window; }
  };

  class Whiskers {
  private:
    std::vector< Whisker > _whiskers;

  public:
    Whiskers() : _whiskers( 1 ) {}
    const Whisker & get_whisker( const Memory & _memory );
  };

private:
  Whiskers _whiskers;
  Memory _memory;

  unsigned int window( const unsigned int tickno );

  unsigned int _packets_sent, _packets_received;

public:
  Rat( const Whiskers & s_whiskers );

  void packets_received( const std::vector< Packet > & packets );
  void dormant_tick( const unsigned int tickno ); /* do nothing */

  template <class NextHop>
  void send( const unsigned int id, NextHop & next, const unsigned int tickno );
};

#endif
