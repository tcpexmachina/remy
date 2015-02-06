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
        const unsigned int s_packets __attribute((unused)) = 0,
	const unsigned int s_limit = std::numeric_limits<unsigned int>::max() )
    : _buffer(), _pending_packet( 1.0 / s_rate ), _limit( s_limit ) 
  {
    /* Initialize the buffer with some dummy packets */
    for ( unsigned int i = 0; i < s_packets; i++ ) {
      _buffer.emplace( -1, -1, 0, i );
    }
  }

  void accept( const Packet & p, const double & tickno ) noexcept {
    if ( _pending_packet.empty() ) {
      _pending_packet.accept( p, tickno );
    } else {
      if ( _buffer.size() < _limit ) {
        _buffer.push( p );
      }
    }
  }

  template <class NextHop>
  void tick( NextHop & next, const double & tickno );

  double next_event_time( const double & tickno ) const { return _pending_packet.next_event_time( tickno ); }

  unsigned int buffer_size( void ) const { return _buffer.size(); }
};

#endif
