#include "configrange.hh"

RemyBuffers::ConfigRange ConfigRange::DNA( void ) const

{
  RemyBuffers::ConfigRange ret;
  ret.set_bw_low( link_packets_per_ms.first );
  ret.set_bw_high( link_packets_per_ms.second );
  ret.set_rtt_low( rtt_ms.first );
  ret.set_rtt_high( rtt_ms.second );
  ret.set_min_senders( 1 );
  ret.set_max_senders( max_senders );
  ret.set_mean_off_duration( mean_off_duration );
  ret.set_mean_on_duration( mean_on_duration );
  return ret;
}
