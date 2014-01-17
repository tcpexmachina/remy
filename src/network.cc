#include <algorithm>

#include "network.hh"

#include "sendergangofgangs.cc"
#include "link-templates.cc"

template <class SenderType1, class SenderType2>
Network<SenderType1, SenderType2>::Network( const SenderType1 & example_sender1,
                                            PRNG & s_prng,
                                            const NetConfig & config )
  : _prng( s_prng ),
    _senders_vector(),
    _link_vector( { config.link1_ppt, config.link2_ppt } ),
    _delay_vector( { config.delay, config.delay } ),
    _rec(),
    _router( _link_vector.at( 1 ), _rec ),
    _tickno( 0 )
{
  for (uint32_t i = 0; i < 3; i++ ) {
    _senders_vector.emplace_back( SenderGang<SenderType1>( config.mean_on_duration, config.mean_off_duration, 1, example_sender1, _prng, i ),
                                  SenderGang<SenderType2>() );
  }
}

template <class SenderType1, class SenderType2>
void Network<SenderType1, SenderType2>::tick( void )
{
  /* Set up the topology here manually */

  /* 1st flow traverses first link */
  _senders_vector.at( 0 ).tick( _link_vector.at( 0 ), _rec, _tickno );

  /* 2nd flow traverses second link */
  _senders_vector.at( 1 ).tick( _link_vector.at( 1 ), _rec, _tickno );

  /* 3rd flow traverses both */
  _senders_vector.at( 2 ).tick( _link_vector.at( 0 ), _rec, _tickno );

  /* Link1 methods */
  _link_vector.at( 0 ).tick( _delay_vector.at( 0 ), _tickno );
  _delay_vector.at( 0 ).tick( _router, _tickno );

  /* Link2 methods */
  _link_vector.at( 1 ).tick( _delay_vector.at( 1 ), _tickno );
  _delay_vector.at( 1 ).tick( _rec, _tickno );

}

template <class SenderType1, class SenderType2>
void Network<SenderType1, SenderType2>::run_simulation( const double & duration )
{
  assert( _tickno == 0 );
  typedef SenderGangofGangs<SenderType1, SenderType2> t_sender;

  while ( _tickno < duration ) {
    /* find element with soonest event */
    _tickno = min( { std::min_element( _senders_vector.begin(), _senders_vector.end(),
                                       [this] (const t_sender & x, const t_sender & y) { return x.next_event_time( _tickno ) < y.next_event_time( _tickno ); } )->next_event_time( _tickno ),
                     std::min_element( _link_vector.begin(), _link_vector.end(),
                                       [this] (const Link & x, const Link & y) { return x.next_event_time( _tickno ) < y.next_event_time( _tickno ); } )->next_event_time( _tickno ),
                     std::min_element( _delay_vector.begin(), _delay_vector.end(),
                                       [this] (const Delay & x, const Delay & y) { return x.next_event_time( _tickno ) < y.next_event_time( _tickno ); } )->next_event_time( _tickno ),
                     _rec.next_event_time( _tickno ) } );

    if ( _tickno > duration ) break;
    assert( _tickno < std::numeric_limits<double>::max() );

    tick();
  }
}
