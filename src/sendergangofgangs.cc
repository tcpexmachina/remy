#include <memory>
#include "sendergangofgangs.hh"
#include "sendergang.cc"

using namespace std;

template <class Sender1, class Sender2>
SenderGangofGangs<Sender1,Sender2>::SenderGangofGangs( SenderGang<Sender1> && gang1,
						       SenderGang<Sender2> && gang2 )
  : gang1_( move( gang1 ) ), gang2_( move( gang2 ) )
{
  /* Make sure no addresses conflict and no gap in address range */
  assert( gang1_.id_of_first_sender() == 0 );
  assert( gang2_.id_of_first_sender() == gang1_.count_senders() );
}

template <class Sender1, class Sender2>
unsigned int SenderGangofGangs<Sender1,Sender2>::count_active_senders( void ) const
{
  return gang1_.count_active_senders() + gang2_.count_active_senders();
}

template <class Sender1, class Sender2>
void SenderGangofGangs<Sender1,Sender2>::switch_senders( const unsigned int num_sending,
							 const double & tickno )
{
  gang1_.switch_senders( num_sending, tickno );
  gang2_.switch_senders( num_sending, tickno );
}

template <class Sender1, class Sender2>
template <class NextHop>
void SenderGangofGangs<Sender1,Sender2>::run_senders( NextHop & next, Receiver & rec,
						      const unsigned int num_sending,
						      const double & tickno )
{
  gang1_.run_senders( next, rec, num_sending, tickno );
  gang2_.run_senders( next, rec, num_sending, tickno );
}

/* Same implementation as a SenderGang */
template <class Sender1, class Sender2>
template <class NextHop>
void SenderGangofGangs<Sender1,Sender2>::tick( NextHop & next, Receiver & rec, const double & tickno )
{
  unsigned int num_sending = count_active_senders();

  switch_senders( num_sending, tickno );

  num_sending = count_active_senders();

  run_senders( next, rec, num_sending, tickno );
}

template <class Sender1, class Sender2>
double SenderGangofGangs<Sender1,Sender2>::utility( void ) const
{
  return gang1_.utility() + gang2_.utility();
}

template <class Sender1, class Sender2>
vector< pair< double, double > > SenderGangofGangs<Sender1,Sender2>::throughputs_delays( void ) const
{
  auto ret = gang1_.throughputs_delays();
  const auto gang2_tpd = gang2_.throughputs_delays();

  ret.insert( ret.end(), gang2_tpd.begin(), gang2_tpd.end() );

  return ret;
}

template <class Sender1, class Sender2>
double SenderGangofGangs<Sender1,Sender2>::next_event_time( const double & tickno ) const
{
  return min( gang1_.next_event_time( tickno ), gang2_.next_event_time( tickno ) );
}
