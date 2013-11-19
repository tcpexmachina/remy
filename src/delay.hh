#ifndef DELAY_HH
#define DELAY_HH

#include <queue>
#include <tuple>
#include <cassert>
#include <limits>
#include <cstdio>
#include <string>

#include "packet.hh"

class Delay
{
private:
  std::queue< std::tuple< double, Packet > > _queue;
  const double _delay;
  std::string tag_;

public:
  Delay( const double s_delay, const std::string t_tag ) : _queue(), _delay( s_delay ), tag_( t_tag ) {}
 
  void accept( Packet && p, const double & tickno ) noexcept
  {
    _queue.emplace( tickno + _delay, std::move( p ) );
  }

  template <class NextHop>
  void tick( NextHop & next, const double & tickno )
  {
    while ( (!_queue.empty()) && (std::get< 0 >( _queue.front() ) <= tickno) ) {
      if (tag_ == "prop") printf("%f, received feedback\n", tickno);
      assert( std::get< 0 >( _queue.front() ) == tickno );
      next.accept( std::move( std::get< 1 >( _queue.front() ) ), tickno );
      _queue.pop();
    }
  }

  double next_event_time( const double & tickno ) const
  {
    if ( _queue.empty() ) {
      return std::numeric_limits<double>::max();
    } else {
      if ( tickno > std::get< 0 >( _queue.front() ) ) {
	fprintf( stderr, "Error, tickno = %f but packet should have been released at time %f\n",
		 tickno, std::get< 0 >( _queue.front() ) );
	assert( false );
      }
      return std::get< 0 >( _queue.front() );
    }
  }

  bool empty( void ) const { return _queue.empty(); }
};

#endif
