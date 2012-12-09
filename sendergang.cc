#include "sendergang.hh"

SenderGang::SenderGang( const double mean_interjoin_interval,
			const double mean_flow_duration,
			const unsigned int s_window_size )
  : _gang(),
    _join_distribution( mean_interjoin_interval ),
    _flow_duration_distribution( mean_flow_duration ),
    _window_size( s_window_size ),
    _next_join_tick( _join_distribution.sample() )
{
}

void SenderGang::tick( Network & net, Receiver & rec, const unsigned int tickno )
{
  /* add senders */
  while ( tickno >= _next_join_tick ) {
    _gang.emplace( _next_join_tick + _flow_duration_distribution.sample(),
		   WindowSender( 0, _window_size ) );
    _next_join_tick += _join_distribution.sample();
  }
}
