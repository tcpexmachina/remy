#ifndef RECEIVER_HH
#define RECEIVER_HH

#include <unordered_map>
#include <vector>

#include "packet.hh"

class Receiver
{
private:
  std::unordered_map< int, std::vector< Packet > > _collector;

public:
  Receiver();

  void accept( const Packet & p, const double & tickno ) noexcept;
  const std::vector< Packet > & packets_for( const int src ) { return _collector[ src ]; }
  void clear( const int src ) { _collector[ src ].clear(); }
  bool readable( const int src ) noexcept
  { return (src < int(_collector.size())) && (!_collector[ src ].empty()); }

  double next_event_time( const double & tickno ) const;
};

#endif
