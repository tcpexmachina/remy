#ifndef RAT_HH
#define RAT_HH

#include <vector>
#include <string>

#include "whiskertree.hh"
#include "memory.hh"
#include "sender_interface.hh"

class Rat : public SenderInterface
{
private:
  const WhiskerTree & _whiskers;
  Memory _memory;

  unsigned int _packets_sent, _packets_received;

  bool _track;

  double _last_send_time;

  unsigned int _the_window;
  double _intersend_time;

  unsigned int _flow_id;

public:
  Rat( WhiskerTree & s_whiskers, const bool s_track=false );

  void packets_received( const std::vector< Packet > & packets ) override;
  void reset( const double & tickno ) override; /* start new flow */

  std::vector<Packet> send( const unsigned int id, const double & tickno ) override;

  const WhiskerTree & whiskers( void ) const { return _whiskers; }

  Rat & operator=( const Rat & ) { assert( false ); }

  double next_event_time( const double & tickno ) const override;
};

#endif
