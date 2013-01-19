#ifndef MEMORYRANGE_HH
#define MEMORYRANGE_HH

#include <boost/accumulators/accumulators.hpp>
#include <boost/accumulators/statistics/stats.hpp>
#include <boost/accumulators/statistics/median.hpp>
#include <vector>
#include <string>

#include "memory.hh"

class MemoryRange {
private:
  Memory _lower, _upper;  

  mutable std::vector< boost::accumulators::accumulator_set< Memory::DataType,
							     boost::accumulators::stats<
							       boost::accumulators::tag::median > > > _acc;

public:
  MemoryRange( const Memory & s_lower, const Memory & s_upper )
    : _lower( s_lower ), _upper( s_upper ), _acc( Memory::datasize() )
  {}

  std::vector< MemoryRange > bisect( void ) const;
  Memory range_median( void ) const;
  Memory query_median( void ) const;

  bool contains( const Memory & query ) const;
  void track( const Memory & query ) const;

  bool operator==( const MemoryRange & other ) const;

  std::string str( void ) const;
};

#endif
