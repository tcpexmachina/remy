#ifndef MEMORY_HH
#define MEMORY_HH

#include <vector>
#include <string>

#include "packet.hh"

class Memory {
public:
  typedef unsigned int DataType;

private:
  std::vector< DataType > _data;

  enum fields { LAST_DELAY };

public:
  Memory( const std::vector< DataType > & s_data )
    : _data( s_data )
  {}

  Memory()
    : _data( datasize(), 0 )
  {}

  static constexpr unsigned int datasize( void ) { return 1; }
  const std::vector< DataType > & data( void ) const { return _data; }

  void packet_sent( const Packet & packet __attribute((unused)) ) {}
  void packets_received( const std::vector< Packet > & packets );
  void advance_to( const unsigned int tickno __attribute((unused)) ) {}
  bool operator==( const Memory & other ) const;

  std::string str( void ) const;
};

extern const Memory & MAX_MEMORY( void );

#endif
