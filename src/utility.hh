#ifndef UTILITY_HH
#define UTILITY_HH

#include <cmath>
#include <cassert>
#include <climits>
#include "simulationresults.pb.h"

class Utility
{
private:
  double _tick_share_sending;
  unsigned int _packets_received;
  double _total_delay;
  double _last_seqnum;
  double _loss_penalty_rate; // rate to penalize delay term in objective calculate for a loss
public:
  Utility( void ) : _tick_share_sending( 0 ), _packets_received( 0 ), _total_delay( 0 ), _last_seqnum( -1 ),  _loss_penalty_rate( 0 ) {}

  void sending_duration( const double & duration, const unsigned int num_sending ) { _tick_share_sending += duration / double( num_sending ); }
  void packets_received( const std::vector< Packet > & packets ) {
    _packets_received += packets.size();

    for ( auto &x : packets ) {
      assert( x.tick_received >= x.tick_sent );
      double outstanding_pkts = 1;
      if ( x.seq_num > _last_seqnum && ( _loss_penalty_rate > 0 ) ) {
        // if there is a positive loss penalty rate,
        // multiply the loss penalty by the outstanding packets amount
        outstanding_pkts = ( x.seq_num - _last_seqnum )*_loss_penalty_rate;
      }
      _last_seqnum = x.seq_num;
      _total_delay += ( x.tick_received - x.tick_sent )* outstanding_pkts;
    }
  }
  
  void set_delay_penalty( double penalty ) {
    _loss_penalty_rate = penalty;
  }

  /* returns throughput normalized to equal share of link */
  double average_throughput_normalized_to_equal_share( void ) const
  {
    if ( _tick_share_sending == 0 ) {
      return 0.0;
    }
    return double( _packets_received ) / _tick_share_sending;
  }

  double average_delay( void ) const
  {
    if ( _packets_received == 0 ) {
      return 0.0;
    }
    return double( _total_delay ) / double( _packets_received );
  }

  double utility( void ) const
  {
    if ( _tick_share_sending == 0 ) {
      return 0.0;
    }

    if ( _packets_received == 0 ) {
      return -INT_MAX;
    }

    const double throughput_utility = log2( average_throughput_normalized_to_equal_share() );
    const double delay_penalty = log2( average_delay() / 100.0 );

    return throughput_utility - delay_penalty;
  }

  SimulationResultBuffers::UtilityData DNA() const {
    SimulationResultBuffers::UtilityData ret;
    ret.set_sending_duration( _tick_share_sending );
    ret.set_packets_received( _packets_received );
    ret.set_total_delay( _total_delay );
    return ret;
  }

};

#endif
