#ifndef NETWORK_HH
#define NETWORK_HH

#include <vector>
#include <string>

#include "sendergangofgangs.hh"
#include "link.hh"
#include "delay.hh"
#include "receiver.hh"
#include "router.hh"
#include "random.hh"
#include "answer.pb.h"

class NetConfig
{
public:
  double mean_on_duration, mean_off_duration;
  unsigned int num_senders;
  double link1_ppt;
  double link2_ppt;
  double delay;

  NetConfig( void )
    : mean_on_duration( 5000.0 ),
      mean_off_duration( 5000.0 ),
      num_senders( 8 ),
      link1_ppt( 1.0 ),
      link2_ppt( 1.0 ),
      delay( 150 )
  {}

  NetConfig( const RemyBuffers::NetConfig & dna )
    : mean_on_duration( dna.mean_on_duration() ),
      mean_off_duration( dna.mean_off_duration() ),
      num_senders( dna.num_senders() ),
      link1_ppt( dna.link_ppt() ),
      link2_ppt( dna.link_ppt() ),
      delay( dna.delay() )
  {}
  
  NetConfig & set_link1_ppt( const double s_link_ppt ) { link1_ppt = s_link_ppt; return *this; }
  NetConfig & set_link2_ppt( const double s_link_ppt ) { link2_ppt = s_link_ppt; return *this; }
  NetConfig & set_delay( const double s_delay ) { delay = s_delay; return *this; }
  NetConfig & set_num_senders( const unsigned int n ) { num_senders = n; return *this; }
  NetConfig & set_on_duration( const double & duration ) { mean_on_duration = duration; return *this; }
  NetConfig & set_off_duration( const double & duration ) { mean_off_duration = duration; return *this; }

  RemyBuffers::NetConfig DNA( void ) const
  {
      RemyBuffers::NetConfig ret;
      ret.set_mean_on_duration( mean_on_duration );
      ret.set_mean_off_duration( mean_off_duration );
      ret.set_num_senders( num_senders );
      ret.set_delay( delay );
      ret.set_link_ppt( link1_ppt );

      return ret;
  }

  std::string str( void ) const
  {
    char tmp[ 256 ];
    snprintf( tmp, 256, "mean_on=%f, mean_off=%f, nsrc=%d, link1_ppt=%f, link2_ppt=%f, delay=%f\n",
	      mean_on_duration, mean_off_duration, num_senders, link1_ppt, link2_ppt, delay );
    return tmp;
  }
};

template <class SenderType1, class SenderType2>
class Network
{
private:
  PRNG & _prng;

  std::vector< SenderGangofGangs<SenderType1, SenderType2> > _senders_vector;
  std::vector< Link > _link_vector;
  std::vector< Delay > _delay_vector;

  Receiver _rec;

  Router _router;

  double _tickno;

  void tick( void );

public:
  Network( const SenderType1 & example_sender1,
           PRNG & s_prng,
           const NetConfig & config );
  void run_simulation( const double & duration );

  const std::vector< SenderGangofGangs<SenderType1,SenderType2> > & senders_vector( void ) const { return _senders_vector; }
};

#endif
