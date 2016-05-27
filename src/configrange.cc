#include "configrange.hh"

using namespace std;

static RemyBuffers::Range pair_to_range( const Range &p )
{
  RemyBuffers::Range ret;
  ret.set_low( p.low );
  ret.set_high( p.high );
  ret.set_incr( p.incr );
  return ret;
}

ConfigRange::ConfigRange( void ) :
  link_ppt( Range() ),
  rtt( Range() ),
  mean_on_duration( Range() ),
  mean_off_duration( Range() ),
  num_senders( Range() ),
  buffer_size( Range() ),
  utility_penalty( 0 ),
  stochastic_loss_rate( 0 ),
  simulation_ticks( 1000000 )
{
}

ConfigRange::ConfigRange( RemyBuffers::ConfigRange input_config ) :
  link_ppt( Range( input_config.link_packets_per_ms() ) ),
  rtt( Range( input_config.rtt() ) ),
  mean_on_duration( Range( input_config.mean_on_duration() ) ),
  mean_off_duration( Range( input_config.mean_off_duration() ) ),
  num_senders( Range( input_config.num_senders() ) ),
  buffer_size( Range( input_config.buffer_size() ) ),
  utility_penalty( input_config.utility_penalty() ),
  stochastic_loss_rate( input_config.stochastic_loss_rate() ),
  simulation_ticks( input_config.simulation_ticks() )
{
}

RemyBuffers::ConfigRange ConfigRange::DNA( void ) const
{
  RemyBuffers::ConfigRange ret;
  ret.mutable_link_packets_per_ms()->CopyFrom( pair_to_range( link_ppt) );
  ret.mutable_rtt()->CopyFrom( pair_to_range( rtt ) );
  ret.mutable_num_senders()->CopyFrom( pair_to_range( num_senders ) );
  ret.mutable_mean_on_duration()->CopyFrom( pair_to_range( mean_on_duration ) );
  ret.mutable_mean_off_duration()->CopyFrom( pair_to_range( mean_off_duration ) );
  ret.mutable_buffer_size()->CopyFrom( pair_to_range( buffer_size ) );
  ret.set_utility_penalty( utility_penalty );
  ret.set_stochastic_loss_rate( stochastic_loss_rate );
  ret.set_simulation_ticks( simulation_ticks );

  return ret;
}
