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
  std::deque< std::tuple< double, Packet, bool > > _queue;
  /* queue members: release time, contents, whether release time was adjusted after-the-fact */
  double _delay;
  bool _adjusted_packets_are_in_flight;

  void fixup_adjusted_packets( const double tickno )
  {
    if ( not _adjusted_packets_are_in_flight ) {
      return;
    }

    /* for packets that were in-flight when delay was reduced,
       make sure that they get released asap */
    for ( auto & p : _queue ) {
      if ( std::get< 2 >( p ) and std::get< 0 >( p ) < tickno ) {
	std::get< 0 >( p ) = tickno;
	std::get< 2 >( p ) = false;
      }
    }

    _adjusted_packets_are_in_flight = false;
  }

public:
  Delay( const double s_delay ) : _queue(), _delay( s_delay ), _adjusted_packets_are_in_flight( false ) {}
 
  void accept( const Packet & p, const double & tickno ) noexcept
  {
    /* Make sure that we haven't reordered packets when delay was adjusted
       on packets already in-flight */
    if ( not _queue.empty() ) {
      assert( tickno + _delay >= std::get< 0 >( _queue.front() ) );
    }

    _queue.emplace_back( tickno + _delay, p, false );
  }

  template <class NextHop>
  void tick( NextHop & next, const double & tickno )
  {
    fixup_adjusted_packets( tickno );

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
    }

    if ( std::get< 0 >( _queue.front() ) < tickno
	 and std::get< 2 >( _queue.front() ) ) {
      return tickno; /* packet's delay was adjusted to be earlier than present time,
			so just release asap */
    }

    assert( std::get< 0 >( _queue.front() ) >= tickno );

    return std::get< 0 >( _queue.front() );
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

  void set_delay( const double delay )
  {
    /* Step 1: By how much is the delay changing? */
    const double delay_difference = delay - _delay;

    /* Step 2: Adjust existing packets-in-flight */
    for ( auto & p : _queue ) {
      std::get< 0 >( p ) += delay_difference;

      if ( delay_difference < 0 ) {
	std::get< 2 >( p ) = true;
	_adjusted_packets_are_in_flight = true;
      }
    }

    /* Step 3: Change delay that will be applied to future packets */
    _delay = delay;
  }

  const double & delay( void ) const { return _delay; }
};

#endif
