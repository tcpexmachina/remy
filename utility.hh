#ifndef UTILITY_HH
#define UTILITY_HH

#include <math.h>

class Utility
{
private:
  unsigned int _ticks_sending, _packets_received;
  unsigned int _total_delay;

public:
  Utility( void ) : _ticks_sending( 0 ), _packets_received( 0 ), _total_delay( 0 ) {}

  void sending_tick( void ) { _ticks_sending++; }
  void packets_received( const std::vector< Packet > & packets ) {
    _packets_received += packets.size();

    for ( auto &x : packets ) {
      assert( x.tick_received >= x.tick_sent );
      _total_delay += x.tick_received - x.tick_sent;
    }
  }

  double utility( void ) const
  {
    if ( (_packets_received == 0) || (_ticks_sending == 0) ) {
      return -INT_MAX;
    }
    const double average_delay = double( _total_delay ) / double( _packets_received );

    const double throughput_utility = log2( double( _packets_received ) / double( _ticks_sending ) );
    const double delay_penalty = log2( average_delay / 100.0 );

    return throughput_utility - delay_penalty;
  }
};

#endif
