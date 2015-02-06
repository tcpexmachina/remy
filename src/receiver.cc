#include <cassert>
#include <limits>

#include "receiver.hh"

Receiver::Receiver()
  : _collector()
{
}

void Receiver::accept( const Packet & p, const double & tickno ) noexcept
{
  _collector[ p.src ].push_back( p );
  _collector[ p.src ].back().tick_received = tickno;
}

double Receiver::next_event_time( const double & tickno ) const
{
  for ( const auto & x : _collector ) {
    if ( (not x.second.empty()) and (x.first > 0) ) {
      return tickno;
    }
  }
  return std::numeric_limits<double>::max();
}
