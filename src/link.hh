#ifndef LINK_HH
#define LINK_HH

#include <deque>
#include <vector>

#include "packet.hh"
#include "delay.hh"

class Link
{
private:
  std::deque< Packet > _buffer;

  Delay _pending_packet;

  unsigned int _limit;

public:
  Link( const double s_rate,
	const unsigned int s_limit )
    : _buffer(), _pending_packet( 1.0 / s_rate ), _limit( s_limit ) {}

  void accept( const Packet & p, const double & tickno ) noexcept {
    if ( _pending_packet.empty() ) {
      _pending_packet.accept( p, tickno );
    } else {
      if ( _limit and _buffer.size() < _limit ) {
        _buffer.push_back( p );
      }
    }
  }

  template <class NextHop>
  void tick( NextHop & next, const double & tickno );

  double next_event_time( const double & tickno ) const { return _pending_packet.next_event_time( tickno ); }

  std::vector<unsigned int> packets_in_flight( const unsigned int num_senders ) const
  {
    std::vector<unsigned int> ret( num_senders );
    for ( const auto & x : _buffer ) {
      ret.at( x.src )++;
    }
    std::vector<unsigned int> propagating = _pending_packet.packets_in_flight( num_senders );
    for ( unsigned int i = 0; i < num_senders; i++ ) {
      ret.at( i ) += propagating.at( i );
    }
    return ret;
  }

  void set_rate( const double rate ) { _pending_packet.set_delay( 1.0 / rate ); }
  double rate( void ) const { return 1.0 / _pending_packet.delay(); }
  void set_limit( const unsigned int limit )
  {
    _limit = limit;
    while ( _buffer.size() > _limit ) {
      _buffer.pop_back();
    }
  }
};

#endif
