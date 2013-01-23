#ifndef MEMORY_HH
#define MEMORY_HH

#include <vector>
#include <string>

#include "packet.hh"

class Memory {
public:
  typedef unsigned int DataType;

private:
  DataType _last_delay;

public:
  Memory( const std::vector< DataType > & s_data )
    : _last_delay( s_data.at( 0 ) )
  {}

  Memory()
    : _last_delay( 0 )
  {}

  void reset( void ) { _last_delay = 0; }

  static const unsigned int datasize = 1;
  const DataType & field( unsigned int ) const { return _last_delay; }
  DataType & mutable_field( unsigned int ) { return _last_delay; }

  void packet_sent( const Packet & packet __attribute((unused)) ) {}
  void packets_received( const std::vector< Packet > & packets );
  void advance_to( const unsigned int tickno __attribute((unused)) ) {}
  bool operator==( const Memory & other ) const;

  std::string str( void ) const;

  bool operator>=( const Memory & other ) const { return _last_delay >= other._last_delay; }
  bool operator<( const Memory & other ) const { return _last_delay < other._last_delay; }

};

extern const Memory & MAX_MEMORY( void );

#endif
