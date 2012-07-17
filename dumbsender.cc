#include "dumbsender.hh"

DumbSender::DumbSender( unsigned int s_id, const double s_rate )
  : _id( s_id ),
    _packets_sent( 0 ),
    _sending_process( s_rate ),
    _current_tick( -1 )
{
}

void DumbSender::advance( const int tick_to, Network & rec )
{
  assert( _current_tick < tick_to );

  while ( _current_tick < tick_to ) {
    /* We do transmission just before departing each tick. */

    const int num = _sending_process.sample();

    for ( int i = 0; i < num; i++ ) {
      rec.send( Packet( _id, _packets_sent++, _current_tick ) );
    }

    _current_tick++;
  }

  assert( _current_tick == tick_to );
}
