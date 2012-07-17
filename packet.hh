#ifndef PACKET_HH
#define PACKET_HH

class Packet
{
public:
  unsigned int src;
  int packet_id;
  int tick_sent, tick_received;

  Packet( int s_src, int s_packet_id, int s_tick_sent )
    : src( s_src ), packet_id( s_packet_id ), tick_sent( s_tick_sent ), tick_received( -1 )
  {}
};

#endif
