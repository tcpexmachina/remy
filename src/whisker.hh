#ifndef WHISKER_HH
#define WHISKER_HH

#include <string>
#include <vector>

#include "memoryrange.hh"
#include "dna.pb.h"

class Whisker {
private:
  unsigned int _generation;

  int _window_increment;
  double _window_multiple;
  double _intersend;

  MemoryRange _domain;

public:
  Whisker( const Whisker & other );
  Whisker( const unsigned int s_window_increment, const double s_window_multiple, const double s_intersend, const MemoryRange & s_domain );

  Whisker( const MemoryRange & s_domain ) : Whisker( get_optimizer().window_increment.default_value,
						     get_optimizer().window_multiple.default_value,
						     get_optimizer().intersend.default_value, s_domain ) {}

  void use( void ) const { _domain.use(); }
  void reset_count( void ) const { _domain.reset_count(); }
  unsigned int count( void ) const { return _domain.count(); }

  const unsigned int & generation( void ) const { return _generation; }
  unsigned int window( const unsigned int previous_window ) const { return std::min( std::max( 0, int( previous_window * _window_multiple + _window_increment ) ), 1000000 ); }
  const double & intersend( void ) const { return _intersend; }
  const MemoryRange & domain( void ) const { return _domain; }

  std::vector< Whisker > next_generation( void ) const;

  void promote( const unsigned int generation );

  std::string str( const unsigned int total=1 ) const;

  std::vector< Whisker > bisect( void ) const;

  void demote( const unsigned int generation ) { _generation = generation; }

  RemyBuffers::Whisker DNA( void ) const;
  Whisker( const RemyBuffers::Whisker & dna );

  void round( void );

  bool operator==( const Whisker & other ) const { return (_window_increment == other._window_increment) && (_window_multiple == other._window_multiple) && (_intersend == other._intersend) && (_domain == other._domain); }

  friend size_t hash_value( const Whisker & whisker );

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

    std::vector< T > alternatives( const T & value ) const;
    bool eligible_value( const T & value ) const;

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

  struct OptimizationSettings
  {
    OptimizationSetting< unsigned int > window_increment;
    OptimizationSetting< double > window_multiple;
    OptimizationSetting< double > intersend;

    RemyBuffers::OptimizationSettings DNA( void ) const
    {
      RemyBuffers::OptimizationSettings ret;

      ret.mutable_window_increment()->CopyFrom( window_increment.DNA() );
      ret.mutable_window_multiple()->CopyFrom( window_multiple.DNA() );
      ret.mutable_intersend()->CopyFrom( intersend.DNA() );

      return ret;
    }
  };

  static const OptimizationSettings & get_optimizer( void ) {
    static OptimizationSettings default_settings {
      { 0,    256, 1,    32,  4, 1 }, /* window increment */
      { 0,    1,   0.01, 0.5, 4, 1 }, /* window multiple */
      { 0.25, 3,   0.05, 1,   4, 3 } /* intersend */
    };
    return default_settings;
  }
};

#endif
