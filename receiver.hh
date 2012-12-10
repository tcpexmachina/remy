#ifndef RECEIVER_HH
#define RECEIVER_HH

#include <vector>
#include "peekable_queue.hh"

#include "packet.hh"

class Receiver
{
private:
  std::vector< std::pair< unsigned int, std::vector< Packet > > > _collector;
  std::priority_queue< unsigned int > _free_src_numbers;

public:
  Receiver();

  void accept( Packet && p, const unsigned int tickno ) noexcept;
  std::vector< Packet > collect( const unsigned int src );

  std::pair< unsigned int, unsigned int > new_src( void );
  void free_src( const unsigned int src );
};

#endif
