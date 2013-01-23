#ifndef WHISKER_HH
#define WHISKER_HH

#include <string>
#include <vector>

#include "memoryrange.hh"

class Whisker {
private:
  unsigned int _generation;
  unsigned int _window;
  double _intersend;

  MemoryRange _domain;

public:
  Whisker( const Whisker & other );
  Whisker( const unsigned int s_window, const double s_intersend, const MemoryRange & s_domain );

  void use( void ) const { _domain.use(); }
  void reset_count( void ) const { _domain.reset_count(); }
  unsigned int count( void ) const { return _domain.count(); }

  const unsigned int & generation( void ) const { return _generation; }
  const unsigned int & window( void ) const { return _window; }
  const double & intersend( void ) const { return _intersend; }
  const MemoryRange & domain( void ) const { return _domain; }

  std::vector< Whisker > next_generation( void ) const;

  void promote( const unsigned int generation );

  std::string str( void ) const;

  std::vector< Whisker > bisect( void ) const;

  void demote( const unsigned int generation ) { _generation = generation; }
};

static const unsigned int MAX_WINDOW = 256;
static constexpr double MAX_INTERSEND = 10.0;

static const unsigned int MAX_WINDOW_INCR = 64;
static constexpr double MAX_INTERSEND_INCR = 0.5;

static const unsigned int DEFAULT_WINDOW = 1;
static constexpr double MIN_INTERSEND = 0.05;

static constexpr double INTERSEND_INCR = 0.05;
#endif
