#ifndef RANDOM_HH
#define RANDOM_HH

#include <boost/random/mersenne_twister.hpp>

typedef boost::random::mt19937 PRNG;

extern PRNG & global_PRNG();

#endif
