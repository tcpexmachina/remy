#include <cassert>
#include <utility>

#include "rat.hh"

using namespace std;

template <class NextHop>
void Rat::send( const unsigned int id, NextHop & next, const double & tickno,
		const unsigned int packets_sent_cap )
{
  assert( _memory.num_packets_sent() >= _largest_ack + 1 );

  if ( ( _memory.num_packets_sent() < _largest_ack + 1 + _the_window)
       and (_last_send_time + _intersend_time <= tickno) ) {

    /* Have we reached the end of the flow for now? */
    if (  _memory.num_packets_sent() >= packets_sent_cap ) {
      return;
    }

    Packet p( id, _flow_id, tickno, _memory.num_packets_sent() );
    _memory.packet_sent( p );
    next.accept( p, tickno );
    _last_send_time = tickno;

    const Whisker & current_whisker( _whiskers.use_whisker( _memory, _track ) );
    _the_window = current_whisker.window( _the_window );
    _intersend_time = current_whisker.intersend();
  }
}
