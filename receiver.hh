#ifndef RECEIVER_HH
#define RECEIVER_HH

#include <vector>

#include "packet.hh"

class Receiver
{
private:
  std::vector< std::vector< Packet > > _collector;
  void autosize( const unsigned int index );

public:
  Receiver();

  void accept( Packet && p, const unsigned int tickno ) noexcept;
  std::vector< Packet > collect( const unsigned int src );
};

#endif
