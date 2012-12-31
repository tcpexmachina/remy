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
  SenderGang<SenderType> _senders;
  Link _link;
  Delay _delay;
  Receiver _rec;

  unsigned int _tickno;

public:
  Network( const SenderType & example_sender, PRNG & prng );

  void tick( void );

  const SenderGang<SenderType> & senders( void ) { return _senders; }
};

#endif
