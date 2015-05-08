#include <algorithm>
#include <limits>

#include "rat.hh"

using namespace std;

Rat::Rat( WhiskerTree & s_whiskers, const bool s_track )
  :  _whiskers( s_whiskers ),
     _memory(),
     _initial_state(),
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
  _memory.reset_to( _initial_state );
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

void Rat::set_initial_state( const vector< Memory::DataType > & data )
{
  _initial_state = data;
}

const std::vector<double> Rat::get_state( const double & tickno __attribute((unused)) )
{
  std::vector<double> state;
  //state.push_back( _memory.rec_ewma() );
  //state.push_back( _memory.outstanding_packets() );
  state.push_back( _memory.imputed_delay() );
  state.push_back( _intersend_time );
  return state;
}
