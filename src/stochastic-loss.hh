#ifndef STOCHASTICLOSS_HH
#define STOCHASTICLOSS_HH

#include "packet.hh"
#include <deque>
#include <tuple>
#include <random>
#include "exponential.hh"

class StochasticLoss
{
  private:
    std::deque< std::tuple< double, Packet > > _buffer;
    double _loss_rate;
    PRNG & _prng;
    std::bernoulli_distribution _distr;

  public:
    StochasticLoss( const double & rate, PRNG &prng ) :  _buffer(), _loss_rate( rate ), _prng( prng ), _distr() { _distr = std::bernoulli_distribution( rate );}
    template <class NextHop>
    void tick( NextHop & next, const double & tickno )
    {
      // pops items off buffer and sends them to the next item
      while ( (!_buffer.empty())) {
        next.accept(std::get< 1 >(_buffer.front()), tickno);
        _buffer.pop_front();
      }
    }
    void accept( const Packet & p, const double & tickno ) noexcept
    {
      if (!(_distr( _prng ))) {
        _buffer.emplace_back( tickno, p );
      }
    }


    double next_event_time( const double & tickno ) const
    {
      if ( _buffer.empty() ) {
        return std::numeric_limits<double>::max();
      }
      assert( std::get< 0 >( _buffer.front() ) >= tickno );
      return std::get< 0 >( _buffer.front() );
    }

};

#endif
