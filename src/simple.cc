#include <algorithm>
#include <limits>

#include "simple.hh"

using namespace std;

Simple::Simple( void )
  :  _memory(),
     _last_send_time( 0 ),
     _intersend_time( 1 ),
     _flow_id( 0 ),
     _largest_ack( -1 )
{
}

void Simple::packets_received( const vector< Packet > & packets ) {
  /* Assumption: There is no reordering */
  _largest_ack = max( packets.at( packets.size() - 1 ).seq_num, _largest_ack );
  _memory.packets_received( packets, _flow_id  );
}

void Simple::reset( const double & )
{
  _memory.reset();
  _last_send_time = 0;
  _intersend_time = 1;
  _flow_id++;
  _largest_ack = packets_sent() - 1; /* Assume everything's been delivered */
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
