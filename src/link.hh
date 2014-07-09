#ifndef LINK_HH
#define LINK_HH

#include <queue>

#include "packet.hh"
#include "delay.hh"

class Link
{
protected:
  unsigned int limit_;
  std::queue< std::pair< double, Packet > > buffer_;

public:
  Link( const unsigned int s_limit = std::numeric_limits<unsigned int>::max() ) 
    : limit_( s_limit ), buffer_() {}

  double next_event_time( const double & tickno ) const {
    if( buffer_.empty() ) {
      return std::numeric_limits<double>::max();
    } else {
      if( tickno > buffer_.front().first ) {
        fprintf( stderr, "Error, tickno = %f but packet should have been released at time %f\n",
		 tickno, buffer_.front().first );
	assert( false );
      }

      return buffer_.front().first;
    }
  }
  
  /* is abstract base class */
  virtual void accept( const Packet & p, const double & tickno ) noexcept = 0;
  
  template <class NextHop>
  void tick( NextHop & next, const double & tickno );
};

class IsochronousLink : public Link {
private:
  double rate_;

public:
  IsochronousLink( const double rate ) 
  : Link(), rate_( rate ) {}

  void accept( const Packet & p, const double & tickno ) noexcept override;
};

class TraceLink : public Link {
private:
  std::queue<double> trace_;

public:
  TraceLink( const std::queue< double > trace ) 
    : Link(), trace_( trace ) {}

  void accept( const Packet & p, const double & tickno ) noexcept override;
};

#endif
