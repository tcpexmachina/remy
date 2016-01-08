#ifndef CONFIG_RANGE_HH
#define CONFIG_RANGE_HH

#include "dna.pb.h"
class Range
{
public:
  double low;
  double high;
  double incr;
  Range ( void ) 
    : low( 1 ),
      high ( 1 ),
      incr ( 0 )
    {}
  Range( double lo, double hi, double inc )
    : low( lo ),
      high( hi ),
      incr( inc )
  {}
  Range & set_low( const double lo ) { low = lo; return *this; }
  Range & set_high( const double hi ) { high = hi; return *this; }
  Range & set_incr( const double inc ) { incr = inc; return *this; }
};

class IntRange
{
  unsigned int low;
  unsigned int  high;
  unsigned int  incr;
  IntRange( unsigned int lo, unsigned int  hi, unsigned int inc )
    : low( lo ),
      high( hi ),
      incr( inc )
  {}
  IntRange ( void )
    : low( 1 ),
      high ( 1 ),
      incr ( 0 )
    {}
  IntRange & set_low( const unsigned int lo ) { low = lo; return *this; }
  IntRange & set_high( const unsigned int hi ) { high = hi; return *this; }
  IntRange & set_incr( const unsigned int inc ) { incr = inc; return *this; }
};
class ConfigRange
{
public:
  Range link_ppt = Range();
  Range rtt = Range();
  Range mean_on_duration = Range();
  Range mean_off_duration = Range();
  Range num_senders = Range();
  Range buffer_size = Range();
  /*std::pair< double, double > link_packets_per_ms { 1, 2 };
  std::pair< double, double > rtt_ms { 100, 200 };
  unsigned int min_senders { 1 };
  unsigned int max_senders { 16 };
  double mean_on_duration { 1000 };
  double mean_off_duration { 1000 };
  bool lo_only { false }; */
  
  RemyBuffers::ConfigRange DNA( void ) const;
};

#endif  // CONFIG_RANGE_HH
