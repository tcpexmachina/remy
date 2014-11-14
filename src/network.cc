#include "network.hh"

#include "sendergangofgangs.cc"
#include "queue-templates.cc"
#include "jitter-templates.cc"
#include "link-templates.cc"

template <class SenderType1, class SenderType2>
Network<SenderType1, SenderType2>::Network( const SenderType1 & example_sender1,
                                            const SenderType2 & example_sender2,
                                            PRNG & s_prng,
                                            const NetConfig & config )
  : _prng( s_prng ),
    _senders( SenderGang<SenderType1>( config.mean_on_duration, config.mean_off_duration, config.num_senders, example_sender1, _prng ),
	      SenderGang<SenderType2>( config.mean_on_duration, config.mean_off_duration, config.num_senders, example_sender2, _prng, config.num_senders ) ),
    _jitter( 1.0, _prng ),
    _uplink( config.trace ),
    _downlink( config.trace ),
    _uplink_delay( config.delay / 2.0 ),
    _downlink_delay( config.delay / 2.0 ),
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
    _jitter(  1.0, _prng ),
    _uplink( config.trace ),
    _downlink( config.trace ),
    _uplink_delay( config.delay / 2.0 ),
    _downlink_delay( config.delay / 2.0 ),
    _rec(),
    _tickno( 0 )
{
}

template <class SenderType1, class SenderType2>
void Network<SenderType1, SenderType2>::tick( void )
{
  _senders.tick( _uplink, _rec, _tickno );
  _uplink.tick( _uplink_delay, _tickno );
  _uplink_delay.tick( _downlink_delay, _tickno );
  _downlink_delay.tick( _downlink, _tickno );
  _downlink.tick( _jitter, _tickno );
  _jitter.tick( _rec, _tickno );
}

template <class SenderType1, class SenderType2>
void Network<SenderType1, SenderType2>::run_simulation( const double & duration )
{ 
  assert( _tickno == 0 );

  while ( _tickno < duration ) {
    /* find element with soonest event */
    _tickno = min( min( _uplink_delay.next_event_time( _tickno ), 
                        ( min( _senders.next_event_time( _tickno ),
                               _uplink.next_event_time( _tickno ) ) ) ),
		   min( _downlink_delay.next_event_time( _tickno ),
                        min( _rec.next_event_time( _tickno ),
                             min( _downlink.next_event_time( _tickno ),
                             _jitter.next_event_time( _tickno ) ) ) ) );
      /*_tickno = min( min( _uplink_delay.next_event_time( _tickno ), 
                        ( min( _senders.next_event_time( _tickno ),
                               _uplink.next_event_time( _tickno ) ) ) ),
		   min( _downlink_delay.next_event_time( _tickno ),
                        min( _rec.next_event_time( _tickno ),
                        _downlink.next_event_time( _tickno ) ) ) );*/
    /*_tickno = min( ( min( _senders.next_event_time( _tickno ),
                            _link.next_event_time( _tickno ) ) ),
                     min( _delay.next_event_time( _tickno ),
                     _rec.next_event_time( _tickno ) ) );*/
    
    if ( _tickno > duration ) break;
    assert( _tickno < std::numeric_limits<double>::max() );

    tick();
  }
}
