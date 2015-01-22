#ifndef MEMORY_HH
#define MEMORY_HH

#include <vector>
#include <string>

#include "packet.hh"
#include "dna.pb.h"

class Memory {
public:
  typedef double DataType;

private:
  DataType _imputed_delay;

  double _last_tick_sent = 0;
  double _last_tick_received = 0;
  double _min_rtt = 0;

  double _rec_ewma = 10;

  int _packets_sent = 0, _packets_received = 0;

  void recalculate_signals( void );

public:
  Memory( const std::vector< DataType > & s_data )
    : _imputed_delay( s_data.at( 0 ) )
  {}

  Memory()
    : _imputed_delay( 0 )
  {}

  void reset( void ) { *this = Memory(); }

  static const unsigned int datasize = 1;
  static double precise_round( const double & value )
  {
    return (1.0/10000.0) * int( 10000 * value );
  }

  const DataType & field( unsigned int ) const { return _imputed_delay; }
  DataType & mutable_field( unsigned int )     { return _imputed_delay; }

  void packet_sent( const Packet & packet );
  void packets_received( const std::vector< Packet > & packets, const unsigned int flow_id );
  void advance_to( const unsigned int tickno __attribute((unused)) ) {}

  const double & imputed_delay( void ) { return _imputed_delay; }

  const int & packets_sent( void ) const { return _packets_sent; }
  const int & packets_received( void ) const { return _packets_received; }

  std::string str( void ) const;

  bool operator>=( const Memory & other ) const { return (_imputed_delay >= other._imputed_delay); }
  bool operator<( const Memory & other ) const { return (_imputed_delay < other._imputed_delay); }
  bool operator==( const Memory & other ) const { return (_imputed_delay == other._imputed_delay); }

  RemyBuffers::Memory DNA( void ) const;
  Memory( const bool is_lower_limit, const RemyBuffers::Memory & dna );

  friend size_t hash_value( const Memory & mem );
};

extern const Memory & MAX_MEMORY( void );

#endif
