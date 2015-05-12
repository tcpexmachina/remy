#ifndef CYCLE_FINDER_HH
#define CYCLE_FINDER_HH

#include "network.hh"

template <class SenderType1, class SenderType2>
class CycleFinder {

private:
  Network< SenderType1, SenderType2 > _network;
  bool _cycle_found = false;
  bool _exception = false;

  /* Just records the offset that this network was already set to when
   the cycle finder was constructed. */
  double _offset_value = 0;

  std::vector< std::pair< double, double > > _deltas = {};
  double _cycle_len = -1;
  double _convergence_time = -1;

  /* A network in the state in which the cycle starts */
  Network< SenderType1, SenderType2 > _cycle_start;

public:
  CycleFinder( const Network< SenderType1, SenderType2 > & network,
	       const double offset_value = 0 );

  void run_until_cycle_found( bool verbose = false );

  const std::vector< std::pair< double, double > > deltas( void ) const { return _deltas; };
  const double & cycle_len( void ) const { return _cycle_len; }
  const double & convergence_time ( void ) const { return _convergence_time; }
  const Network< SenderType1, SenderType2 > & cycle_start ( void ) const { return _cycle_start; }

  void print_all_statistics( void ) const;
};

#endif
