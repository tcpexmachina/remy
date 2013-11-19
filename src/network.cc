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
}

template <class SenderType>
void Network<SenderType>::run_simulation( const double & duration )
{
  assert( _tickno == 0 );

  while ( _tickno < duration ) {
    /* find element with soonest event */
    _tickno = min( min( _senders.next_event_time( _tickno ),
			_link.next_event_time( _tickno ) ),
		   min( _delay.next_event_time( _tickno ),
			_rec.next_event_time( _tickno ) ) );

    assert( _tickno < std::numeric_limits<double>::max() );

    if ( _tickno > duration ) break;
    tick();
  }
}
