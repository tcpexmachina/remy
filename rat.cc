#include "rat.hh"

using namespace std;

Rat::Rat( const Whiskers & s_whiskers, const bool s_track )
  :  _whiskers( s_whiskers ),
     _memory(),
     _packets_sent( 0 ),
     _packets_received( 0 ),
     _track( s_track ),
     _internal_tick( 0 )
{
  _whiskers.reset_counts();
}

void Rat::packets_received( const vector< Packet > & packets ) {
  _packets_received += packets.size();
  _memory.packets_received( packets );
}

void Rat::dormant_tick( const unsigned int tickno )
{
  _memory = Memory();
  _internal_tick = tickno;
}

