#ifndef NETWORK_HH
#define NETWORK_HH

#include <string>

#include "sendergangofgangs.hh"
#include "link.hh"
#include "delay.hh"
#include "stochastic-loss.hh"
#include "receiver.hh"
#include "random.hh"
#include "answer.pb.h"

class SimulationRunData; // from simulationresults.hh

class NetConfig
{
public:
  double mean_on_duration, mean_off_duration;
  double num_senders;
  double link_ppt;
  double delay;
  double buffer_size;
  double stochastic_loss_rate;

  NetConfig( void )
    : mean_on_duration( 5000.0 ),
      mean_off_duration( 5000.0 ),
      num_senders( 8 ),
      link_ppt( 1.0 ),
      delay( 150 ),
      buffer_size( std::numeric_limits<unsigned int>::max() ),
      stochastic_loss_rate( 0 )
  {}

  NetConfig( const RemyBuffers::NetConfig & dna )
    : mean_on_duration( dna.mean_on_duration() ),
      mean_off_duration( dna.mean_off_duration() ),
      num_senders( dna.num_senders() ),
      link_ppt( dna.link_ppt() ),
      delay( dna.delay() ),
      buffer_size( dna.buffer_size() ),
      stochastic_loss_rate( dna.stochastic_loss_rate() )
  {}

  NetConfig & set_link_ppt( const double s_link_ppt ) { link_ppt = s_link_ppt; return *this; }
  NetConfig & set_delay( const double s_delay ) { delay = s_delay; return *this; }
  NetConfig & set_num_senders( const unsigned int n ) { num_senders = n; return *this; }
  NetConfig & set_on_duration( const double & duration ) { mean_on_duration = duration; return *this; }
  NetConfig & set_off_duration( const double & duration ) { mean_off_duration = duration; return *this; }
  NetConfig & set_buffer_size( const unsigned int n ) { buffer_size = n; return *this; }
  NetConfig & set_stochastic_loss_rate( const double loss_rate ) { stochastic_loss_rate = loss_rate; return *this; }

  RemyBuffers::NetConfig DNA( void ) const
  {
      RemyBuffers::NetConfig ret;
      ret.set_mean_on_duration( mean_on_duration );
      ret.set_mean_off_duration( mean_off_duration );
      ret.set_num_senders( num_senders );
      ret.set_delay( delay );
      ret.set_link_ppt( link_ppt );
      ret.set_buffer_size( buffer_size );
      ret.set_stochastic_loss_rate( stochastic_loss_rate );
      return ret;
  }

  std::string str( void ) const
  {
    char tmp[ 256 ];
    snprintf( tmp, 256, "mean_on=%f, mean_off=%f, nsrc=%f, link_ppt=%f, delay=%f, buffer_size=%f, stochastic_loss_rate = %f\n",
	     mean_on_duration, mean_off_duration, num_senders, link_ppt, delay, buffer_size, stochastic_loss_rate );
    return tmp;
  }
};

template <class Gang1Type, class Gang2Type>
class Network
{
private:
  PRNG & _prng;
  SenderGangofGangs<Gang1Type, Gang2Type> _senders;
  Link _link;
  Delay _delay;
  Receiver _rec;

  double _tickno;
  StochasticLoss _stochastic_loss;
  void tick( void );

public:
  Network( const typename Gang1Type::Sender & example_sender1, const typename Gang2Type::Sender & example_sender2, PRNG & s_prng, const NetConfig & config );

  Network( const typename Gang1Type::Sender & example_sender1, PRNG & s_prng, const NetConfig & config );

  void run_simulation( const double & duration );

  void run_simulation_with_logging_until( const double tick_limit, SimulationRunData &, const double interval );

  void run_simulation_until( const double tick_limit );

  const SenderGangofGangs<Gang1Type, Gang2Type> & senders( void ) const { return _senders; }

  SenderGangofGangs<Gang1Type, Gang2Type> & mutable_senders( void ) { return _senders; }

  std::vector< unsigned int > packets_in_flight( void ) const;

  const double & tickno( void ) const { return _tickno; }

  Link & mutable_link( void ) { return _link; }

  Delay & mutable_delay( void ) { return _delay; }
};

#endif
