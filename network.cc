#include "network.hh"

#include "sendergang.cc"
#include "link-templates.cc"

template <class SenderType>
Network<SenderType>::Network( const SenderType & example_sender, PRNG & s_prng, const NetConfig & config )
  : _prng( s_prng ),
    _senders( config.mean_on_duration, config.mean_off_duration, config.num_senders, example_sender, _prng ),
    _link( config.link_ppt ),
    _delay( config.delay ),
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
