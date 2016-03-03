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
  DataType _rtt_diff;
  DataType _queueing_delay;

  double _last_tick_sent;
  double _last_tick_received;
  double _min_rtt;

public:
  Memory( const std::vector< DataType > & s_data )
    : _rec_send_ewma( s_data.at( 0 ) ),
      _rec_rec_ewma( s_data.at( 1 ) ),
      _rtt_ratio( s_data.at( 2 ) ),
      _slow_rec_rec_ewma( s_data.at( 3 ) ),
      _rtt_diff( s_data.at(4) ),
      _queueing_delay( s_data.at(5) ),
      _last_tick_sent( 0 ),
      _last_tick_received( 0 ),
      _min_rtt( 0 )
  {}

  Memory()
    : _rec_send_ewma( 0 ),
      _rec_rec_ewma( 0 ),
      _rtt_ratio( 0.0 ),
      _slow_rec_rec_ewma( 0 ),
      _rtt_diff( 0 ),
      _queueing_delay( 0 ),
      _last_tick_sent( 0 ),
      _last_tick_received( 0 ),
      _min_rtt( 0 )
  {}

  void reset( void ) { _rec_send_ewma = _rec_rec_ewma = _rtt_ratio = _slow_rec_rec_ewma = _rtt_diff = _queueing_delay = _last_tick_sent = _last_tick_received = _min_rtt = 0; }

  static const unsigned int datasize = 6;

  const DataType & field( unsigned int num ) const { return num == 0 ? _rec_send_ewma : num == 1 ? _rec_rec_ewma : num == 2 ? _rtt_ratio : num == 3 ? _slow_rec_rec_ewma : num == 4 ? _rtt_diff : _queueing_delay ; }
  DataType & mutable_field( unsigned int num )     { return num == 0 ? _rec_send_ewma : num == 1 ? _rec_rec_ewma : num == 2 ? _rtt_ratio : num == 3 ? _slow_rec_rec_ewma : num == 4 ? _rtt_diff : _queueing_delay ; }

  void packet_sent( const Packet & packet __attribute((unused)) ) {}
  void packets_received( const std::vector< Packet > & packets, const unsigned int flow_id, const int largest_ack );
  void advance_to( const unsigned int tickno __attribute((unused)) ) {}

  std::string str( void ) const;
  std::string str( unsigned int num ) const;

  bool operator>=( const Memory & other ) const { 
    for (unsigned int i = 0; i < datasize; i ++) { if ( field(i) < other.field(i) ) return false; }
    return true;
  }
  bool operator<( const Memory & other ) const { 
    for (unsigned int i = 0; i < datasize; i ++) { if ( field(i) >= other.field(i) ) return false; }
    return true;
  }
  bool operator==( const Memory & other ) const { 
    for (unsigned int i = 0; i < datasize; i ++) { if ( field(i) != other.field(i) ) return false; }
    return true;
  }

  RemyBuffers::Memory DNA( void ) const;
  Memory( const bool is_lower_limit, const RemyBuffers::Memory & dna );

  friend size_t hash_value( const Memory & mem );
};

extern const Memory & MAX_MEMORY( void );

#endif
