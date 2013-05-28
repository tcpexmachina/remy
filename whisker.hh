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

  void use( void ) const { _domain.use(); }
  void reset_count( void ) const { _domain.reset_count(); }
  unsigned int count( void ) const { return _domain.count(); }

  const unsigned int & generation( void ) const { return _generation; }
  unsigned int window( const unsigned int previous_window ) const { return std::max( 0, int( previous_window * _window_multiple + _window_increment ) ); }
  const double & intersend( void ) const { return _intersend; }
  const MemoryRange & domain( void ) const { return _domain; }

  std::vector< Whisker > next_generation( void ) const;

  void promote( const unsigned int generation );

  std::string str( void ) const;

  std::vector< Whisker > bisect( void ) const;

  void demote( const unsigned int generation ) { _generation = generation; }

  RemyBuffers::Whisker DNA( void ) const;
  Whisker( const RemyBuffers::Whisker & dna );

  void round( void );

  bool operator==( const Whisker & other ) const { return (_window_increment == other._window_increment) && (_window_multiple == other._window_multiple) && (_intersend == other._intersend) && (_domain == other._domain); }

  friend size_t hash_value( const Whisker & whisker );
};

static const unsigned int MAX_WINDOW = 10;
static constexpr double MAX_INTERSEND = 80;
static constexpr double MAX_MULTIPLE = 2.0;

static const unsigned int MAX_WINDOW_INCR = 10;
static constexpr double MAX_INTERSEND_INCR = 80;
static constexpr double MAX_MULTIPLE_INCR = 0.5;

static const unsigned int DEFAULT_WINDOW = 1;
static const unsigned int DEFAULT_MULTIPLE = 1;

static constexpr double MIN_INTERSEND = 0.1;
static constexpr double MULTIPLE_INCR = 0.01;

static constexpr double INTERSEND_INCR = 0.1;
#endif
