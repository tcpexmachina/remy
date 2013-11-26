#ifndef CONFIG_RANGE_HH
#define CONFIG_RANGE_HH

#include "dna.pb.h"

class ConfigRange
{
public:
  std::pair< double, double > link_packets_per_ms;
  std::pair< double, double > rtt_ms;
  unsigned int max_senders;
  double mean_on_duration, mean_off_duration;
  bool lo_only;

  ConfigRange( void ) : link_packets_per_ms(), rtt_ms(), max_senders( 1 ),
      		  mean_on_duration(), mean_off_duration(), lo_only( false ) {}

  RemyBuffers::ConfigRange DNA( void ) const;
};

#endif  // CONFIG_RANGE_HH
