#ifndef FADER_HH
#define FADER_HH

#include <string>
#include <deque>

class Fader
{
  int fd_;
  std::deque< uint8_t > buffer_;

public:
  Fader( const std::string & filename );

  void update( void );
};

#endif /* FADER_HH */
