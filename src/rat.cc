#include <algorithm>
#include <limits>

#include "rat.hh"

using namespace std;

Rat::Rat( WhiskerTree & s_whiskers, const bool s_track )
  :  _whiskers( s_whiskers ),
     _memory(),
     _track( s_track ),
     _last_send_time( 0 ),
     _the_window( 0 ),
     _intersend_time( 0 ),
     _flow_id( 0 ),
     _largest_ack( -1 )
{
  /* initial window and intersend time */
  const Whisker & current_whisker( _whiskers.use_whisker( _memory, _track ) );
  _the_window = current_whisker.window( _the_window );
  _intersend_time = current_whisker.intersend();
}

void Rat::packets_received( const vector< Packet > & packets ) {
  /* Assumption: There is no reordering */
  _largest_ack = max( packets.at( packets.size() - 1 ).seq_num, _largest_ack );
  _memory.packets_received( packets, _flow_id );
}

void Rat::reset( const double & )
{
  _memory.reset();
  _last_send_time = 0;
  _the_window = 0;
  _intersend_time = 0;
  _flow_id++;
  _largest_ack = _memory.num_packets_sent() - 1; /* Assume everything's been delivered */
  assert( _flow_id != 0 );

  if ( _the_window == 0 ) {
    /* initial window and intersend time */
    const Whisker & current_whisker( _whiskers.use_whisker( _memory, _track ) );
    _the_window = current_whisker.window( _the_window );
    _intersend_time = current_whisker.intersend();
  }
}

double Rat::next_event_time( const double & tickno ) const
{
  if ( _memory.num_packets_sent() < _largest_ack + 1 + _the_window ) {
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

const vector<double> Rat::get_state( const double & tickno __attribute((unused)) )
{
  std::vector<double> state;
  state.reserve( 5 );
  for( unsigned int i = 0; i < _memory.datasize; i++ ) {
    state.push_back( _memory.field( i ) );
  }
  //state.push_back( _intersend_time );
  state.push_back( _the_window );
  state.push_back( _memory.last_tick_received() - tickno );
  return state;
}
