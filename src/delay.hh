#ifndef DELAY_HH
#define DELAY_HH

#include <deque>
#include <tuple>
#include <cassert>
#include <limits>
#include <cstdio>

#include "packet.hh"

class Delay
{
private:
  std::deque< std::tuple< double, Packet > > _queue;
  const double _delay;

public:
  Delay( const double s_delay ) : _queue(), _delay( s_delay ) {}
 
  void accept( const Packet & p, const double & tickno ) noexcept
  {
    _queue.emplace_back( tickno + _delay, p );
  }

  template <class NextHop>
  void tick( NextHop & next, const double & tickno )
  {
    while ( (!_queue.empty()) && (std::get< 0 >( _queue.front() ) <= tickno) ) {
      assert( std::get< 0 >( _queue.front() ) == tickno );
      next.accept( std::get< 1 >( _queue.front() ), tickno );
      _queue.pop_front();
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

  std::vector<unsigned int> packets_in_flight( const unsigned int num_senders ) const
  {
    std::vector<unsigned int> ret( num_senders );
    for ( const auto & x : _queue ) {
      ret.at( std::get<1>( x ).src )++;
    }
    return ret;
  }
};

#endif
