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
  SimulationRunData & add_run_data( const NetConfig & config, double interval );

  void set_prng_seed( unsigned int prng_seed ) { this->prng_seed = prng_seed; }
  void set_tick_count( unsigned int tick_count ) { this->tick_count = tick_count; }
  void set_log_interval_ticks( unsigned int log_interval_ticks ) { this->log_interval_ticks = log_interval_ticks; }

private:
  void _populate_actions( SimulationResultBuffers::SimulationsData & pb ) const;

  ActionTree actions;
  std::vector< struct SimulationRunData > run_data;

  // problem settings
  unsigned int prng_seed = 0;
  unsigned int tick_count = 0;
  unsigned int log_interval_ticks = 0;
};

template class SimulationResults< WhiskerTree >;
template class SimulationResults< FinTree >;


class SimulationRunData
{
public:
  SimulationRunData( NetConfig config, double interval ) : _config( config ), _interval( interval ), _data() {};

  // Adds a data point and returns a reference to it
  struct SimulationRunDataPoint & add_datum( double seconds );

  const NetConfig & config() const { return _config; }
  double interval() const { return _interval; }
  const std::vector< struct SimulationRunDataPoint > & data() const { return _data; }

private:
  NetConfig _config;
  double _interval;
  std::vector< struct SimulationRunDataPoint > _data;
};

class SimulationRunDataPoint
{
public:
  SimulationRunDataPoint( double seconds ) : _seconds( seconds ), _sender_data() {};

  void add_sender_data( std::vector< SenderDataPoint > );
  void add_network_data( std::vector< unsigned int > );

  double seconds() const { return _seconds; }
  const std::vector< SenderDataPoint > & sender_data() const { return _sender_data; }

private:
  double _seconds;
  std::vector< SenderDataPoint > _sender_data;
};

#endif // SIMULATIONRESULTS_HH