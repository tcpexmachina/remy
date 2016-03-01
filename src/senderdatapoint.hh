#ifndef SENDER_DATA_POINT_HH
#define SENDER_DATA_POINT_HH

#include "simulationresults.pb.h"
#include "dna.pb.h"
#include "memory.hh"

enum SenderDataPointSenderType { RAT, FISH };

/**
 * Represents the data that is logged relating to a single sender at a single
 * point in time.
 */
class SenderDataPoint
{

public:

  SimulationResultBuffers::SenderDataPoint DNA( void ) const
  {
    SimulationResultBuffers::SenderDataPoint ret;
    ret.set_average_throughput( average_throughput );
    ret.set_average_delay( average_delay );
    ret.set_sending_duration( sending_duration );
    ret.set_packets_received( packets_received );
    ret.set_total_delay( total_delay );
    if (type == RAT) {
      ret.set_window_size( window_size );
      ret.set_intersend_time( intersend_time );
    } else if (type == FISH) {
      ret.set_lambda( lambda );
    }
    ret.mutable_memory()->CopyFrom( memory.DNA() );
    ret.set_sending( sending );
    return ret;
  }

  SenderDataPoint( SenderDataPointSenderType type, Memory memory, double average_throughput,
      double average_delay, double sending_duration, unsigned int
      packets_received, double total_delay, unsigned int window_size, double
      intersend_time, double lambda, bool sending ) :
    type( type ),
    memory( memory ),
    average_throughput( average_throughput ),
    average_delay( average_delay ),
    sending_duration( sending_duration ),
    packets_received( packets_received ),
    total_delay( total_delay ),
    window_size( window_size ),
    intersend_time( intersend_time ),
    lambda( lambda ),
    sending( sending ) {};

private:
  SenderDataPointSenderType type;
  Memory memory;
  double average_throughput = 0;
  double average_delay = 0;
  double sending_duration = 0;
  unsigned int packets_received = 0;
  double total_delay = 0;
  unsigned int window_size = 0;
  double intersend_time = 0;
  double lambda = 0;
  bool sending = false;
};

#endif // SENDER_DATA_POINT_HH
