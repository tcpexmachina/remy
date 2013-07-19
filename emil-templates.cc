#include <assert.h>
#include <stdio.h>

#include "emil.hh"

template <class NextHop>
void Emil::send( const unsigned int id, NextHop & next, const unsigned int tickno )
{
  assert( _packets_sent >= _packets_received );

  unsigned int window = 110;

  const double adj_ewma = _ewma * (1 - alpha);

  if ( adj_ewma < 0.4 ) {
    window = 55;
  }

  Poisson & send_process( window > 100 ? _fast_send_process : _slow_send_process );

  fprintf( stderr, "%u adj_ewma=%f, window=%u\n", id, adj_ewma, window );

  const unsigned int packets_to_send( send_process.sample() );

  unsigned int _packets_sent_now = 0;

  while ( (_packets_sent < _packets_received + window)
	  && (_packets_sent_now < packets_to_send) ) {
    next.accept( Packet( id, _packets_sent++, tickno ) );
    _packets_sent_now++;
  }

  _ewma *= alpha;
}
