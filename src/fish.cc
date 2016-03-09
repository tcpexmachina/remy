#include <limits>
#include <algorithm>

#include "fish.hh"

using namespace std;

Fish::Fish( const FinTree & fins, const unsigned int s_prng_seed, const bool s_track )
  :  _fins( fins ),
     _memory(),
     _packets_sent( 0 ),
     _packets_received( 0 ),
     _last_send_time( 0 ),
     _next_send_time( 0 ),
     _flow_id( 0 ),
     _largest_ack( -1 ), 
     _track( s_track ),
     _lambda( 0 ),
     _max_intersend( 0 ),
     _batch_size( 5 ),
     _prng( s_prng_seed ),
     _distribution( 1 )
{
}

void Fish::packets_received( const vector< Packet > & packets ) {
    _packets_received += packets.size();
    _memory.packets_received( packets, _flow_id, _largest_ack );
    _largest_ack = max( packets.at( packets.size() - 1 ).seq_num, _largest_ack );
    
    const Fin & current_fin( _fins.use_fin( _memory, _track ) );
    _update_lambda( current_fin.lambda() );
    _update_send_time( _last_send_time );
}

void Fish::reset( const double & )
{
  _memory.reset();
  _last_send_time = 0;
  _next_send_time = 0;
  _lambda = 0;
  _max_intersend = 0;
  _flow_id++;
  /* Give up on everything sent so far that hasn't been acked,
     Fixes the problem of tail losses */
  _largest_ack = _packets_sent - 1;
  assert( _flow_id != 0 );
}

double Fish::next_event_time( const double & tickno ) const
{
  if (_next_send_time == 0) {
    return tickno;
  } else {
    return _next_send_time;
  }
}

void Fish::_update_send_time( const double tickno )
{
  _last_send_time = tickno;
  _next_send_time = _last_send_time + _batch_size * min( _distribution.sample( _prng ), _max_intersend );
}

void Fish::_update_lambda( const double lambda ) 
{
  _lambda = lambda;
  _max_intersend = 2.0 / lambda;
  _distribution.set_lambda( lambda );
}

SimulationResultBuffers::SenderState Fish::state_DNA() const
{
  SimulationResultBuffers::SenderState ret;
  ret.mutable_memory()->CopyFrom( _memory.DNA() );
  ret.set_packets_sent( _packets_sent );
  ret.set_lambda( _lambda );
  return ret;
}
