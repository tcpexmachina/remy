#include <algorithm>
#include <limits>

#include "rat.hh"

using namespace std;

Rat::Rat( WhiskerTree & s_whiskers, const bool s_track )
  :  _whiskers( s_whiskers ),
     _memory(),
     _using_whisker( _whiskers.use_whisker( _memory, _track ) ),
     _packets_sent( 0 ),
     _packets_received( 0 ),
     _track( s_track ),
     _last_send_time( 0 ),
     _the_window( 0 ),
     _intersend_time( 0.01 ),
     _flow_id( 0 ),
     _largest_ack( -1 )
{
}

void Rat::packets_received( const vector< Packet > & packets ) {
  _packets_received += packets.size();
  /* Assumption: There is no reordering */
  _largest_ack = max( packets.at( packets.size() - 1 ).seq_num, _largest_ack );
  _memory.packets_received( packets, _flow_id );

  const Whisker & current_whisker( _whiskers.use_whisker( _memory, _track ) );

  if( ! ( current_whisker == _using_whisker ) ) {
    _using_whisker = current_whisker;
  }
  
  _the_window = current_whisker.window( _the_window );
  _intersend_time = current_whisker.intersend();
}

void Rat::reset( const double & )
{
  //_memory.reset();
  _last_send_time = 0;
  _the_window = 0;
  _intersend_time = 0.01;
  _flow_id++;
  _largest_ack = _packets_sent - 1; /* Assume everything's been delivered */
  assert( _flow_id != 0 );

  /* initial window and intersend time */
  const Whisker & current_whisker( _whiskers.use_whisker( _memory, _track ) );
  _the_window = current_whisker.window( _the_window );
  _intersend_time = current_whisker.intersend();
}

void Rat::set_mem( const std::vector< Memory::DataType > & data )
{
  printf("set %f\n", data.at( 0 ));
  _memory = Memory( data );
}

double Rat::next_event_time( const double & tickno ) const
{
  if ( _packets_sent < _largest_ack + 1 + _the_window ) {
    if ( _last_send_time + _intersend_time <= tickno ) {
      return tickno;
    } else {
      return _last_send_time + _intersend_time;
    }
  } else {
    /* window is currently closed */
    return std::numeric_limits<double>::max();
  }
}
