#ifndef EVALUATOR_HH
#define EVALUATOR_HH

#include <vector>

#include "random.hh"
#include "whiskertree.hh"
#include "network.hh"

enum class ZigZag {
  ZIG,
  ZAG,
};
ZigZag& operator!(ZigZag& z);

class Evaluator
{
public:
  class Outcome
  {
  public:
    double score;
    std::vector< std::pair< NetConfig, std::vector< std::tuple< std::string, double, double > > > > throughputs_delays;
    std::pair<WhiskerTree, WhiskerTree> used_whiskers;
    WhiskerTree & get_used_whiskers( const ZigZag & tree_id ) { return tree_id == ZigZag::ZIG ? used_whiskers.first : used_whiskers.second; }

    Outcome() : score( 0 ), throughputs_delays(), used_whiskers() {}
  };

private:
  const PRNG _prng;
  const WhiskerTree _whiskers1;
  const WhiskerTree _whiskers2;

  std::vector< NetConfig > _configs;

public:
  Evaluator( const WhiskerTree & s_whiskers1, const WhiskerTree & s_whiskers2, const ConfigRange & range );
  Outcome score( WhiskerTree & run_whiskers1, WhiskerTree & run_whiskers2, const bool trace = false, const unsigned int carefulness = 1 ) const;
  Outcome score( const std::vector< Whisker > & replacements, ZigZag tree_id, const bool trace = false, const unsigned int carefulness = 1 ) const;
};

#endif
