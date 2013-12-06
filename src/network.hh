#ifndef NETWORK_HH
#define NETWORK_HH

#include <string>
#include <boost/random/uniform_real_distribution.hpp>
#include <boost/random/uniform_int_distribution.hpp>

#include "configrange.hh"
#include "sendergang.hh"
#include "link.hh"
#include "delay.hh"
#include "receiver.hh"
#include "random.hh"

class NetConfig
{
private:
  bool is_random;
  ConfigRange _range;

public:
  double mean_on_duration, mean_off_duration;
  unsigned int num_senders;
  double link_ppt;
  double delay;

  NetConfig( void )
    : is_random( false ),
      _range(),
      mean_on_duration( 5000.0 ),
      mean_off_duration( 5000.0 ),
      num_senders( 8 ),
      link_ppt( 1.0 ),
      delay( 150 )
  {}

  /* 
     If a configuration range is provided as an argument, 
     draw the network's parameters uniformly at random from the 
     configuration range whenever initialize() is called on this NetConfig.
     Until initialize() is called, the parameters are set to the default
     configuration.
  */
  NetConfig( ConfigRange & range )
    : is_random( true ),
      _range( range ),
      mean_on_duration( 5000.0 ),
      mean_off_duration( 5000.0 ),
      num_senders( 8 ),
      link_ppt( 1.0 ),
      delay( 150 )
  {}

  void initialize()
  {
    if(!is_random) {
      // Not to be randomly generated. Parameters should already be set.
      return; 
    }

    boost::random::uniform_real_distribution<> link_speed( _range.link_packets_per_ms.first, _range.link_packets_per_ms.second );
    boost::random::uniform_real_distribution<> rtt( _range.rtt_ms.first, _range.rtt_ms.second );
    boost::random::uniform_int_distribution<> nsenders( 1, _range.max_senders );

    link_ppt = link_speed(global_PRNG());
    delay = rtt(global_PRNG());
    num_senders = nsenders(global_PRNG());
    mean_on_duration = _range.mean_on_duration;
    mean_off_duration = _range.mean_off_duration;
  }

  NetConfig set_link_ppt( const double s_link_ppt ) { link_ppt = s_link_ppt; return *this; }
  NetConfig set_delay( const double s_delay ) { delay = s_delay; return *this; }
  NetConfig set_num_senders( const unsigned int n ) { num_senders = n; return *this; }
  NetConfig set_on_duration( const double & duration ) { mean_on_duration = duration; return *this; }
  NetConfig set_off_duration( const double & duration ) { mean_off_duration = duration; return *this; }

  std::string str( void ) const
  {
    char tmp[ 256 ];
    snprintf( tmp, 256, "mean_on=%f, mean_off=%f, nsrc=%d, link_ppt=%f, delay=%f\n",
	      mean_on_duration, mean_off_duration, num_senders, link_ppt, delay );
    return tmp;
  }
};

template <class SenderType>
class Network
{
private:
  PRNG & _prng;
  SenderGang<SenderType> _senders;
  Link _link;
  Delay _delay;
  Receiver _rec;

  double _tickno;

  void tick( void );

public:
  Network( const SenderType & example_sender, PRNG & s_prng, const NetConfig & config );

  void run_simulation( const double & duration );

  const SenderGang<SenderType> & senders( void ) const { return _senders; }
};

#endif
