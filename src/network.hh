#ifndef NETWORK_HH
#define NETWORK_HH

#include <utility>
#include <string>

#include "sendergangofgangs.hh"
#include "link.hh"
#include "delay.hh"
#include "receiver.hh"
#include "random.hh"

class NetConfig
{
public:
  double mean_on_duration, mean_off_duration;
  unsigned int num_senders1;
  unsigned int num_senders2;
  double link_ppt;
  double delay;

  NetConfig( void )
    : mean_on_duration( 5000.0 ),
      mean_off_duration( 5000.0 ),
      num_senders1( 8 ),
      num_senders2( 8 ),
      link_ppt( 1.0 ),
      delay( 150 )
  {}

  NetConfig & set_link_ppt( const double s_link_ppt ) { link_ppt = s_link_ppt; return *this; }
  NetConfig & set_delay( const double s_delay ) { delay = s_delay; return *this; }
  NetConfig & set_num_senders1( const unsigned int n ) { num_senders1 = n; return *this; }
  NetConfig & set_num_senders2( const unsigned int n ) { num_senders2 = n; return *this; }
  NetConfig & set_on_duration( const double & duration ) { mean_on_duration = duration; return *this; }
  NetConfig & set_off_duration( const double & duration ) { mean_off_duration = duration; return *this; }

  std::string str( void ) const
  {
    char tmp[ 256 ];
    snprintf( tmp, 256, "mean_on=%f, mean_off=%f, nsrc1=%d, nsrc2=%d, link_ppt=%f, delay=%f\n buffer=%f\n",
	      mean_on_duration, mean_off_duration, num_senders1, num_senders2, link_ppt, delay, 2 * link_ppt * delay );
    return tmp;
  }
};

template <class SenderType1, class SenderType2>
class Network
{
private:
  PRNG & _prng;
  SenderGangofGangs<SenderType1, SenderType2> _senders;
  Link _link;
  Delay _delay;
  Receiver _rec;

  double _tickno;

  void tick( void );

public:
  Network( const SenderType1 & example_sender1, const SenderType2 & example_sender2, PRNG & s_prng, const NetConfig & config, const std::pair<double, double> & deltas = std::make_pair( 1.0, 1.0 ) );

  Network( const SenderType1 & example_sender1, PRNG & s_prng, const NetConfig & config );

  void run_simulation( const double & duration );

  const SenderGangofGangs<SenderType1,SenderType2> & senders( void ) const { return _senders; }
};

#endif
