#ifndef WHISKER_HH
#define WHISKER_HH

#include "memoryrange.hh"

class Whisker {
private:
  unsigned int _generation;
  unsigned int _window;
  mutable unsigned int _count;

  MemoryRange _domain;

public:
  Whisker( const unsigned int s_window, const MemoryRange & s_domain );
  void use( void ) const { _count++; }

  const unsigned int & generation( void ) const { return _generation; }
  const unsigned int & window( void ) const { return _window; }
  const unsigned int & count( void ) const { return _count; }
  const MemoryRange & domain( void ) const { return _domain; }

  bool operator==( const Whisker & other ) const;

  std::vector< Whisker > next_generation( void ) const;

  void reset_count( void ) { _count = 0; }
  void promote( const unsigned int generation );
};

class Whiskers {
private:
  std::vector< Whiskers > _children;
  std::vector< Whisker > _leaf;

public:
  Whiskers();
  const Whisker & whisker( const Memory & _memory ) const;
  const Whisker & use_whisker( const Memory & _memory );

  bool replace( const Whisker & w );
  const Whisker * most_used( const unsigned int max_generation ) const;

  bool contains( const Memory & _memory ) const;

  void reset_counts( void );
  void promote( const unsigned int generation );
};

#endif
