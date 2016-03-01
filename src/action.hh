#ifndef ACTION_HH
#define ACTION_HH

#include <string>
#include <vector>

#include "memoryrange.hh"
#include "dna.pb.h"

using namespace std;

class Action {
protected:
  unsigned int _generation;
  MemoryRange _domain;

public:
  Action( const Action & other ) : _generation( other._generation ), _domain( other._domain ) {};
  Action( const MemoryRange & s_domain ) : _generation( 0 ), _domain( s_domain ) {};
  virtual ~Action() {};

  virtual std::string str( const unsigned int total=1 ) const = 0;
  virtual void round( void ) = 0;
  
  template <typename T>
  vector< T > bisect( void ) const {
    vector< T > ret;
    for ( auto &x : _domain.bisect() ) {
      T new_action( (T &)(* this) );
      new_action._domain = x;
      ret.push_back( new_action );
    }
    return ret;
  }

  void use( void ) const { _domain.use(); }
  void reset_count( void ) const { _domain.reset_count(); }
  unsigned int count( void ) const { return _domain.count(); }

  const unsigned int & generation( void ) const { return _generation; }
  const MemoryRange & domain( void ) const { return _domain; }

  void promote( const unsigned int generation ) { _generation = max( _generation, generation ); }
  void demote( const unsigned int generation ) { _generation = generation; }

  template < typename T >
  struct OptimizationSetting
  {
    T min_value; /* the smallest the value can be */
    T max_value; /* the biggest */

    T min_change; /* the smallest change to the value in an optimization exploration step */
    T max_change; /* the biggest change */

    T multiplier; /* we will explore multiples of the min_change until we hit the max_change */
    /* the multiplier defines which multiple (e.g. 1, 2, 4, 8... or 1, 3, 9, 27... ) */

    T default_value;

    bool eligible_value( const T & value ) const { return value >= min_value and value <= max_value; }

    std::vector< T > alternatives( const T & value, bool active ) const 
    {
      if ( !eligible_value( value ) ) {
        printf("Ineligible value: %s is not between %s and %s\n", to_string( value ).c_str(), to_string( min_value ).c_str(), to_string( max_value ).c_str());
        assert(false);
      }

      vector< T > ret( 1, value );

      /* If this axis isn't active, return only the current value. */
      if (!active) return ret;

      for ( T proposed_change = min_change;
            proposed_change <= max_change;
            proposed_change *= multiplier ) {
        /* explore positive change */
        const T proposed_value_up = value + proposed_change;
        const T proposed_value_down = value - proposed_change;

        if ( eligible_value( proposed_value_up ) ) {
          ret.push_back( proposed_value_up );
        }

        if ( eligible_value( proposed_value_down ) ) {
          ret.push_back( proposed_value_down );
        }
      }

      return ret;
    }
    
    RemyBuffers::OptimizationSetting DNA( void ) const
    {
      RemyBuffers::OptimizationSetting ret;

      ret.set_min_value( min_value );
      ret.set_max_value( max_value );
      ret.set_min_change( min_change );
      ret.set_max_change( max_change );
      ret.set_multiplier( multiplier );
      ret.set_default_value( default_value );

      return ret;
    }
  };
};

#endif