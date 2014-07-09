#include "network.hh"

#include "sendergangofgangs.cc"
#include "link-templates.cc"

/* Base class constructors */
template <class SenderType1, class SenderType2, class LinkType>
NetworkBase<SenderType1, SenderType2, LinkType>::NetworkBase( const SenderType1 & example_sender1,
                                            const SenderType2 & example_sender2,
                                            PRNG & s_prng,
                                            const NetConfig & config,
                                            const LinkType & link )
  : _prng( s_prng ),
    _senders( SenderGang<SenderType1>( config.mean_on_duration, config.mean_off_duration, config.num_senders, example_sender1, _prng ),
	      SenderGang<SenderType2>( config.mean_on_duration, config.mean_off_duration, config.num_senders, example_sender2, _prng, config.num_senders ) ),
    _link( link ),
    _delay( config.delay ),
    _rec(),
    _tickno( 0 )
{
}

template <class SenderType1, class SenderType2, class LinkType>
NetworkBase<SenderType1, SenderType2, LinkType>::NetworkBase( const SenderType1 & example_sender1,
                                            PRNG & s_prng,
                                            const NetConfig & config,
                                            const LinkType & link )
  : _prng( s_prng ),
    _senders( SenderGang<SenderType1>( config.mean_on_duration, config.mean_off_duration, config.num_senders, example_sender1, _prng ),
	      SenderGang<SenderType2>() ),
    _link( link ),
    _delay( config.delay ),
    _rec(),
    _tickno( 0 )
{
}

/* IsochronousLink network constructors */
template <class SenderType1, class SenderType2>
Network<SenderType1, SenderType2, IsochronousLink>::Network( const SenderType1 & example_sender1,
                                            const SenderType2 & example_sender2,
                                            PRNG & s_prng,
                                            const NetConfig & config )
  : Network<SenderType1, SenderType2, IsochronousLink>( example_sender1, example_sender2, s_prng, config, IsochronousLink( config.link_ppt ) )
{
}

template <class SenderType1, class SenderType2>
Network<SenderType1, SenderType2, IsochronousLink>::Network( const SenderType1 & example_sender1,
                                            PRNG & s_prng,
                                            const NetConfig & config )
  : NetworkBase<SenderType1, SenderType2, IsochronousLink>( example_sender1, s_prng, config, IsochronousLink( config.link_ppt ) )
{
}

/* TraceLink constructors */
template <class SenderType1, class SenderType2>
Network<SenderType1, SenderType2, TraceLink>::Network( const SenderType1 & example_sender1,
                                            const SenderType2 & example_sender2,
                                            PRNG & s_prng,
                                            const NetConfig & config )
  : NetworkBase<SenderType1, SenderType2, TraceLink>( example_sender1, example_sender2, s_prng, config, TraceLink( config.trace ) )
{
}

template <class SenderType1, class SenderType2>
Network<SenderType1, SenderType2, TraceLink>::Network( const SenderType1 & example_sender1,
                                            PRNG & s_prng,
                                            const NetConfig & config )
  : NetworkBase<SenderType1, SenderType2, TraceLink>( example_sender1, s_prng, config, TraceLink( config.trace ) )
{
}


template <class SenderType1, class SenderType2, class LinkType>
void NetworkBase<SenderType1, SenderType2, LinkType>::tick( void )
{
  _senders.tick( _link, _rec, _tickno );
  _link.tick( _delay, _tickno );
  _delay.tick( _rec, _tickno );
}

template <class SenderType1, class SenderType2, class LinkType>
void NetworkBase<SenderType1, SenderType2, LinkType>::run_simulation( const double & duration )
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
