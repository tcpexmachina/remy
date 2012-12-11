#include "sendergang.hh"

SenderGang::SenderGang( const double mean_interjoin_interval,
			const double mean_flow_duration,
			const unsigned int s_window_size )
  : _gang(),
    _join_distribution( 1.0 / mean_interjoin_interval ),
    _flow_duration_distribution( 1.0 / mean_flow_duration ),
    _window_size( s_window_size ),
    _next_join_tick( _join_distribution.sample() ),
    _total_stats(),
    _num_stats( 0 )
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
				 _window_size ) );
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

    auto stats( x.second.stats( tickno ) );
    if ( stats.first >= 0 ) {
      _total_stats.first += stats.first;
      _total_stats.second += stats.second;
      _num_stats++;
    }

    if ( _num_stats % 10000 == 0 ) {
      fprintf( stderr, "completed flows = %d, avg tput = %f, avg avg delay = %f\n",
	       _num_stats,
	       _total_stats.first / _num_stats,
	       _total_stats.second / _num_stats );
    }

    rec.free_src( x.second.id() );
  }
}
