#include <limits>

#include "aimd.hh"

using namespace std;

Aimd::Aimd()
  :  _packets_sent( 0 ),
     _packets_received( 0 ),
     _the_window( INITIAL_WINDOW ),
     _flow_id( 0 ),
     _largest_ack( -1 ),
     _slow_start( true )
{
}

void Aimd::packets_received( const vector< Packet > & packets ) {
  bool loss_detected = false;
  for ( auto & packet : packets ) {
    assert( packet.seq_num > _largest_ack );
    loss_detected = ( not loss_detected ) ?
                    ( packet.seq_num > _largest_ack + 1 ): /* At least lost one packet */
                    ( true ); /* Bypass seq_num check if you saw a loss already */
    _largest_ack = max( _largest_ack, packet.seq_num );
    /* If in slow_start, exit slow start on detecting loss, else don't bother */
    _slow_start  = ( _slow_start ) ? ( loss_detected  ? false : _slow_start )
                                   : _slow_start;
  }
  _packets_received += packets.size();
  _the_window = loss_detected ? ( _the_window / 2.0 ) :
                                ( _slow_start ? ( _the_window + 1.0 )
                                              : ( _the_window + 1.0 / _the_window ) );
}

void Aimd::reset( const double & )
{
  _the_window = INITIAL_WINDOW;
  _flow_id++;
  _slow_start = true;
  assert( _flow_id != 0 );
}

double Aimd::next_event_time( const double & tickno __attribute ((unused)) ) const
{
  return std::numeric_limits<double>::max();
}
