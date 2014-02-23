#include "network.hh"

#include "sendergangofgangs.cc"
#include "link-templates.cc"

template <class SenderType1, class SenderType2>
Network<SenderType1, SenderType2>::Network( const SenderType1 & example_sender1,
                                            const SenderType2 & example_sender2,
                                            PRNG & s_prng,
                                            const NetConfig & config )
  : _prng( s_prng ),
    _senders( SenderGang<SenderType1>( config.mean_on_duration, config.mean_off_duration, config.num_senders, example_sender1, _prng ),
	      SenderGang<SenderType2>( config.mean_on_duration, config.mean_off_duration, config.num_senders, example_sender2, _prng, config.num_senders ) ),
    _link( config.link_ppt ),
    _delay( config.delay ),
    _rec(),
    _tickno( 0 )
{
}

template <class SenderType1, class SenderType2>
Network<SenderType1, SenderType2>::Network( const SenderType1 & example_sender1,
                                            PRNG & s_prng,
                                            const NetConfig & config )
  : _prng( s_prng ),
    _senders( SenderGang<SenderType1>( config.mean_on_duration, config.mean_off_duration, config.num_senders, example_sender1, _prng ),
	      SenderGang<SenderType2>() ),
    _link( config.link_ppt ),
    _delay( config.delay ),
    _rec(),
    _tickno( 0 )
{
}

template <class SenderType1, class SenderType2>
void Network<SenderType1, SenderType2>::tick( void )
{
  _senders.tick( _link, _rec, _tickno );
  _link.tick( _delay, _tickno );
  _delay.tick( _rec, _tickno );
}

template <class SenderType1, class SenderType2>
void Network<SenderType1, SenderType2>::run_simulation( const double & duration )
{
  assert( _tickno == 0 );

  while ( _tickno < duration ) {
    /* find element with soonest event */
    _tickno = min( min( _senders.next_event_time( _tickno ),
			_link.next_event_time( _tickno ) ),
		   min( _delay.next_event_time( _tickno ),
			_rec.next_event_time( _tickno ) ) );

    if ( _tickno > duration ) break;
    assert( _tickno < std::numeric_limits<double>::max() );

    tick();
  }
}

template <class SenderType1, class SenderType2>
void Network<SenderType1, SenderType2>::run_simulation_until( const double tick_limit )
{
  if ( _tickno >= tick_limit ) {
    return;
  }

  while ( true ) {
    /* find element with soonest event */
    double next_tickno = min( min( _senders.next_event_time( _tickno ),
				   _link.next_event_time( _tickno ) ),
			      min( _delay.next_event_time( _tickno ),
				   _rec.next_event_time( _tickno ) ) );

    if ( next_tickno > tick_limit ) break;
    assert( next_tickno < std::numeric_limits<double>::max() );

    _tickno = next_tickno;

    tick();
  }
}
