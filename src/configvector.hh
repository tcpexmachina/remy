#ifndef CONFIG_VECTOR
#define CONFIG_VECTOR
#include "dna.pb.h"
#include "network.hh"
#include <vector>
// Represents a vector of network points to train on
class ConfigVector {
public:
  std::vector< NetConfig > configs = std::vector< NetConfig >();
  ConfigVector ( void )
      : configs( std::vector< NetConfig >() )
  {} 

  void ReadVector(RemyBuffers::ConfigVector vector);
  RemyBuffers::ConfigVector DNA ( void );
};

#endif
