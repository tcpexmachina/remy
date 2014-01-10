#ifndef RECEIVER_HH
#define RECEIVER_HH

#include <vector>

#include "packet.hh"

class Receiver
{
private:
  std::vector< std::vector< Packet > > _collector;
  void autosize( const unsigned int index );

public:
  Receiver();

  void accept( const Packet & p, const double & tickno ) noexcept;
  const std::vector< Packet > & packets_for( const unsigned int src ) { return _collector[ src ]; }
  void clear( const unsigned int src ) { _collector[ src ].clear(); }
  bool readable( const unsigned int src ) const noexcept
  { return (src < _collector.size()) && (!_collector[ src ].empty()); }

  double next_event_time( const double & tickno ) const;
};

#endif
