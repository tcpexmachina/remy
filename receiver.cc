#include <assert.h>

#include "receiver.hh"

Receiver::Receiver()
  : _collector(),
    _free_src_numbers()
{
}

void Receiver::accept( Packet && p, const unsigned int tickno ) noexcept
{
  assert( p.src < _collector.size() );

  if ( p.flow_id == _collector[ p.src ].first ) {
    p.tick_received = tickno;
    _collector[ p.src ].second.push_back( std::move( p ) );
  }
}

std::vector< Packet > Receiver::collect( const unsigned int src )
{
  assert( src < _collector.size() );

  auto ret( std::move( _collector[ src ].second ) );
  assert( _collector[ src ].second.empty() );

  return ret;
}

std::pair< unsigned int, unsigned int > Receiver::new_src( void )
{
  if ( _free_src_numbers.empty() ) {
    /* need to resize collector */
    _collector.resize( _collector.size() + 1 );
    _free_src_numbers.push( _collector.size() - 1 );
  }

  /* get source slot */
  const int new_src_number = _free_src_numbers.top();
  _free_src_numbers.pop();

  /* increment flow id */
  _collector[ new_src_number ].first++;

  /* clear collector */
  _collector[ new_src_number ].second.clear();

  return std::make_pair( new_src_number, _collector[ new_src_number ].first );
}

void Receiver::free_src( const unsigned int src )
{
  _free_src_numbers.push( src );
  _collector[ src ].first++;

  /* make sure we don't double-free */
  assert( _collector[ src ].first % 2 == 0 );
}
