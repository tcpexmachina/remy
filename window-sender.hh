#ifndef WINDOWSENDER_HH
#define WINDOWSENDER_HH

#include "poisson.hh"
#include "network.hh"
#include "utility.hh"

template <class NextHop>
class WindowSender
{
private:
  unsigned int _id;
  unsigned int _window;
  unsigned int _packets_sent, _packets_received;

  bool _sending;

  Utility _utility;

public:
  WindowSender( const unsigned int s_id,
		const unsigned int s_window );

  void tick( NextHop & next, Receiver & rec, const unsigned int tickno );

  void set_sending( const bool s_sending ) { _sending = s_sending; }
  bool sending( void ) const { return _sending; }

  double utility( void ) const { return _utility.utility(); }
};

#endif
