#ifndef RATBREEDER_HH
#define RATBREEDER_HH

#include <unordered_map>
#include <boost/functional/hash.hpp>

#include "configrange.hh"
#include "evaluator.hh"

struct WhiskerImproverOptions
{
  bool optimize_window_increment = true;
  bool optimize_window_multiple = true;
  bool optimize_intersend = true;
};

struct RatBreederOptions
{
  ConfigRange config_range = ConfigRange();
  WhiskerImproverOptions improver_options = WhiskerImproverOptions();
};

class WhiskerImprover
{
private:
  const Evaluator eval_;

  WhiskerTree rat_;
  WhiskerImproverOptions options_;

  std::unordered_map< Whisker, double, boost::hash< Whisker > > eval_cache_ {};

  double score_to_beat_;

public:
  WhiskerImprover( const Evaluator & evaluator, const WhiskerTree & rat, const WhiskerImproverOptions & options,
                   const double score_to_beat );
  double improve( Whisker & whisker_to_improve );
};

class RatBreeder
{
private:
  RatBreederOptions _options;

  void apply_best_split( WhiskerTree & whiskers, const unsigned int generation ) const;

public:
  RatBreeder( const RatBreederOptions & s_options ) : _options( s_options ) {}

  Evaluator::Outcome improve( WhiskerTree & whiskers );
};

#endif
