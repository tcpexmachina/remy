#ifndef WINDOWSENDER_HH
#define WINDOWSENDER_HH

#include "poisson.hh"
#include "network.hh"

class WindowSender
{
private:
  unsigned int _id;
  unsigned int _window;
  unsigned int _packets_sent, _packets_received;

  bool _sending;

public:
  WindowSender( const unsigned int s_id,
		const unsigned int s_window );

  void tick( Network & net, Receiver & rec, const unsigned int tickno );

  void set_sending( const bool s_sending ) { _sending = s_sending; }
  bool sending( void ) { return _sending; }
};

#endif
