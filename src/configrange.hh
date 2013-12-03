#ifndef CONFIG_RANGE_HH
#define CONFIG_RANGE_HH

#include "dna.pb.h"

class ConfigRange
{
public:
  std::pair< double, double > link_packets_per_ms { 1, 2 };
  std::pair< double, double > rtt_ms { 100, 200 };
  unsigned int min_senders { 1 };
  unsigned int max_senders { 16 };
  double mean_on_duration { 1000 };
  double mean_off_duration { 1000 };
  bool lo_only { false };

  RemyBuffers::ConfigRange DNA( void ) const;
};

#endif  // CONFIG_RANGE_HH
