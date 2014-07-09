#include "link.hh"

using namespace std;

void IsochronousLink::accept( const Packet & p, const double & tickno ) noexcept
{
  if( buffer_.empty() ) {
    // add transmission delay only
    buffer_.emplace( tickno + ( 1.0/rate_ ), p );
  } else if( buffer_.size() < limit_ ) {
    // there is already something on the queue;
    // add transmission delay plus queueing delay
    buffer_.emplace( buffer_.back().first + ( 1.0/rate_ ), p );
  }
}

void TraceLink::accept( const Packet & p, const double & tickno ) noexcept
{
  while( not trace_.empty() and (trace_.front() <= tickno) ) {
    // these potential release times have passed already
    trace_.pop();
  }

  if( not trace_.empty() and ( buffer_.size() < limit_ ) ) {
    double next_time = trace_.front();
    buffer_.emplace( next_time, p );
    trace_.pop();
  }

  // if reached end of trace, drop all incoming packets
}
