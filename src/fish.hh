#ifndef FISH_HH
#define FISH_HH

#include <cassert>
#include <vector>
#include <string>

#include "packet.hh"
#include "memory.hh"
#include "random.hh"
#include "exponential.hh"
#include "fintree.hh"
#include "simulationresults.pb.h"

class Fish
{
private:
  const FinTree & _fins;
  Memory _memory;

  int _packets_sent, _packets_received;
  double _last_send_time;
  double _next_send_time;
  unsigned int _flow_id;
  int _largest_ack;
  bool _track;

  double _lambda;
  double _max_intersend;
  unsigned int _batch_size;

  PRNG _prng;
  Exponential _distribution;

  void _update_send_time( const double tickno );

  void _update_lambda( const double lambda );

public:
  Fish( const FinTree & fins, const unsigned int s_prng_seed, const bool s_track );

  void packets_received( const std::vector< Packet > & packets );
  void reset( const double & tickno ); /* start new flow */

  const FinTree & fins( void ) const { return _fins; }

  template <class NextHop>
  void send( const unsigned int id, NextHop & next, const double & tickno );

  Fish & operator=( const Fish & ) { assert( false ); return *this; }

  double next_event_time( const double & tickno ) const;

  const int & packets_sent( void ) const { return _packets_sent; }

  SimulationResultBuffers::SenderState state_DNA() const;

};

#endif
