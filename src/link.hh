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

  const std::queue< Packet >::size_type _limit;

public:
  Link( const double s_rate, const decltype( _limit ) s_limit = std::numeric_limits<decltype( _limit )>::max() ) : _buffer(), _pending_packet( 1.0 / s_rate ), _limit( s_limit ) {}

  void accept( Packet && p, const double & tickno ) noexcept {
    if ( _pending_packet.empty() ) {
      _pending_packet.accept( std::move( p ), tickno );
    } else {
     if ( _buffer.size() < _limit ) {
        _buffer.push( std::move( p ) );
     }
    }
  }

  template <class NextHop>
  void tick( NextHop & next, const double & tickno );

  double next_event_time( const double & tickno ) const { return _pending_packet.next_event_time( tickno ); }
};

#endif
