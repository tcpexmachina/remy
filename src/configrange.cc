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

  return ret;
}
