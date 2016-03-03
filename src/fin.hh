#ifndef FIN_HH
#define FIN_HH

#include <string>
#include <vector>

#include "memoryrange.hh"
#include "action.hh"
#include "dna.pb.h"

class Fin : public Action {
private:
  double _lambda;
public:
  Fin( const Fin & other );
  Fin( const double s_rate, const MemoryRange & s_domain ) : Action( s_domain ), _lambda( s_rate) {};
  Fin( const MemoryRange & s_domain ) : Fin( get_optimizer().lambda.default_value, s_domain ) {};
  virtual ~Fin() {};

  const double & lambda( void ) const { return _lambda; }
  
  std::vector< Fin > next_generation( void ) const;

  std::string str( const unsigned int total=1 ) const;

  RemyBuffers::Fin DNA( void ) const;
  Fin( const RemyBuffers::Fin & dna );
  
  void round( void );
  
  bool operator==( const Fin & other ) const { return (_lambda == other._lambda) && (_domain == other._domain); }
  
  friend size_t hash_value( const Fin & Fin );

  struct OptimizationSettings
  {
    OptimizationSetting< double > lambda;

    RemyBuffers::OptimizationSettings DNA( void ) const
    {
      RemyBuffers::OptimizationSettings ret;

      ret.mutable_lambda()->CopyFrom( lambda.DNA() );

      return ret;
    }
  };

  static const OptimizationSettings & get_optimizer( void ) {
    static OptimizationSettings default_settings {
      { 0.01, 30,   0.01, 3,   4, 5 },  /* lambda */
    };
    return default_settings;
  }
};

#endif
