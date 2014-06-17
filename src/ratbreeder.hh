#ifndef RATBREEDER_HH
#define RATBREEDER_HH

#include <string>
#include <unordered_map>
#include <boost/functional/hash.hpp>

#include "network.hh"
#include "evaluator.hh"
#include "http_transmitter.hh"
#include "dna.pb.h"

class WhiskerImprover
{
private:
  bool evaluate_if_done( std::tuple< Whisker, bool, std::string > & candidate,
                         HttpTransmitter & http_transmitter );

  const Evaluator eval_;

  WhiskerTree rat_;

  std::unordered_map< Whisker, double, boost::hash< Whisker > > eval_cache_ {};

  double score_to_beat_;

  std::string http_server_;

  std::string http_host_;

  uint32_t sleep_time_us_; /* Time to sleep in us before retrieving results */

public:
  WhiskerImprover( const Evaluator & evaluator, const WhiskerTree & rat, const double score_to_beat, const uint32_t s_sleep_time_us );
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
