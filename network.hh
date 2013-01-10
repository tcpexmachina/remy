#ifndef NETWORK_HH
#define NETWORK_HH

#include "sendergang.hh"
#include "link.hh"
#include "delay.hh"
#include "receiver.hh"
#include "random.hh"

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
  Network( const SenderType & example_sender, PRNG & s_prng );

  void tick( void );
  void tick( const unsigned int reps );

  const SenderGang<SenderType> & senders( void ) const { return _senders; }
};

#endif
