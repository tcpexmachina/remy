#ifndef PACKET_HH
#define PACKET_HH

class Packet
{
public:
  unsigned int src;
  unsigned int flow_id;
  double tick_sent, tick_received;

  Packet( const unsigned int & s_src,
	  const unsigned int & s_flow_id, const double & s_tick_sent )
    : src( s_src ),
      flow_id( s_flow_id ), tick_sent( s_tick_sent ),
      tick_received( -1 )
  {}

  Packet( const Packet & other ) = delete;

  Packet( Packet && other ) = default;
};

#endif
