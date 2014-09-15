#include <cassert>
#include <utility>

#include "aimd.hh"

using namespace std;

/* I am assuming this is going to be called as soon as packets_received
   is called */
template <class NextHop>
void Aimd::send( const unsigned int id, NextHop & next, const double & tickno )
{
  assert( int(_packets_sent) >= _largest_ack + 1 );
  while ( int(_packets_sent) < _largest_ack + 1 + _the_window ) {
    Packet p( id, _flow_id, tickno, _packets_sent );
    _packets_sent++;
    next.accept( p, tickno );
  }
}
