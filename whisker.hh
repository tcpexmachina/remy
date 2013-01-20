#ifndef WHISKER_HH
#define WHISKER_HH

#include <string>
#include <vector>

#include "memoryrange.hh"

class Whisker {
private:
  unsigned int _generation;
  unsigned int _window;
  double _rate;

  mutable unsigned int _count;

  MemoryRange _domain;

public:
  Whisker( const Whisker & other );
  Whisker( const unsigned int s_window, const double s_rate, const MemoryRange & s_domain );
  void use( void ) const { _count++; }

  const unsigned int & generation( void ) const { return _generation; }
  const unsigned int & window( void ) const { return _window; }
  const double & rate( void ) const { return _rate; }
  const unsigned int & count( void ) const { return _count; }
  const MemoryRange & domain( void ) const { return _domain; }

  bool operator==( const Whisker & other ) const;

  std::vector< Whisker > next_generation( void ) const;

  void reset_count( void ) { _count = 0; }
  void promote( const unsigned int generation );

  std::string str( void ) const;

  std::vector< Whisker > bisect( void ) const;
};

class Whiskers {
private:
  MemoryRange _domain;

  std::vector< Whiskers > _children;
  std::vector< Whisker > _leaf;

  const Whisker * whisker( const Memory & _memory ) const;

public:
  Whiskers();

  Whiskers( const Whisker & whisker, const bool bisect );

  const Whisker & use_whisker( const Memory & _memory, const bool track ) const;

  bool replace( const Whisker & w );
  bool replace( const Whisker & src, const Whiskers & dst );
  const Whisker * most_used( const unsigned int max_generation ) const;

  void reset_counts( void );
  void promote( const unsigned int generation );

  std::string str( void ) const;

  unsigned int num_children( void ) const;
};

#endif
