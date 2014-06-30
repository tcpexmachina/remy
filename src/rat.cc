#include <algorithm>
#include <limits>

#include "rat.hh"

using namespace std;

Rat::Rat( WhiskerTree & s_whiskers, const uint64_t current_time, const bool s_track )
  :  _whiskers( s_whiskers ),
     _memory(),
     _packets_sent( 0 ),
     _packets_received( 0 ),
     _track( s_track ),
     _last_send_time( current_time ),
     _the_window( 0 ),
     _intersend_time( 0 ),
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

  _the_window = current_whisker.window( _the_window );
  _intersend_time = current_whisker.intersend();
}

void Rat::reset( const double & )
{
  _memory.reset();
  _last_send_time = 0;
  _the_window = 0;
  _intersend_time = 0;
  _flow_id++;
  _largest_ack = _packets_sent - 1; /* Assume everything's been delivered */
  assert( _flow_id != 0 );
}

std::vector< Packet > Rat::send( const unsigned int id,
				 const double & tickno,
				 const int packets_sent_cap )
{
  assert( _packets_sent >= _largest_ack + 1 );

  if ( _the_window == 0 ) {
    /* initial window and intersend time */
    const Whisker & current_whisker( _whiskers.use_whisker( _memory, _track ) );
    _the_window = current_whisker.window( _the_window );
    _intersend_time = current_whisker.intersend();
  }

  if ( (_packets_sent < _largest_ack + 1 + _the_window)
       and (_last_send_time + _intersend_time <= tickno) ) {

    /* Have we reached the end of the flow for now? */
    if ( _packets_sent >= packets_sent_cap ) {
      return std::vector< Packet >();
    }

    Packet p( id, _flow_id, tickno, _packets_sent );
    _packets_sent++;
    _memory.packet_sent( p );
    _last_send_time = tickno;
    return std::vector< Packet >( 1, p );
  } else {
    return std::vector< Packet >();
  }
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
