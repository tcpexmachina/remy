#ifndef WHISKER_HH
#define WHISKER_HH

#include <string>
#include <vector>

#include "memoryrange.hh"
#include "action.hh"
#include "dna.pb.h"

class Whisker : public Action {
private:
  int _window_increment;
  double _window_multiple;
  double _intersend;

public:
  Whisker( const Whisker & other );
  Whisker( const unsigned int s_window_increment, const double s_window_multiple, const double s_intersend, const MemoryRange & s_domain );

  Whisker& operator=( const Whisker & ) = default;
  
  Whisker( const MemoryRange & s_domain ) : Whisker( get_optimizer().window_increment.default_value,
                 get_optimizer().window_multiple.default_value,
                 get_optimizer().intersend.default_value, s_domain ) {};
  virtual ~Whisker() {};

  unsigned int window( const unsigned int previous_window ) const { return std::min( std::max( 0, int( previous_window * _window_multiple + _window_increment ) ), 1000000 ); }
  const double & intersend( void ) const { return _intersend; }
  
  std::vector< Whisker > next_generation( bool optimize_window_increment, bool optimize_window_multiple, bool optimize_intersend ) const;

  std::string str( const unsigned int total=1 ) const;
  
  RemyBuffers::Whisker DNA( void ) const;
  Whisker( const RemyBuffers::Whisker & dna );
  
  void round( void );

  bool operator==( const Whisker & other ) const { return (_window_increment == other._window_increment) && (_window_multiple == other._window_multiple) && (_intersend == other._intersend) && (_domain == other._domain); }

  friend size_t hash_value( const Whisker & Whisker );

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
