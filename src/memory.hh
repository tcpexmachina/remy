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
  DataType _rec_send_ewma;
  DataType _rec_rec_ewma;
  DataType _rtt_ratio;
  DataType _slow_rec_rec_ewma;

  double _last_tick_sent = 0;
  double _last_tick_received = 0;
  double _min_rtt = 0;

  int _packets_sent = 0, _packets_received = 0;

public:
  Memory( const std::vector< DataType > & s_data )
    : _rec_send_ewma( s_data.at( 0 ) ),
      _rec_rec_ewma( s_data.at( 1 ) ),
      _rtt_ratio( s_data.at( 2 ) ),
      _slow_rec_rec_ewma( s_data.at( 3 ) )
  {}

  Memory()
    : _rec_send_ewma( 0 ),
      _rec_rec_ewma( 0 ),
      _rtt_ratio( 0.0 ),
      _slow_rec_rec_ewma( 0 )
  {}

  void reset( void ) { *this = Memory(); }

  static const unsigned int datasize = 4;

  const DataType & field( unsigned int num ) const { return num == 0 ? _rec_send_ewma : num == 1 ? _rec_rec_ewma : num == 2 ? _rtt_ratio : _slow_rec_rec_ewma ; }
  DataType & mutable_field( unsigned int num )     { return num == 0 ? _rec_send_ewma : num == 1 ? _rec_rec_ewma : num == 2 ? _rtt_ratio : _slow_rec_rec_ewma ; }

  const double & last_tick_received( void ) const { return _last_tick_received; }

  void packet_sent( const Packet & packet __attribute((unused)) ) { _packets_sent++; }
  void packets_received( const std::vector< Packet > & packets, const unsigned int flow_id );
  void advance_to( const unsigned int tickno __attribute((unused)) ) {}

  const int & num_packets_sent( void ) const { return _packets_sent; }
  const int & num_packets_received( void ) const { return _packets_received; }
  int num_outstanding_packets( void ) const { return _packets_sent - _packets_received; }

  std::string str( void ) const;

  bool operator>=( const Memory & other ) const { return (_rec_send_ewma >= other._rec_send_ewma) && (_rec_rec_ewma >= other._rec_rec_ewma) && (_rtt_ratio >= other._rtt_ratio) && (_slow_rec_rec_ewma >= other._slow_rec_rec_ewma); }
  bool operator<( const Memory & other ) const { return (_rec_send_ewma < other._rec_send_ewma) && (_rec_rec_ewma < other._rec_rec_ewma) && (_rtt_ratio < other._rtt_ratio) && (_slow_rec_rec_ewma < other._slow_rec_rec_ewma); }
  bool operator==( const Memory & other ) const { return (_rec_send_ewma == other._rec_send_ewma) && (_rec_rec_ewma == _rec_rec_ewma) && (_rtt_ratio == other._rtt_ratio) && (_slow_rec_rec_ewma == other._slow_rec_rec_ewma); }

  RemyBuffers::Memory DNA( void ) const;
  Memory( const bool is_lower_limit, const RemyBuffers::Memory & dna );

  friend size_t hash_value( const Memory & mem );
};

extern const Memory & MAX_MEMORY( void );

#endif
