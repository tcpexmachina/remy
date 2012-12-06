#include "dumbsender.hh"

DumbSender::DumbSender( unsigned int s_id, const double s_rate, const double s_switch_rate )
  : _id( s_id ),
    _packets_sent( 0 ),
    _sending_process( s_rate ),
    _switching_process( s_switch_rate ),
    _active( false ),
    /* stats */
    total_packets( 0 ),
    total_delay( 0 )
{
}

void DumbSender::tick( Network & net, Receiver & rec, const int tickno )
{
  const int switch_num = _switching_process.tick();
  if ( switch_num % 2 ) {
    _active = !_active;
    printf( "%d tick %d: now %s\n", _id, tickno, _active ? "sending" : "dormant" );
  }

  if ( !_active ) {
    return;
  }

  /* Send */
  const int num = _sending_process.tick();

  for ( int i = 0; i < num; i++ ) {
    net.accept( Packet( _id, _packets_sent++, tickno ) );
  }

  /* Receive feedback */
  const std::vector< Packet > packets = rec.collect( _id );

  for ( auto &x : packets ) {
    total_packets++;
    total_delay += x.tick_received - x.tick_sent;
  }

  /* Print stats */
  if ( tickno % 1000000 == 0 ) {
    printf( "%d tick %d: avg throughput = %.4f, avg delay = %.4f\n",
	    _id,
	    tickno,
	    double( total_packets ) / double( tickno ),
	    double( total_delay ) / double( total_packets ) );
  }
}
