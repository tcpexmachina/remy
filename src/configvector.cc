#include "configvector.hh"

using namespace std;

RemyBuffers::ConfigVector ConfigVector::DNA ( void ) {
  RemyBuffers::ConfigVector ret;
  for ( auto &config : configs) {
    // record the config to the protobuf
    RemyBuffers::NetConfig* net_config = ret.add_config();
    *net_config = config.DNA();
  }
  return ret;
}

// Reads an input protobuf of points, converts to ConfigVector
void ConfigVector::ReadVector(RemyBuffers::ConfigVector vector) {
  for (int i=0; i < vector.config_size(); i++) {
    const RemyBuffers::NetConfig &config = vector.config(i);
    configs.push_back( NetConfig().set_link_ppt( config.link_ppt() ).set_delay( config.delay() ).set_num_senders( config.num_senders() ).set_on_duration( config.mean_on_duration() ).set_off_duration( config.mean_off_duration() ).set_buffer_size( config.buffer_size() ));
  }
}



