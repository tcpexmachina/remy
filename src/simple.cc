#include <algorithm>
#include <limits>

#include "simple.hh"

using namespace std;

Simple::Simple( void )
  :  _memory(),
     _initial_state(),
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
  _memory.reset_to( _initial_state );
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

void Simple::set_initial_state( const vector< Memory::DataType > & data )
{
  _initial_state = data;
}

const std::vector< double > Simple::get_state( const double & tickno ) 
{
  std::vector<double> state;
  state.push_back( _memory.rec_ewma() );
  state.push_back( _memory.outstanding_packets() );
  state.push_back( _intersend_time );
  //state.push_back( (1.0/10000.0) * int( 10000 * ( next_event_time( tickno ) - tickno )) );
  state.push_back( next_event_time( tickno ) - tickno );
  return state;
}
