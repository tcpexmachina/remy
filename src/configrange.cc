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
  simulation_ticks( 1000000 ),
  stochastic_loss_rate( Range().set_low(0).set_high(0).set_incr(0) )
{
}

ConfigRange::ConfigRange( RemyBuffers::ConfigRange input_config ) :
  link_ppt( Range( input_config.link_packets_per_ms() ) ),
  rtt( Range( input_config.rtt() ) ),
  mean_on_duration( Range( input_config.mean_on_duration() ) ),
  mean_off_duration( Range( input_config.mean_off_duration() ) ),
  num_senders( Range( input_config.num_senders() ) ),
  buffer_size( Range( input_config.buffer_size() ) ),
  simulation_ticks( input_config.simulation_ticks() ),
  stochastic_loss_rate( Range( input_config.stochastic_loss_rate() ) )
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
  ret.set_simulation_ticks( simulation_ticks );
  ret.mutable_stochastic_loss_rate()->CopyFrom( pair_to_range( stochastic_loss_rate ) );
  return ret;
}
