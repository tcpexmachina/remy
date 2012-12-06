#ifndef RANDOM_HH
#define RANDOM_HH

#include <boost/random/mersenne_twister.hpp>

extern boost::random::mt19937 & get_generator( void );

#endif
