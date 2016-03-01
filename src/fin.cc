#include <cassert>
#include <cmath>
#include <algorithm>
#include <boost/functional/hash.hpp>

#include "fin.hh"

using namespace std;

Fin::Fin( const Fin & other )
  : Action( other ),
    _lambda( other._lambda )
{
}

Fin::Fin( const RemyBuffers::Fin & dna )
  : Action ( dna.domain() ), 
    _lambda( dna.lambda() )
{
}

RemyBuffers::Fin Fin::DNA( void ) const
{
  RemyBuffers::Fin ret;

  ret.set_lambda( _lambda );
  ret.mutable_domain()->CopyFrom( _domain.DNA() );

  return ret;
}

vector< Fin > Fin::next_generation( void ) const
{
  vector< Fin> ret;

  auto lambda_alternatives = get_optimizer().lambda.alternatives( _lambda, true );
  
  printf("Alternatives: lambda %f to %f\n",
         *(min_element(lambda_alternatives.begin(), lambda_alternatives.end())),
         *(max_element(lambda_alternatives.begin(), lambda_alternatives.end()))
  );

  for ( const auto & alt_lambda : lambda_alternatives ) {
        Fin new_fin{ *this };
        new_fin._generation++;

        new_fin._lambda = alt_lambda;
        new_fin.round();

        ret.push_back( new_fin );
  }

  return ret;
}

string Fin::str( const unsigned int total ) const
{
  char tmp[ 256 ];
  snprintf( tmp, 256, "{%s} gen=%u usage=%.4f => (lambda=%f)",
            _domain.str().c_str(), _generation, double( _domain.count() ) / double( total ), _lambda );
  return tmp;
}

void Fin::round( void )
{
  _lambda = (1.0/10000.0) * int( 10000 * _lambda );
}

size_t hash_value( const Fin & fin )
{
  size_t seed = 0;
  boost::hash_combine( seed, fin._lambda );
  boost::hash_combine( seed, fin._domain );

  return seed;
}