#include <cassert>
#include <utility>

#include "simple.hh"

using namespace std;

template <class NextHop>
void Simple::send( const unsigned int id, NextHop & next, const double & tickno,
                   const int packets_sent_cap )
{
  if ( _last_send_time + _intersend_time <= tickno ) {
    /* Have we reached the end of the flow for now? */
    if ( packets_sent() >= packets_sent_cap ) {
      return;
    }

    Packet p( id, _flow_id, tickno, packets_sent() );
    _memory.packet_sent( p );
    next.accept( p, tickno );
    _last_send_time = tickno;
  }

  if( _memory.imputed_delay() > 150 ) {
    _intersend_time = 100000;
  } else {
    _intersend_time = 0.01;
  }
}
