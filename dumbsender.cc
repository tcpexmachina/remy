#include "dumbsender.hh"

DumbSender::DumbSender( unsigned int s_id, const double s_rate )
  : _id( s_id ),
    _packets_sent( 0 ),
    _sending_process( s_rate )
{
}

void DumbSender::tick( Network & rec, const int tickno )
{
  const int num = _sending_process.sample();

  for ( int i = 0; i < num; i++ ) {
    rec.send( Packet( _id, _packets_sent++, tickno ) );
  }
}
