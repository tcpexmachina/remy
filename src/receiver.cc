#include <cassert>
#include <limits>

#include "receiver.hh"

Receiver::Receiver()
  : _collector()
{
}

void Receiver::accept( const Packet & p, const double & tickno ) noexcept
{
  autosize( p.src );

  _collector[ p.src ].push_back( p );
  _collector[ p.src ].back().tick_received = tickno;
}

void Receiver::autosize( const unsigned int index )
{
  if ( index >= _collector.size() ) {
    _collector.resize( index + 1 );
  }
}

double Receiver::next_event_time( const double & tickno ) const
{
  for ( const auto & x : _collector ) {
    if ( not x.empty() ) {
      return tickno;
    }
  }
  return std::numeric_limits<double>::max();
}
