#ifndef STATE_HH
#define STATE_HH

#include <cmath>
#include <boost/functional/hash.hpp>

class State {
private:
  std::vector< double > _state_values;
  double _timestamp;

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

  State( const std::vector< double > values, const double time )
    : _state_values( values ),
      _timestamp( time )
  {}

  State()
    : _state_values(),
      _timestamp( 0.0 )
  {}

  const std::vector< double > values( void ) const { return _state_values; }
  double timestamp( void ) const { return _timestamp; }

  bool operator==( const State & other ) const 
  { 
    auto compare = other.values();
    if ( _state_values.size() != compare.size() ) return false;

    const double eps = 0; /* tolerance in the fourth decimal place */
    for ( unsigned int i = 0; i < _state_values.size(); i++ ) {
      if ( std::abs(_state_values.at( i ) - compare.at( i )) > eps ) {
        //printf("discrepancy %f %f\n", _state_values.at( i ), compare.at( i ));
        return false;
      }
    }
    return true;
  }

  void print_state( void ) const 
  {
    cout << "Time: " << _timestamp << "\t State: ";
    for( auto val : _state_values ) {
      cout << "\t" << val << " ";
    }
    cout << endl;
  }
};

#endif
