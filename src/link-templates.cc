#include <utility>

#include "link.hh"

using namespace std;

template <class NextHop>
void Link::tick( NextHop & next, const double & tickno )
{
  while( !buffer_.empty() and buffer_.front().first <= tickno ) {
    assert( buffer_.front().first == tickno );

    next.accept( buffer_.front().second, tickno );
    buffer_.pop();
  }
}
