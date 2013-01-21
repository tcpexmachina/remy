#ifndef NETWORK_HH
#define NETWORK_HH

#include "sendergang.hh"
#include "link.hh"
#include "delay.hh"
#include "receiver.hh"
#include "random.hh"

class NetConfig
{
public:
  double mean_on_duration, mean_off_duration;
  unsigned int num_senders;
  double link_ppt;
  double delay;

  NetConfig( void )
    : mean_on_duration( 1000.0 ),
      mean_off_duration( 1000.0 ),
      num_senders( 2 ),
      link_ppt( 1.0 ),
      delay( 100 )
  {}
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

  unsigned int _tickno;

public:
  Network( const SenderType & example_sender, PRNG & s_prng, const NetConfig & config );

  void tick( void );
  void tick( const unsigned int reps );

  const SenderGang<SenderType> & senders( void ) const { return _senders; }
};

#endif
