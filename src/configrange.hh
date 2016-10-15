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

  Range( RemyBuffers::Range range )
    : low( range.low() ),
      high( range.high() ),
      incr ( range.incr() )
    {}
  bool isOne( void ) const
  {
    return ( ( low == high ) || ( incr == 0 ) );
  }
};

class ConfigRange
{
public:
  Range link_ppt;
  Range rtt;
  Range mean_on_duration;
  Range mean_off_duration;
  Range num_senders;
  Range buffer_size;
  unsigned int simulation_ticks;
  Range stochastic_loss_rate;

  ConfigRange( void );
  ConfigRange( RemyBuffers::ConfigRange configrange );
  RemyBuffers::ConfigRange DNA( void ) const;
};

#endif  // CONFIG_RANGE_HH
