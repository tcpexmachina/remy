#ifndef SENDER_DATA_POINT_HH
#define SENDER_DATA_POINT_HH

#include "simulationresults.pb.h"
#include "dna.pb.h"
#include "memory.hh"

/**
 * Represents the data that is logged relating to a single sender at a single
 * point in time. Since this class is intended to do nothing than hold data,
 * it just takes protobuf objects directly from relevant classes.
 */
class SenderDataPoint
{

public:

  SimulationResultBuffers::SenderDataPoint DNA( void ) const
  {
    SimulationResultBuffers::SenderDataPoint ret;
    ret.mutable_sender_state()->CopyFrom( sender_state );
    ret.mutable_utility_data()->CopyFrom( utility_data );
    ret.set_sending( sending );
    ret.set_packets_in_flight( packets_in_flight );
    return ret;
  }

  SenderDataPoint( SimulationResultBuffers::SenderState sender_state,
      SimulationResultBuffers::UtilityData utility_data,
      bool sending ) :
    sender_state( sender_state ),
    utility_data( utility_data ),
    sending( sending ),
    packets_in_flight( 0 ) {};

  void set_packets_in_flight( unsigned int _packets )
  {
    packets_in_flight = _packets;
  }

private:

  // composite protobufs from other objects
  SimulationResultBuffers::SenderState sender_state;
  SimulationResultBuffers::UtilityData utility_data;

  // information directly stored in this object
  bool sending = false;
  unsigned int packets_in_flight = 0;
};

#endif // SENDER_DATA_POINT_HH
