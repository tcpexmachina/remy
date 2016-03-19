#ifndef MEMORYRANGE_HH
#define MEMORYRANGE_HH

#include <boost/accumulators/accumulators.hpp>
#include <boost/accumulators/statistics/stats.hpp>
#include <boost/accumulators/statistics/median.hpp>
#include <vector>
#include <string>

#include "memory.hh"
#include "dna.pb.h"

typedef RemyBuffers::MemoryRange::Axis Axis;

class MemoryRange {
private:
  Memory _lower, _upper;  

  /* _active_axis specifies the group of signals in Memory used by the sender. 
     For example, Fish only uses signal rtt_diff, while Rat uses four signals: 
     rec_send_ewma, rec_rec_ewma, rtt_ratio and slow_rec_rec_rewma. */
  std::vector< Axis > _active_axis;

  mutable std::vector< boost::accumulators::accumulator_set< Memory::DataType,
							     boost::accumulators::stats<
							     boost::accumulators::tag::median > > > _acc;
  mutable unsigned int _count;

public:
  MemoryRange( const Memory & s_lower, const Memory & s_upper, 
    std::vector< Axis > s_active = { RemyBuffers::MemoryRange::SEND_EWMA, RemyBuffers::MemoryRange::REC_EWMA, RemyBuffers::MemoryRange::RTT_RATIO, RemyBuffers::MemoryRange::SLOW_REC_EWMA } )
    : _lower( s_lower ), _upper( s_upper ), _active_axis( s_active ), _acc( Memory::datasize ), _count( 0 )
  {}

  std::vector< MemoryRange > bisect( void ) const;
  Memory range_median( void ) const;

  bool contains( const Memory & query ) const;

  void use( void ) const { _count++; }
  unsigned int count( void ) const { return _count; }
  void reset_count( void ) const { _count = 0; }

  void track( const Memory & query ) const;

  bool operator==( const MemoryRange & other ) const;

  std::string str( void ) const;

  RemyBuffers::MemoryRange DNA( void ) const;
  MemoryRange( const RemyBuffers::MemoryRange & dna );

  friend size_t hash_value( const MemoryRange & mr );
};

#endif
