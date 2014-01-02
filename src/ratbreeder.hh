#ifndef RATBREEDER_HH
#define RATBREEDER_HH

#include <unordered_map>
#include <boost/functional/hash.hpp>

#include "configrange.hh"
#include "evaluator.hh"

class WhiskerImprover
{
private:
  const Evaluator eval_;
  std::unordered_map< Whisker, double, boost::hash< Whisker > > eval_cache_ {};

  double score_to_beat_;

public:
  WhiskerImprover( const Evaluator & evaluator, const double score_to_beat );
  double improve( Whisker & whisker_to_improve, const ZigZag & tree_to_improve );
};

class RatBreeder
{
private:
  ConfigRange _range;

  void apply_best_split( WhiskerTree & whiskers1, WhiskerTree & whiskers2, const unsigned int generation, const ZigZag & tree_to_split ) const;

public:
  RatBreeder( const ConfigRange & s_range ) : _range( s_range ) {}

  Evaluator::Outcome improve( WhiskerTree & whiskers1, WhiskerTree & whiskers2 );
};

#endif
