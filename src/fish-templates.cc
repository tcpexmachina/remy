#include <cassert>
#include <utility>

#include "fish.hh"

using namespace std;

template <class NextHop>
void Fish::send( const unsigned int id, NextHop & next, const double & tickno )
{
  assert( _packets_sent >= _largest_ack + 1 );
  Packet p( id, _flow_id, tickno, _packets_sent );
  _packets_sent++;
  _memory.packet_sent( p );
  next.accept( p, tickno );
  _update_send_time( tickno );
}
