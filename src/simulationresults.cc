#include <iostream>
#include "simulationresults.hh"

template <typename ActionTree>
SimulationRunData & SimulationResults< ActionTree >::add_run_data( const NetConfig & config )
{

  run_data.emplace_back( config );
  return run_data.back();
}

SimulationRunDataPoint & SimulationRunData::add_datum( double seconds )
{
  data.emplace_back( seconds );
  return data.back();
}

void SimulationRunDataPoint::add_sender_data( std::vector< std::pair < double, double > > sender_throughputs_delays )
{
  for ( std::pair < double, double > & input_data : sender_throughputs_delays ) {
    sender_data.push_back( {input_data.first, input_data.second} );
  }
}

template <>
void SimulationResults< WhiskerTree >::_populate_actions( SimulationResultBuffers::SimulationsData & pb ) const
{
  pb.mutable_whiskers()->CopyFrom( actions.DNA() );
}

template <>
void SimulationResults< FinTree >::_populate_actions( SimulationResultBuffers::SimulationsData & pb ) const
{
  pb.mutable_fins()->CopyFrom( actions.DNA() );
}

template <typename ActionTree>
SimulationResultBuffers::SimulationsData SimulationResults< ActionTree >::DNA( void ) const
{
  SimulationResultBuffers::SimulationsData ret;

  _populate_actions(ret);

  ProblemBuffers::ProblemSettings settings;
  settings.set_prng_seed( prng_seed );
  settings.set_tick_count( tick_count );
  ret.mutable_settings()->CopyFrom( settings );

  for ( const auto &run : run_data ) {
    SimulationResultBuffers::SimulationRunData * run_data_pb = ret.add_run_data();
    run_data_pb->mutable_config()->CopyFrom( run.config.DNA() );
    std::cerr << "There are " << run.data.size() << " data points." << std::endl;

    for ( const auto &datum : run.data ) {
      SimulationResultBuffers::SimulationRunDataPoint * data_pb = run_data_pb->add_point();
      data_pb->set_seconds( datum.seconds );

      for (const auto &sender_datum : datum.sender_data ) {
        SimulationResultBuffers::SenderDataPoint * sender_data_pb = data_pb->add_sender_data();
        sender_data_pb->set_average_throughput( sender_datum.average_throughput_since_start );
        sender_data_pb->set_average_delay( sender_datum.average_delay_since_start );
      }
    }
  }

  return ret;
}
