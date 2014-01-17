#ifndef ROUTER_HH_
#define ROUTER_HH_

#include <vector>
#include <queue>
#include "link.hh"
#include "packet.hh"
#include "receiver.hh"

class Router
{
private:
  Link & next_;
  Receiver & rec_;

public:
  Router(Link & next, Receiver & rec) : next_( next ), rec_( rec ) {}

  void accept( const Packet & p, const double & tickno ) noexcept;
};

#endif // ROUTER_HH_
