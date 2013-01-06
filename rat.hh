#ifndef RAT_HH
#define RAT_HH

#include <vector>
#include <string>

#include "packet.hh"

class Rat
{
private:
  class Memory {
  private:
    double _last_delay;

    static unsigned int binsize( void ) { return 20; }

  public:
    void packet_sent( const Packet & packet __attribute((unused)) ) {}
    void packets_received( const std::vector< Packet > & packets );
    void advance_to( const unsigned int tickno __attribute((unused)) ) {}
    bool operator==( const Memory & other ) const;

    static std::vector< Memory > all_memories( void );
    unsigned int bin( const unsigned int max_val ) const;
    std::string str( void ) const;
  };

public:
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
  };

private:
  Whiskers _whiskers;
  Memory _memory;

  unsigned int window( const unsigned int tickno );

  unsigned int _packets_sent, _packets_received;

public:
  Rat( const Whiskers & s_whiskers );

  void packets_received( const std::vector< Packet > & packets );
  void dormant_tick( const unsigned int tickno ); /* do nothing */

  template <class NextHop>
  void send( const unsigned int id, NextHop & next, const unsigned int tickno );

  const Whiskers & whiskers( void ) const { return _whiskers; }
};

#endif
