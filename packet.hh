#ifndef PACKET_HH
#define PACKET_HH

#include <assert.h>

class Packet
{
public:
  unsigned int src, flow_id;
  unsigned int packet_id;
  unsigned int tick_sent, tick_received;

  Packet( const unsigned int & s_src, const unsigned int & s_flow_id,
	  const unsigned int & s_packet_id, const unsigned int & s_tick_sent )
    : src( s_src ), flow_id( s_flow_id ),
      packet_id( s_packet_id ), tick_sent( s_tick_sent ),
      tick_received( -1 )
  {}

  Packet( const Packet & ) = delete;

  Packet( const Packet && other )
    : src( other.src ),
      flow_id( other.flow_id ),
      packet_id( other.packet_id ),
      tick_sent( other.tick_sent ),
      tick_received( other.tick_received )
  {}
};

#endif
