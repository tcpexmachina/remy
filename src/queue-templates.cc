#include <cassert>

#include "queue.hh"

template <class NextHop>
void Queue::tick( NextHop & next, const double & tickno )
{
  double now = tickno;

  if ( not packet_queue_.empty() ) {
    assert( packet_queue_.front().arrival_time <= next_event_time( tickno ) );
  }
  
  while ( next_event_time( tickno ) <= now ) {
    const double this_delivery_time = next_event_time( tickno );
    
    /* burn a delivery opportunity */
    unsigned int bytes_left_in_this_delivery = PACKET_SIZE;
    use_a_delivery_opportunity( schedule_.front() );
    
    while ( (bytes_left_in_this_delivery > 0)
            and (not packet_queue_.empty())
            and (packet_queue_.front().arrival_time <= this_delivery_time) ) {
      packet_queue_.front().bytes_to_transmit -= bytes_left_in_this_delivery;
      bytes_left_in_this_delivery = 0;
      
      if ( packet_queue_.front().bytes_to_transmit <= 0 ) {
        /* restore the surplus bytes beyond what the packet requires */
        bytes_left_in_this_delivery += (- packet_queue_.front().bytes_to_transmit);
        
        /* this packet is ready to go */
        next.accept( packet_queue_.front().contents, tickno );
        packet_queue_.pop();
      }
    }
  }
}
