#ifndef RATBREEDER_HH
#define RATBREEDER_HH

#include <unordered_map>
#include <boost/functional/hash.hpp>

#include "network.hh"
#include "evaluator.hh"
#include "dna.pb.h"

class WhiskerImprover
{
private:
  const Evaluator eval_;

  WhiskerTree rat_;

  std::unordered_map< Whisker, double, boost::hash< Whisker > > eval_cache_ {};

  double score_to_beat_;

public:
  WhiskerImprover( const Evaluator & evaluator, const WhiskerTree & rat, const double score_to_beat );
  double improve( Whisker & whisker_to_improve );
};

class RatBreeder
{
private:
  std::vector<NetConfig> _net_configs;

  void apply_best_split( WhiskerTree & whiskers, const unsigned int generation ) const;

public:
  RatBreeder( const std::vector<NetConfig> & s_net_configs );

  Evaluator::Outcome improve( WhiskerTree & whiskers );
};

#endif
