#ifndef WHISKER_HH
#define WHISKER_HH

#include <string>
#include <vector>

#include "memoryrange.hh"
#include "dna.pb.h"

class Whisker {
private:
  unsigned int _generation;

  double _intersend_increment;
  double _intersend_multiple;

  MemoryRange _domain;

public:
  Whisker( const Whisker & other );
  Whisker( const double s_intersend_increment, const double s_intersend_multiple, const MemoryRange & s_domain );

  Whisker( const MemoryRange & s_domain ) : Whisker( get_optimizer().intersend_increment.default_value,
						     get_optimizer().intersend_multiple.default_value, s_domain ) {}

  void use( void ) const { _domain.use(); }
  void reset_count( void ) const { _domain.reset_count(); }
  unsigned int count( void ) const { return _domain.count(); }

  const unsigned int & generation( void ) const { return _generation; }

  //double intersend( const double previous_intersend ) const { return std::min<double>( std::max<double>( 0.01, previous_intersend * _intersend_multiple + _intersend_increment ), 1000000 ); }
  double intersend( const double previous_intersend __attribute((unused)) ) const { return _intersend_increment; }

  const MemoryRange & domain( void ) const { return _domain; }
  std::vector< Whisker > next_generation( void ) const;

  void promote( const unsigned int generation );

  std::string str( const unsigned int total=1 ) const;

  std::vector< Whisker > bisect( void ) const;

  void demote( const unsigned int generation ) { _generation = generation; }

  RemyBuffers::Whisker DNA( void ) const;
  Whisker( const RemyBuffers::Whisker & dna );

  void round( void );

  bool operator==( const Whisker & other ) const { return (_intersend_increment == other._intersend_increment) && (_intersend_multiple == other._intersend_multiple) && (_domain == other._domain); }

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
    OptimizationSetting< double > intersend_increment;
    OptimizationSetting< double > intersend_multiple;

    RemyBuffers::OptimizationSettings DNA( void ) const
    {
      RemyBuffers::OptimizationSettings ret;

      ret.mutable_intersend_increment()->CopyFrom( intersend_increment.DNA() );
      ret.mutable_intersend_multiple()->CopyFrom( intersend_multiple.DNA() );

      return ret;
    }
  };

  static const OptimizationSettings & get_optimizer( void ) {
    static OptimizationSettings default_settings {
      /* min, max, min change, max change, multiplier, default */
      { -10,    10, 0.05, 1,   8, 1 }, /* intersend increment */
      { 1,      4,  0.01, 0.5, 8, 1 }, /* intersend multiple */
    };
    return default_settings;
  }
};

#endif
