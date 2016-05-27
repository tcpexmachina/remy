[![Build Status](https://travis-ci.org/tcpexmachina/remy.svg?branch=master)](https://travis-ci.org/tcpexmachina/remy)

Remy: TCP ex Machina (computer-generated congestion control)
============================================================

Remy is an optimization tool to develop new TCP congestion-control
schemes, given prior knowledge about the network it will encounter
and an objective to optimize for.

It is described further at the Web site for [TCP ex
Machina](http://web.mit.edu/remy). A research paper on Remy was
published at the ACM SIGCOMM 2013 annual conference.

Basic usage:

* Remy requires a C++11 compiler to compile, e.g. gcc 4.6 or
  contemporary clang++. You will also need the Google
  protobuf-compiler and the Boost C++ libraries.

* From the version-control repository checkout, run `./autogen.sh`,
  `./configure`, and `make` to build Remy.

* Run `./remy` to design a RemyCC (congestion-control algorithm) for
  the default scenario, with link speed drawn uniformly between 10 and
  20 Mbps, minRTT drawn uniformly between 100 and 200 ms, the maximum
  degree of multiplexing drawn uniformly between 1 and 32, and each
  sender "on" for an exponentially-distributed amount of time (mean 5
  s) and off for durations drawn from the same distribution.

* Use the `cf=` argument to pass in a specific configuration file into Remy.
  To create this configuration protobuf, run `./configuration`, with the 
  desired specifications about the range of networks to train over. The 
  configuration executable takes in information about link speed, minRTT, 
  on and off times, number of senders, buffer size, utility function loss 
  penalty and stochastic loss rate.

* Use the of= argument to have Remy save its RemyCCs to disk. It will
  save every step of the iteration.

* Use the if= argument to get Remy to read previous RemyCCs as the
  starting point for optimization.

* The `sender-runner` tool will execute saved RemyCCs. The filename
  should be set with a `if=` argument. It also accepts `link=` to set
  the link speed (in packets per millisecond), `rtt=` to set the RTT,
  and `nsrc=` to set the maximum degree of multiplexing.

If you have any questions, please visit [Remy's Web
site](http://web.mit.edu/remy) or e-mail `remy at mit dot edu`.

-- Keith Winstein
