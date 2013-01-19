#ifndef MEMORYRANGE_HH
#define MEMORYRANGE_HH

#include <boost/accumulators/accumulators.hpp>
#include <boost/accumulators/statistics/stats.hpp>
#include <boost/accumulators/statistics/median.hpp>
#include <vector>

#include "memory.hh"

class MemoryRange {
private:
  Memory _lower, _upper;  

  std::vector< boost::accumulators::accumulator_set< Memory::DataType,
						     boost::accumulators::stats<
						       boost::accumulators::tag::median > > > _acc;

  Memory query_median( void ) const;

public:
  MemoryRange( const Memory & s_lower, const Memory & s_upper )
    : _lower( s_lower ), _upper( s_upper ), _acc( Memory::datasize() )
  {}

  std::vector< MemoryRange > bisect( void ) const;
  Memory range_median( void ) const;
  bool contains( const Memory & query );
};


#endif
