#include "sendergang.hh"

template <class NextHop>
void SenderGang::tick( NextHop & next, Receiver & rec, const double & tickno )
{
  /* let senders switch */
  for ( auto &x : _gang ) {
    x.switcher( tickno, _start_distribution, _stop_distribution, _num_sending );
  }

  /* recount number sending */
  _num_sending = accumulate( _gang.begin(), _gang.end(),
			     0, []( const unsigned int a, const SwitchedSender & b )
			     { return a + b.sending; } );

  /* run senders */
  for ( auto &x : _gang ) {
    x.tick( next, rec, tickno, _num_sending );
  }
}

template <class NextHop>
void SenderGang::SwitchedSender::tick( NextHop & next, Receiver & rec,
				       const double & tickno,
				       const unsigned int num_sending )
{
  /* receive feedback */
  if ( rec.readable( id ) ) {
    const std::vector< Packet > & packets = rec.packets_for( id );

    utility.packets_received( packets );
    sender->packets_received( packets );

    rec.clear( id );
  }

  /* possibly send packets */
  if ( sending ) {
    auto packets = sender->send( id, tickno );
    assert (packets.size() <= 1);
    if (packets.size() == 1) {
      next.accept( std::move( packets.at( 0 ) ), tickno );
    }
    utility.sending_duration( tickno - internal_tick, num_sending );
  }

  internal_tick = tickno;
}
