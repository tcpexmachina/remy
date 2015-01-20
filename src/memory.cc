#include <boost/functional/hash.hpp>
#include <vector>
#include <cassert>

#include "memory.hh"

using namespace std;

static const double alpha = 1.0 / 8.0;

static const double slow_alpha = 1.0 / 256.0;

void Memory::recalculate_signals( void )
{
  _imputed_delay = _rec_ewma * (_packets_sent - _packets_received);

  /*
  fprintf( stderr, "rec_ewma=%f, outstanding=%d, imputed=%f\n",
	   _rec_ewma, _packets_sent - _packets_received, _imputed_delay );
  */
}

void Memory::packets_received( const vector< Packet > & packets, const unsigned int flow_id )
{
  _packets_received += packets.size();

  for ( const auto &x : packets ) {
    if ( x.flow_id != flow_id ) {
      continue;
    }

    /* update the state of the memory that is NOT the congestion signals */
    const double rtt = x.tick_received - x.tick_sent;
    if ( _last_tick_sent == 0 || _last_tick_received == 0 ) {
      _last_tick_sent = x.tick_sent;
      _last_tick_received = x.tick_received;
      _min_rtt = rtt;
    } else {
      _rec_ewma = (1 - alpha) * _rec_ewma + alpha * (x.tick_received - _last_tick_received);
      _last_tick_sent = x.tick_sent;
      _last_tick_received = x.tick_received;
      _min_rtt = min( _min_rtt, rtt );
    }

    /* now recalculate the signals themselves */
    recalculate_signals();
  }
}

void Memory::packet_sent( const Packet & packet __attribute((unused)) )
{
  _packets_sent++;

  recalculate_signals();
}

string Memory::str( void ) const
{
  char tmp[ 64 ];
  snprintf( tmp, 64, "imputed_delay=%f", _imputed_delay );
  return tmp;
}

const Memory & MAX_MEMORY( void )
{
  static const Memory max_memory( { 10000 } );
  return max_memory;
}

RemyBuffers::Memory Memory::DNA( void ) const
{
  RemyBuffers::Memory ret;
  ret.set_imputed_delay( _imputed_delay );
  return ret;
}

/* If fields are missing in the DNA, we want to wildcard the resulting rule to match anything */
#define get_val_or_default( protobuf, field, limit ) \
  ( (protobuf).has_ ## field() ? (protobuf).field() : (limit) ? 0 : 163840 )

Memory::Memory( const bool is_lower_limit, const RemyBuffers::Memory & dna )
  : _imputed_delay( get_val_or_default( dna, imputed_delay, is_lower_limit ) )
{
}

size_t hash_value( const Memory & mem )
{
  size_t seed = 0;
  boost::hash_combine( seed, mem._imputed_delay );

  return seed;
}
