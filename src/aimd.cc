#include <limits>
#include <algorithm>

#include "aimd.hh"

using namespace std;

static constexpr double INITIAL_WINDOW = 1.0; /* INITIAL WINDOW OF 1 */

Aimd::Aimd()
  :  _packets_sent( 0 ),
     _packets_received( 0 ),
     _the_window( INITIAL_WINDOW ),
     _flow_id( 0 ),
     _largest_ack( -1 ),
     _slow_start( true ),
     _last_loss( -1 ),
     _rtt_at_loss( -1 )
{
}

void Aimd::packets_received( const vector< Packet > & packets ) {
  bool loss_detected = false;
  for ( auto & packet : packets ) {
    loss_detected = ( not loss_detected ) ?
                    ( packet.seq_num > _largest_ack + 1 ): /* At least lost one packet */
                    ( true ); /* Bypass seq_num check if you saw a loss already */
    _largest_ack = max( _largest_ack, packet.seq_num );

    _packets_received++;
    if ( packet.flow_id != _flow_id ) {
      /* This was from the previous flow, ignore it for congestion control */
      continue;
    }

    /* If in slow_start, exit slow start on detecting loss, else don't bother */
    _slow_start  = ( _slow_start ) ? ( loss_detected  ? false : _slow_start )
                                   : _slow_start;
    if ( loss_detected ) {
      if ( packet.tick_received > _last_loss + _rtt_at_loss ) {
        /* The DCCP approximation, reduce cwnd at most once per RTT */
        _the_window = _the_window / 2.0;
        _last_loss = packet.tick_received;
        _rtt_at_loss = packet.tick_received - packet.tick_sent;
      }
    } else {
      if ( _slow_start ) {
        _the_window += 1.0;
      } else {
        _the_window += 1.0 / _the_window;
      }
    }
    _the_window = max( INITIAL_WINDOW, _the_window );
  }
}

void Aimd::reset( const double & )
{
  _the_window = INITIAL_WINDOW;
  _flow_id++;
  _slow_start = true;
  /* Give up on everything sent so far that hasn't been acked,
     Fixes the problem of tail losses */
  _largest_ack = _packets_sent - 1;
  assert( _flow_id != 0 );
}

double Aimd::next_event_time( const double & tickno ) const
{
  if ( _packets_sent < _largest_ack + 1 + _the_window ) {
    return tickno;
  } else {
    return std::numeric_limits<double>::max();
  }
}
