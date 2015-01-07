#ifndef NETWORK_HH
#define NETWORK_HH

#include <string>

#include "sendergangofgangs.hh"
#include "link.hh"
#include "delay.hh"
#include "receiver.hh"
#include "random.hh"
#include "answer.pb.h"
#include "memory.hh"

static const double EPSILON = 0.0001;

struct StatePoint
{
  Memory::DataType _sewma;
  Memory::DataType _rewma;
  Memory::DataType _rttr;
  Memory::DataType _slow_rewma;
  
  long unsigned int _buffer_size;

  double _tickno;

  StatePoint( void )
  : _sewma( 0.0 ), _rewma( 0.0 ),
    _rttr( 0.0 ), _slow_rewma( 0.0 ),
    _buffer_size( 0 ), _tickno( 0.0 )
  {
  }
  
  StatePoint( const double sewma, const double rewma,
              const double rttr, const double slow_rewma,
              const long unsigned int buffer,
              const double tickno )
    :  _sewma( sewma ), _rewma( rewma ),
       _rttr( rttr ), _slow_rewma( slow_rewma ),
       _buffer_size( buffer ), _tickno( tickno )
  {
  }
  
  bool operator==( const StatePoint & other ) const 
  { 
    return ((fabs(_rewma - other._rewma) <  EPSILON)
            && (fabs(_sewma - other._sewma) < EPSILON)
            && (fabs(_rttr - other._rttr) < EPSILON)
            && (fabs(_slow_rewma - other._slow_rewma) < EPSILON)
            && (_buffer_size == other._buffer_size));
  }
  
  std::string str( void ) const {
    char tmp[ 256 ];
    snprintf( tmp, 256, "sewma=%f, rewma=%f, rttr=%f, slowrewma=%f, buffer=%lu, tickno %f", 
              _sewma, _rewma, _rttr, _slow_rewma, _buffer_size, _tickno );
    return tmp;
  }
};

class NetConfig
{
public:
  double mean_on_duration, mean_off_duration;
  unsigned int num_senders;
  double link_ppt;
  double delay;

  NetConfig( void )
    : mean_on_duration( 5000.0 ),
      mean_off_duration( 5000.0 ),
      num_senders( 8 ),
      link_ppt( 1.0 ),
      delay( 150 )
  {}

  NetConfig( const RemyBuffers::NetConfig & dna )
    : mean_on_duration( dna.mean_on_duration() ),
      mean_off_duration( dna.mean_off_duration() ),
      num_senders( dna.num_senders() ),
      link_ppt( dna.link_ppt() ),
      delay( dna.delay() )
  {}
  
  NetConfig & set_link_ppt( const double s_link_ppt ) { link_ppt = s_link_ppt; return *this; }
  NetConfig & set_delay( const double s_delay ) { delay = s_delay; return *this; }
  NetConfig & set_num_senders( const unsigned int n ) { num_senders = n; return *this; }
  NetConfig & set_on_duration( const double & duration ) { mean_on_duration = duration; return *this; }
  NetConfig & set_off_duration( const double & duration ) { mean_off_duration = duration; return *this; }

  RemyBuffers::NetConfig DNA( void ) const
  {
      RemyBuffers::NetConfig ret;
      ret.set_mean_on_duration( mean_on_duration );
      ret.set_mean_off_duration( mean_off_duration );
      ret.set_num_senders( num_senders );
      ret.set_delay( delay );
      ret.set_link_ppt( link_ppt );

      return ret;
  }

  std::string str( void ) const
  {
    char tmp[ 256 ];
    snprintf( tmp, 256, "mean_on=%f, mean_off=%f, nsrc=%d, link_ppt=%f, delay=%f\n",
	      mean_on_duration, mean_off_duration, num_senders, link_ppt, delay );
    return tmp;
  }
};

template <class Gang1Type, class Gang2Type>
class Network
{
private:
  PRNG & _prng;
  SenderGangofGangs<Gang1Type, Gang2Type> _senders;
  Link _link;
  Delay _delay;
  Receiver _rec;

  double _tickno;

  void tick( void );

  std::deque< StatePoint > _history;
  const unsigned int _max_history;
  StatePoint _start_config;

public:
  Network( const typename Gang1Type::Sender & example_sender1, const typename Gang2Type::Sender & example_sender2, PRNG & s_prng, const NetConfig & config );

  Network( const typename Gang1Type::Sender & example_sender1, PRNG & s_prng, const NetConfig & config );

  void run_simulation( const double & duration );

  void run_simulation_until( const double tick_limit );

  void run_simulation_with_config( const double & duration,
                                   const double & sewma, const double & rewma,
                                   const double & rttr,
                                   const double & slow_rewma,
                                   const unsigned int buffer_size );

  const SenderGangofGangs<Gang1Type, Gang2Type> & senders( void ) const { return _senders; }

  SenderGangofGangs<Gang1Type, Gang2Type> & mutable_senders( void ) { return _senders; }

  std::vector< unsigned int > packets_in_flight( void ) const;

  const double & tickno( void ) const { return _tickno; }

  Link & mutable_link( void ) { return _link; }

  const std::deque< StatePoint > & history ( void ) { return _history; }
};

#endif
