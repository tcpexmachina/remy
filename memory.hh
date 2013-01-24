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

  unsigned int _last_tick_sent;
  unsigned int _last_tick_received;
  unsigned int _min_rtt;

public:
  Memory( const std::vector< DataType > & s_data )
    : _rec_send_ewma( s_data.at( 0 ) ),
      _rec_rec_ewma( s_data.at( 1 ) ),
      _rtt_ratio( s_data.at( 2 ) ),
      _last_tick_sent( 0 ),
      _last_tick_received( 0 ),
      _min_rtt( 0 )
  {}

  Memory()
    : _rec_send_ewma( 0 ),
      _rec_rec_ewma( 0 ),
      _rtt_ratio( 0.0 ),
      _last_tick_sent( 0 ),
      _last_tick_received( 0 ),
      _min_rtt( 0 )
  {}

  void reset( void ) { _rec_send_ewma = _rec_rec_ewma = _rtt_ratio = _last_tick_sent = _last_tick_received = _min_rtt = 0; }

  static const unsigned int datasize = 3;

  const DataType & field( unsigned int num ) const { return num == 0 ? _rec_send_ewma : num == 1 ? _rec_rec_ewma : _rtt_ratio; }
  DataType & mutable_field( unsigned int num ) { return  num == 0 ? _rec_send_ewma : num == 1 ? _rec_rec_ewma : _rtt_ratio; }

  void packet_sent( const Packet & packet __attribute((unused)) ) {}
  void packets_received( const std::vector< Packet > & packets );
  void advance_to( const unsigned int tickno __attribute((unused)) ) {}

  std::string str( void ) const;

  bool operator>=( const Memory & other ) const { return (_rec_send_ewma >= other._rec_send_ewma) && (_rec_rec_ewma >= other._rec_rec_ewma) && (_rtt_ratio >= other._rtt_ratio); }
  bool operator<( const Memory & other ) const { return (_rec_send_ewma < other._rec_send_ewma) && (_rec_rec_ewma < other._rec_rec_ewma) && (_rtt_ratio < other._rtt_ratio); }
  bool operator==( const Memory & other ) const { return (_rec_send_ewma == other._rec_send_ewma) && (_rec_rec_ewma == _rec_rec_ewma) && (_rtt_ratio == other._rtt_ratio); }

  RemyBuffers::Memory DNA( void ) const;
  Memory( const RemyBuffers::Memory & dna );
};

extern const Memory & MAX_MEMORY( void );

#endif
