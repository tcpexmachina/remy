#include <algorithm>
#include <limits>

#include "simple.hh"

using namespace std;

Simple::Simple( void )
  :  _memory(),
     _packets_sent( 0 ),
     _packets_received( 0 ),
     _last_send_time( 0 ),
     _intersend_time( 1 ),
     _flow_id( 0 ),
     _largest_ack( -1 )
{
}

void Simple::packets_received( const vector< Packet > & packets ) {
  _packets_received += packets.size();
  /* Assumption: There is no reordering */
  _largest_ack = max( packets.at( packets.size() - 1 ).seq_num, _largest_ack );
  _memory.packets_received( packets, _flow_id, _packets_sent - _packets_received );
}

void Simple::reset( const double & )
{
  _memory.reset();
  _last_send_time = 0;
  _intersend_time = 1;
  _flow_id++;
  _largest_ack = _packets_sent - 1; /* Assume everything's been delivered */
  assert( _flow_id != 0 );
}

double Simple::next_event_time( const double & tickno ) const
{
  if ( _last_send_time + _intersend_time <= tickno ) {
    return tickno;
  } else {
    return _last_send_time + _intersend_time;
  }
}
