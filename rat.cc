#include "rat.hh"

using namespace std;

Rat::Rat( const Whiskers & s_whiskers, const bool s_track )
  :  _whiskers( s_whiskers ),
     _memory(),
     _packets_sent( 0 ),
     _packets_received( 0 ),
     _the_window( 100 ),
     _track( s_track )
{
}

void Rat::packets_received( const vector< Packet > & packets ) {
  _packets_received += packets.size();
  _memory.packets_received( packets );

  if ( !packets.empty() ) {
    _memory.advance_to( packets.back().tick_received );
    _the_window = _whiskers.use_whisker( _memory, _track ).window();
    _memory.new_window( _the_window );
  }
}

void Rat::dormant_tick( const unsigned int tickno __attribute((unused)) )
{
  _memory = Memory();
}

