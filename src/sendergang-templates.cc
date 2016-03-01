#include "sendergang.hh"
#include "rat.hh"
#include "fish.hh"

template <>
SenderDataPoint SwitchedSender<Fish>::statistics_for_log( void ) const
{
  return SenderDataPoint( FISH, sender.memory(),
      utility.average_throughput_normalized_to_equal_share(),
      utility.average_delay(), utility.tick_share_sending(),
      utility.packets_received(), utility.total_delay(), 0, 0, sender.lambda(),
      sending );
}

template <>
SenderDataPoint SwitchedSender<Rat>::statistics_for_log( void ) const
{
  return SenderDataPoint( RAT, sender.memory(),
      utility.average_throughput_normalized_to_equal_share(),
      utility.average_delay(), utility.tick_share_sending(),
      utility.packets_received(), utility.total_delay(), sender.window_size(),
      sender.intersend_time(), 0, sending );
}

