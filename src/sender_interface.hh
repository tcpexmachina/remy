#ifndef SENDER_INTERFACE_HH
#define SENDER_INTERFACE_HH

#include "packet.hh"
#include "link.hh"

class SenderInterface
{
public:
  virtual void packets_received( const std::vector< Packet > & packets ) = 0;
  virtual void reset( const double & tickno ) = 0;
  virtual std::vector<Packet> send( const unsigned int id, const double & tickno ) = 0;
  virtual double next_event_time( const double & tickno ) const = 0;
};

#endif  // SENDER_INTERFACE_HH
