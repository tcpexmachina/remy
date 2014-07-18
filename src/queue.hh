/* -*-mode:c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

#ifndef QUEUE_HH
#define QUEUE_HH

#include <queue>
#include <cstdint>
#include <string>
#include <fstream>
#include <memory>

#include "packet.hh"

class Queue
{
protected:
    const static unsigned int PACKET_SIZE = 1504; /* default max TUN payload size */

    struct QueuedPacket
    {
        int bytes_to_transmit;
        Packet contents;
        double arrival_time;

        QueuedPacket( const Packet & p, const double & s_arrival_tick );
    };

    unsigned int next_delivery_;
    std::vector< double > schedule_;
    double base_timestamp_;

    std::queue< QueuedPacket > packet_queue_;

    bool repeat_;

    /* Needs to be overloaded in case schedule is built 
       dynamically, e.g. in PoissonQueue. */
    virtual void use_a_delivery_opportunity( void ) = 0;

public:
    Queue( void );

    double next_event_time( const double & tickno ) const;
    double next_scheduled_time( void ) const;

    void accept( const Packet & p, const double & tickno );

    template <class NextHop>
    void tick( NextHop & next, const double & tickno );
};


class LinkQueue : public Queue
{
private:
    void use_a_delivery_opportunity( void );

public:
    LinkQueue( const double & link_packets_per_ms );
};

#endif /* QUEUE_HH */
