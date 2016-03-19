#include <memory>
#include "sendergangofgangs.hh"
#include "sendergang.cc"

using namespace std;

template <class Gang1Type, class Gang2Type>
SenderGangofGangs<Gang1Type, Gang2Type>::SenderGangofGangs( const Gang1Type & gang1,
							    const Gang2Type & gang2 )
  : gang1_( gang1 ), gang2_( gang2 )
{
  /* Make sure no addresses conflict and no gap in address range */
  assert( gang1_.id_of_first_sender() == 0 );
  assert( gang2_.count_senders() == 0 or gang2_.id_of_first_sender() == gang1_.count_senders() );
}

template <class Gang1Type, class Gang2Type>
unsigned int SenderGangofGangs<Gang1Type, Gang2Type>::count_active_senders( void ) const
{
  return gang1_.count_active_senders() + gang2_.count_active_senders();
}

template <class Gang1Type, class Gang2Type>
void SenderGangofGangs<Gang1Type, Gang2Type>::switch_senders( const unsigned int num_sending,
							      const double & tickno )
{
  gang1_.switch_senders( num_sending, tickno );
  gang2_.switch_senders( num_sending, tickno );
}

template <class Gang1Type, class Gang2Type>
template <class NextHop>
void SenderGangofGangs<Gang1Type, Gang2Type>::run_senders( NextHop & next, Receiver & rec,
							   const unsigned int num_sending,
							   const double & tickno )
{
  gang1_.run_senders( next, rec, num_sending, tickno );
  gang2_.run_senders( next, rec, num_sending, tickno );
}

/* Same implementation as a SenderGang */
template <class Gang1Type, class Gang2Type>
template <class NextHop>
void SenderGangofGangs<Gang1Type, Gang2Type>::tick( NextHop & next, Receiver & rec, const double & tickno )
{
  unsigned int num_sending = count_active_senders();

  switch_senders( num_sending, tickno );

  num_sending = count_active_senders();

  run_senders( next, rec, num_sending, tickno );
}

template <class Gang1Type, class Gang2Type>
double SenderGangofGangs<Gang1Type, Gang2Type>::utility( void ) const
{
  return gang1_.utility() + gang2_.utility();
}

template <class Gang1Type, class Gang2Type>
vector< pair< double, double > > SenderGangofGangs<Gang1Type, Gang2Type>::throughputs_delays( void ) const
{
  auto ret = gang1_.throughputs_delays();
  const auto gang2_tpd = gang2_.throughputs_delays();

  ret.insert( ret.end(), gang2_tpd.begin(), gang2_tpd.end() );

  return ret;
}

template <class Gang1Type, class Gang2Type>
vector < SenderDataPoint > SenderGangofGangs<Gang1Type,Gang2Type>::statistics_for_log( void ) const
{
  auto ret = gang1_.statistics_for_log();
  const auto gang2_stats = gang2_.statistics_for_log();
  ret.insert( ret.end(), gang2_stats.begin(), gang2_stats.end() );
  return ret;
}

template <class Gang1Type, class Gang2Type>
double SenderGangofGangs<Gang1Type, Gang2Type>::next_event_time( const double & tickno ) const
{
  return min( gang1_.next_event_time( tickno ), gang2_.next_event_time( tickno ) );
}
