#include <limits>

#include "rat.hh"

using namespace std;

Rat::Rat( WhiskerTree & s_whiskers, const bool s_track )
  :  _whiskers( s_whiskers ),
     _memory(),
     _packets_sent( 0 ),
     _packets_received( 0 ),
     _track( s_track ),
     _internal_tick( 0 ),
     _the_window( 0 ),
     _intersend_time( 0 ),
     _flow_id( 0 )
{
}

void Rat::packets_received( const vector< Packet > & packets ) {
  _packets_received += packets.size();
  _memory.packets_received( packets, _flow_id );

  const Whisker & current_whisker( _whiskers.use_whisker( _memory, _track ) );

  _the_window = current_whisker.window( _the_window );
  _intersend_time = current_whisker.intersend();
}

void Rat::reset( const double & tickno )
{
  _memory.reset();
  _internal_tick = tickno;
  _the_window = 0;
  _intersend_time = 0;
  _flow_id++;
  assert( _flow_id != 0 );
}

double Rat::next_event_time( const double & tickno ) const
{
  if ( _packets_sent < _packets_received + _the_window ) {
    if ( _internal_tick > tickno ) {
      return _internal_tick + _intersend_time;
    } else {
      return tickno; /* right now */
    }
  } else {
    /* window is currently closed */
    return std::numeric_limits<double>::max();
  }
}
