#ifndef SIMULATIONRESULTS_HH
#define SIMULATIONRESULTS_HH

#include "network.hh"
#include "whiskertree.hh"
#include "fintree.hh"
#include "simulationresults.pb.h"
#include "senderdatapoint.hh"
#include <vector>

class SimulationRunData;
class SimulationRunDataPoint;

// top level results class
template <typename ActionTree>
class SimulationResults
{
public:
  SimulationResults() : actions(), run_data() {};
  SimulationResults( ActionTree actions ) : actions( actions ), run_data() {};

  SimulationResultBuffers::SimulationsData DNA( void ) const;

  // Adds a run and returns a reference to it
  SimulationRunData & add_run_data( const NetConfig & config );

private:
  void _populate_actions( SimulationResultBuffers::SimulationsData & pb ) const;

  ActionTree actions;
  std::vector< struct SimulationRunData > run_data;

  // problem settings
  const unsigned int prng_seed = 0;
  const unsigned int tick_count = 0;
};

// Generate machine code for these classes, since it doesn't seem to be generated without prompt.
template class SimulationResults< WhiskerTree >;
template class SimulationResults< FinTree >;

class SimulationRunData
{
  friend class SimulationResults< WhiskerTree >;
  friend class SimulationResults< FinTree >;

public:
  SimulationRunData( NetConfig config ) : config( config ), data() {};

  // Adds a data point and returns a reference to it
  struct SimulationRunDataPoint & add_datum( double seconds );

private:
  NetConfig config;
  std::vector< struct SimulationRunDataPoint > data;
};

class SimulationRunDataPoint
{
  friend SimulationResultBuffers::SimulationsData SimulationResults< WhiskerTree >::DNA ( void ) const;
  friend SimulationResultBuffers::SimulationsData SimulationResults< FinTree >::DNA ( void ) const;

public:
  SimulationRunDataPoint( double seconds ) : seconds( seconds ), sender_data() {};

  // expects pairs of < throughput, delay >
  void add_sender_data( std::vector< SenderDataPoint > );

private:
  double seconds;
  std::vector< SenderDataPoint > sender_data;
};

#endif // SIMULATIONRESULTS_HH