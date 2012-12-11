#ifndef SENDERGANG_HH
#define SENDERGANG_HH

#include "peekable_queue.hh"

#include "window-sender.hh"
#include "exponential.hh"

class SenderGang
{
private:
  template< typename T, typename Container = std::deque< T >, typename Compare = std::greater< T > >
  class iterable_queue : public std::priority_queue< T, Container, Compare >
  {
  public:
    typedef typename Container::iterator iterator;

    iterator begin() { return this->c.begin(); }
    iterator end() { return this->c.end(); }
  };

  iterable_queue< std::pair< unsigned int, WindowSender > > _gang;

  Exponential _join_distribution;
  Exponential _flow_duration_distribution;
  const unsigned int _window_size;

  unsigned int _next_join_tick;

  std::tuple< double, double, double > _total_stats;

public:
  SenderGang( const double mean_interjoin_interval,
	      const double mean_flow_duration,
	      const unsigned int s_window_size );

  void tick( Network & net, Receiver & rec, const unsigned int tickno );
};

#endif
