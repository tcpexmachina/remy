#ifndef STOCHASTIC_LOSS
#define STOCHASTIC_LOSS
#include <random>
#include <vector>
#include "exponential.hh"
#include "packet.hh"

class StochasticLoss 
{
  private:
   std::bernoulli_distribution _distr;
  public:
    StochasticLoss( const double & rate ): _distr() { _distr = std::bernoulli_distribution( rate ); }

    void set_rate( const double & rate ) 
    {
      _distr = std::bernoulli_distribution( rate );
    }

    std::vector< Packet > drop_packets( std::vector< Packet > packets, PRNG & prng ) 
    {
      std::vector< Packet > new_packets;
      new_packets = std::vector< Packet >();
      for (const auto &x: packets) {
        if (!( _distr ( prng ) )) {
          new_packets.push_back(x);
        }
      }
      return new_packets;
    }  
};
#endif
