#include "simulationresults.hh"

template <typename ActionTree>
SimulationRunData & SimulationResults< ActionTree >::add_run_data( const NetConfig & config, double interval )
{

  run_data.emplace_back( config, interval );
  return run_data.back();
}

SimulationRunDataPoint & SimulationRunData::add_datum( double seconds )
{
  _data.emplace_back( seconds );
  return _data.back();
}

void SimulationRunDataPoint::add_sender_data( std::vector< SenderDataPoint > new_data )
{
  _sender_data.insert( _sender_data.end(), new_data.begin(), new_data.end() );
}

void SimulationRunDataPoint::add_network_data( std::vector< unsigned int > packets_in_flight )
{
  assert( packets_in_flight.size() == _sender_data.size() );
  for (unsigned int i = 0; i < packets_in_flight.size(); i++) {
    _sender_data[i].set_packets_in_flight( packets_in_flight[i] );
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
    run_data_pb->set_log_interval_ticks( run.interval() );
    run_data_pb->mutable_config()->CopyFrom( run.config().DNA() );

    for ( const auto &datum : run.data() ) {
      SimulationResultBuffers::SimulationRunDataPoint * data_pb = run_data_pb->add_point();
      data_pb->set_seconds( datum.seconds() );

      for (const auto &sender_datum : datum.sender_data() ) {
        SimulationResultBuffers::SenderDataPoint * sender_data_pb = data_pb->add_sender_data();
        sender_data_pb->CopyFrom( sender_datum.DNA() );
      }
    }
  }

  return ret;
}
