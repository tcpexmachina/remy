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
    Whiskers();

    void packet_sent( const Packet & packet );
    void packets_received( const std::vector< Packet > & packets );
    void advance_to( const unsigned int tickno );
    unsigned int window( void ) const;
  };

private:
  unsigned int _window;
  unsigned int _packets_sent, _packets_received;

public:
  Rat( const unsigned int s_window );

  void packets_received( const std::vector< Packet > & packets );
  void send( const unsigned int id, NextHop & next, const unsigned int tickno );
  void dormant_tick( const unsigned int tickno ); /* do nothing */
};

#endif
