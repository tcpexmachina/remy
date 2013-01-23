#ifndef MEMORY_HH
#define MEMORY_HH

#include <vector>
#include <string>

#include "packet.hh"

class Memory {
public:
  typedef double DataType;

private:
  DataType _rec_send_ewma;
  DataType _rec_rec_ewma;

  unsigned int _last_tick_sent;
  unsigned int _last_tick_received;

public:
  Memory( const std::vector< DataType > & s_data )
    : _rec_send_ewma( s_data.at( 0 ) ),
      _rec_rec_ewma( s_data.at( 1 ) ),
      _last_tick_sent( 0 ),
      _last_tick_received( 0 )
  {}

  Memory()
    : _rec_send_ewma( 0 ),
      _rec_rec_ewma( 0 ),
      _last_tick_sent( 0 ),
      _last_tick_received( 0 )
  {}

  void reset( void ) { _rec_send_ewma = _rec_rec_ewma = _last_tick_sent = _last_tick_received = 0; }

  static const unsigned int datasize = 2;

  const DataType & field( unsigned int num ) const { return num == 0 ? _rec_send_ewma : _rec_rec_ewma; }
  DataType & mutable_field( unsigned int num ) { return num == 0 ? _rec_send_ewma : _rec_rec_ewma; }

  void packet_sent( const Packet & packet __attribute((unused)) ) {}
  void packets_received( const std::vector< Packet > & packets );
  void advance_to( const unsigned int tickno __attribute((unused)) ) {}

  std::string str( void ) const;

  bool operator>=( const Memory & other ) const { return (_rec_send_ewma >= other._rec_send_ewma) && (_rec_rec_ewma >= other._rec_rec_ewma); }
  bool operator<( const Memory & other ) const { return (_rec_send_ewma < other._rec_send_ewma) && (_rec_rec_ewma < other._rec_rec_ewma); }
  bool operator==( const Memory & other ) const { return (_rec_send_ewma == other._rec_send_ewma) && (_rec_rec_ewma == _rec_rec_ewma); }
};

extern const Memory & MAX_MEMORY( void );

#endif
