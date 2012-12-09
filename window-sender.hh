#ifndef WINDOWSENDER_HH
#define WINDOWSENDER_HH

#include "poisson.hh"
#include "network.hh"

class WindowSender
{
private:
  unsigned int _id;
  unsigned int _window;
  unsigned int _packets_sent;
  unsigned int _packets_received;
  uint64_t _total_delay;

public:
  WindowSender( const unsigned int s_id, const unsigned int s_window );

  void tick( Network & net, Receiver & rec, const unsigned int tickno );

  bool operator<( const WindowSender & other ) const { return _id < other._id; }
};

#endif
