#include "configrange.hh"

using namespace std;


ConfigRange::ConfigRange( void ) :
  configs(std::vector< NetConfig >()),
  simulation_ticks( 1000000 )
{
}

ConfigRange::ConfigRange( RemyBuffers::ConfigRange input_config ) :
  configs( std::vector< NetConfig >()),
  simulation_ticks( input_config.simulation_ticks() )
{
  // read the config vectors from the protobuf
  for (int i=0; i < input_config.configvector().config_size(); i++ ) {
  const RemyBuffers::NetConfig &config = input_config.configvector().config(i);
  configs.push_back( NetConfig().set_link_ppt( config.link_ppt() ).set_delay( config.delay() ).set_num_senders( config.num_senders() ).set_on_duration( config.mean_on_duration() ).set_off_duration( config.mean_off_duration() ).set_buffer_size( config.buffer_size() ));
   
  }

}

RemyBuffers::ConfigRange ConfigRange::DNA( void ) const
{
  RemyBuffers::ConfigVector vector;
  for ( auto &config : configs ) {
    RemyBuffers::NetConfig* net_config = vector.add_config();
    *net_config = config.DNA();
  }
  RemyBuffers::ConfigRange ret;
  ret.mutable_configvector()->CopyFrom( vector );
  ret.set_simulation_ticks( simulation_ticks );

  return ret;
}
