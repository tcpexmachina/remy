#ifndef LINK_HH
#define LINK_HH

#include <queue>

#include "packet.hh"
#include "delay.hh"

class Link
{
private:
  std::queue< Packet > _buffer;

  Delay _pending_packet;

public:
  Link( const double s_rate ) : _buffer(), _pending_packet( 1.0 / s_rate, "tx" ) {}

  void accept( Packet && p, const double & tickno ) noexcept {
    if ( _pending_packet.empty() ) {
      _pending_packet.accept( std::move( p ), tickno );
    } else {
      _buffer.push( std::move( p ) );
    }
  }

  template <class NextHop>
  void tick( NextHop & next, const double & tickno );

  double next_event_time( const double & tickno ) const { return _pending_packet.next_event_time( tickno ); }
};

#endif
