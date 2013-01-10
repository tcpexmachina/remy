#include "network.hh"

#include "sendergang.cc"
#include "link-templates.cc"

static const double MEAN_ON_DURATION  = 1000.0;
static const double MEAN_OFF_DURATION = 1000.0;
static const unsigned int NUM_SENDERS = 2;
static const double LINK_MEAN_PPS = 1.0;
static const double DELAY = 100;

template <class SenderType>
Network<SenderType>::Network( const SenderType & example_sender, PRNG & s_prng )
  : _prng( s_prng ),
    _senders( MEAN_ON_DURATION, MEAN_OFF_DURATION, NUM_SENDERS, example_sender, _prng ),
    _link( LINK_MEAN_PPS, _prng ),
    _delay( DELAY ),
    _rec(),
    _tickno( 0 )
{
}

template <class SenderType>
void Network<SenderType>::tick( void )
{
  _senders.tick( _link, _rec, _tickno );
  _link.tick( _delay, _tickno );
  _delay.tick( _rec, _tickno );
  _tickno++;
}

template <class SenderType>
void Network<SenderType>::tick( const unsigned int reps )
{
  for ( unsigned int i = 0; i < reps; i++ ) {
    tick();
  }
}
