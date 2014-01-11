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

  unsigned int _limit;

public:
  Link( const double s_rate,
	const unsigned int s_limit = std::numeric_limits<unsigned int>::max() )
    : _buffer(), _pending_packet( 1.0 / s_rate ), _limit( s_limit ) {}

  void accept( Packet && p, const double & tickno ) noexcept {
    if ( _pending_packet.empty() ) {
      _pending_packet.accept( std::move( p ), tickno );
    } else {
      if ( _buffer.size() < _limit ) {
        _buffer.push( std::move( p ) );
        printf("%f: queue size %lu\n", tickno, _buffer.size());
      }
    }
  }

  template <class NextHop>
  void tick( NextHop & next, const double & tickno );

  double next_event_time( const double & tickno ) const { return _pending_packet.next_event_time( tickno ); }
};

#endif
