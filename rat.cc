#include "rat.hh"

using namespace std;

Rat::Rat( const Whiskers & s_whiskers )
  :  _whiskers( s_whiskers ),
     _memory(),
     _packets_sent( 0 ),
     _packets_received( 0 )
{
}

void Rat::packets_received( const vector< Packet > & packets ) {
  _packets_received += packets.size();
  _memory.packets_received( packets );
}

void Rat::dormant_tick( const unsigned int tickno __attribute((unused)) )
{
}

unsigned int Rat::window( const unsigned int tickno )
{
  _memory.advance_to( tickno );
  return _whiskers.get_whisker( _memory ).window();
}

const typename Rat::Whisker & Rat::Whiskers::get_whisker( const Rat::Memory & _memory __attribute((unused)) )
{
  return _whiskers[ 0 ]; /* XXX */
}

Rat::Whisker::Whisker()
  : _generation( 0 ),
    _window( 100 ),
    _count( 0 )
{
}
