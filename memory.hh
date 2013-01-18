#ifndef MEMORY_HH
#define MEMORY_HH

#include <vector>
#include <string>

#include "packet.hh"

class Memory {
private:
  unsigned int _last_delay;
  unsigned int _last_window;

public:
  void new_window( const unsigned int s_window ) { _last_window = s_window; }

  void packet_sent( const Packet & packet __attribute((unused)) ) {}
  void packets_received( const std::vector< Packet > & packets );
  void advance_to( const unsigned int tickno __attribute((unused)) ) {}
  bool operator==( const Memory & other ) const;

  static std::vector< Memory > all_memories( void );
  unsigned int bin( const unsigned int max_val ) const;
  std::string str( void ) const;
};

#endif
