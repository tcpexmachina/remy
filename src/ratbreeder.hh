#ifndef RATBREEDER_HH
#define RATBREEDER_HH

//#include "configrange.hh"
#include "network.hh"
#include "evaluator.hh"

class RatBreeder
{
private:
  //  ConfigRange _range;
  std::vector<NetConfig> _configs;

  void apply_best_split( WhiskerTree & whiskers, const unsigned int generation );

public:
  //  RatBreeder( ConfigRange & s_range ) : _range( s_range ) {}
  RatBreeder( std::vector<NetConfig> & configs ) : _configs( configs ) {}

  Evaluator::Outcome improve( WhiskerTree & whiskers );
};

#endif
