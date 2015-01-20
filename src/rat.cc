#include <algorithm>
#include <limits>

#include "rat.hh"

using namespace std;

Rat::Rat( WhiskerTree & s_whiskers, const bool s_track )
  :  _whiskers( s_whiskers ),
     _memory(),
     _track( s_track ),
     _last_send_time( 0 ),
     _intersend_time( 1 ),
     _flow_id( 0 )
{
}

void Rat::packets_received( const vector< Packet > & packets ) {
  /* Assumption: There is no reordering */
  _memory.packets_received( packets, _flow_id );
}

void Rat::reset( const double & )
{
  _memory.reset();
  _last_send_time = 0;
  _intersend_time = 1;
  _flow_id++;
  assert( _flow_id != 0 );
}

double Rat::next_event_time( const double & tickno ) const
{
  if ( _last_send_time + _intersend_time <= tickno ) {
    return tickno;
  } else {
    return _last_send_time + _intersend_time;
  }
}
