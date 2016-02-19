#ifndef MEMORYRANGE_HH
#define MEMORYRANGE_HH

#include <boost/accumulators/accumulators.hpp>
#include <boost/accumulators/statistics/stats.hpp>
#include <boost/accumulators/statistics/median.hpp>
#include <vector>
#include <string>

#include "memory.hh"
#include "dna.pb.h"

class MemoryRange {
public:
  enum Axis {
    SEND_EWMA = 0,
    REC_EWMA = 1,
    RTT_RATIO = 2,
    SLOW_REC_EWMA = 3,
    QUEUEING_DELAY = 4
  };

private:
  Memory _lower, _upper;  

  std::vector< Axis > _active_axis;

  mutable std::vector< boost::accumulators::accumulator_set< Memory::DataType,
							     boost::accumulators::stats<
							       boost::accumulators::tag::median > > > _acc;
  mutable unsigned int _count;

public:
  MemoryRange( const Memory & s_lower, const Memory & s_upper, 
    std::vector< Axis > s_active = { MemoryRange::SEND_EWMA, MemoryRange::REC_EWMA, MemoryRange::RTT_RATIO, MemoryRange::SLOW_REC_EWMA } )
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
