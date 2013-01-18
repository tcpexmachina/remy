#ifndef WHISKER_HH
#define WHISKER_HH

#include "memory.hh"

class Whisker {
private:
  unsigned int _generation;
  unsigned int _window;
  mutable unsigned int _count;

  Memory _representative_value;

public:
  Whisker( const Memory & s_representative_value );
  void use( void ) const { _count++; }

  const Memory & representative_value( void ) const { return _representative_value; }
  const unsigned int & generation( void ) const { return _generation; }
  const unsigned int & window( void ) const { return _window; }
  const unsigned int & count( void ) const { return _count; }
  bool operator==( const Whisker & other ) const;
  std::vector< Whisker > next_generation( void ) const;
  std::string summary( void ) const;

  void reset_count( void ) { _count = 0; }
  void promote( const unsigned int generation );
};

class Whiskers {
private:
  std::vector< Whisker > _whiskers;

public:
  Whiskers();
  const Whisker & whisker( const Memory & _memory ) const;
  const Whisker & use_whisker( const Memory & _memory );
  const std::vector< Whisker > & whiskers( void ) const { return _whiskers; }
  void replace( const Whisker & w );
  const Whisker * most_used( const unsigned int max_generation ) const;

  void reset_counts( void );
  void promote( const unsigned int generation );
};

#endif
