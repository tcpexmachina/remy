#ifndef RATBREEDER_HH
#define RATBREEDER_HH

#include "breeder.hh"

struct WhiskerImproverOptions
{
  bool optimize_window_increment = true;
  bool optimize_window_multiple = true;
  bool optimize_intersend = true;
};

class WhiskerImprover : public ActionImprover< WhiskerTree, Whisker >
{
protected:
  WhiskerImproverOptions _options;

  std::vector< Whisker > get_replacements( Whisker & action_to_improve );

public:
  WhiskerImprover( const Evaluator<  WhiskerTree > & evaluator, const WhiskerTree & rat, const WhiskerImproverOptions & options,
                   const double score_to_beat )
    : ActionImprover< WhiskerTree, Whisker >( evaluator, rat, score_to_beat),
      _options( options ) {};
};

class RatBreeder : public Breeder< WhiskerTree >
{
private:
  WhiskerImproverOptions _whisker_options;

public:
  RatBreeder( const BreederOptions & s_options, const WhiskerImproverOptions & s_whisker_options ) 
    : Breeder( s_options ), _whisker_options( s_whisker_options ) {};

  Evaluator< WhiskerTree >::Outcome improve( WhiskerTree & whiskers );
};

#endif
