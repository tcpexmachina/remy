#include "sendergang.hh"

SenderGang::SenderGang( const double mean_interjoin_interval,
			const double mean_flow_duration,
			const unsigned int s_window_size )
  : _gang(),
    _join_distribution( 1.0 / mean_interjoin_interval ),
    _flow_duration_distribution( 1.0 / mean_flow_duration ),
    _window_size( s_window_size ),
    _next_join_tick( _join_distribution.sample() )
{
}

void SenderGang::tick( Network & net, Receiver & rec, const unsigned int tickno )
{
  /* add senders */
  while ( tickno >= _next_join_tick ) {
    auto src_and_flow = rec.new_src(); 
    _gang.emplace( _next_join_tick + _flow_duration_distribution.sample(),
		   WindowSender( src_and_flow.first,
				 src_and_flow.second,
				 _window_size,
				 tickno ) );
    _next_join_tick += _join_distribution.sample();
  }

  /* run senders */
  for ( auto &x : _gang ) {
    x.second.tick( net, rec, tickno );
  }

  /* delete senders */
  while ( (!_gang.empty())
	  && (_gang.top().first <= tickno) ) {
    auto x( std::move( _gang.top() ) );
    _gang.pop();

    x.second.print_stats( tickno );
    rec.free_src( x.second.id() );
  }
}
