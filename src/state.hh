#ifndef STATE_HH
#define STATE_HH

#include <boost/functional/hash.hpp>

class State {
private:
  std::vector< double > _state_values;
  
public:
  struct StateHash {
    size_t operator()( const State & state ) const {
      size_t seed = 0;
      for ( unsigned int i = 0; i < state.values().size(); i++ ) {
        boost::hash_combine( seed, state.values().at( i ));
      }
      return seed;
    }
  };

  State( const std::vector< double > values )
    : _state_values( values )
  {}

  State()
    : _state_values()
  {}

  const std::vector< double > values ( void ) const { return _state_values; }

  bool operator==( const State & other ) const 
  { 
    return _state_values == other.values();
  }
};

#endif
