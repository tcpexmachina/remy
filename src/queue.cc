/* -*-mode:c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

#include <limits>
#include <cassert>

#include "packet.hh"
#include "queue.hh"

using namespace std;

Queue::QueuedPacket::QueuedPacket( const Packet & p, const double & s_arrival_tick )
    : bytes_to_transmit( PACKET_SIZE ),
      contents( p ),
      arrival_time( s_arrival_tick )
{}

/* construct a fixed schedule based on the supplied link rate  */
Queue::Queue( void )     
    : next_delivery_( 0 ),
      schedule_(),
      base_timestamp_( 0 ),
      packet_queue_(),
      repeat_( true )
{
}

/* construct a fixed schedule based on the supplied link rate  */
LinkQueue::LinkQueue( const double & link_packets_per_ms )     
    : Queue(),
      rate_( link_packets_per_ms )
{
    const double ms = 1.0/link_packets_per_ms;
    schedule_.push( ms );
}

void Queue::accept( const Packet & p, const double & tickno )
{
    const double now = tickno;

    /* pop wasted PDOs */
    while ( next_scheduled_time() <= now
            and (packet_queue_.empty() or packet_queue_.front().arrival_time > next_event_time( tickno )) ) {
        use_a_delivery_opportunity( now );
    }

    packet_queue_.emplace( p, now );
}

double Queue::next_event_time( const double & tickno __attribute ((unused)) ) const
{
    if( packet_queue_.empty() ) {
        return std::numeric_limits<double>::max();
    }
    return schedule_.front();
}

double Queue::next_scheduled_time( void ) const
{
    return schedule_.front();
}

void LinkQueue::use_a_delivery_opportunity( const double now )
{
    schedule_.pop();

    /* wraparound */
    if ( schedule_.empty() ) {
        schedule_.push( now + rate_ );
    } 
}
