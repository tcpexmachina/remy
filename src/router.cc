#include <limits>
#include "router.hh"

void Router::accept( const Packet & p, const double & tickno ) noexcept
{
  assert (p.src != 1);
  /* Only first and third flows need router arbitration */
  if (p.src == 0) {
    rec_.accept( p, tickno );
  } else if (p.src == 2) {
    next_.accept( p, tickno );
  }
}
