#ifndef FADER_HH
#define FADER_HH

#include <string>
#include <deque>
#include <array>

class Fader
{
  int fd_;
  std::deque< uint8_t > buffer_;

  std::array< uint8_t, 94 > physical_values_;

public:
  Fader( const std::string & filename );

  void update( void );
};

#endif /* FADER_HH */
