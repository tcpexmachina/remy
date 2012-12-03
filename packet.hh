#ifndef PACKET_HH
#define PACKET_HH

#include <assert.h>

class Packet
{
public:
  unsigned int src;
  int packet_id;
  int tick_sent, tick_received;

  Packet( int s_src, int s_packet_id, int s_tick_sent )
    : src( s_src ), packet_id( s_packet_id ), tick_sent( s_tick_sent ), tick_received( -1 )
  {}

  Packet( const Packet & ) = delete;

  Packet( const Packet && other )
    : src( other.src ),
      packet_id( other.packet_id ),
      tick_sent( other.tick_sent ),
      tick_received( other.tick_received )
  {}
};

#endif
