AM_CPPFLAGS = -I../protobufs
AM_CXXFLAGS = $(PICKY_CXXFLAGS)
LDADD = ../protobufs/libremyprotos.a -lm $(protobuf_LIBS)

bin_PROGRAMS = remy rat-runner

common_source = delay.hh evaluator.cc evaluator.hh                     \
	exponential.hh link.hh link-templates.cc                       \
	memory.cc memory.hh memoryrange.cc memoryrange.hh              \
	network.cc network.hh packet.hh poisson.hh                     \
	random.cc random.hh rat.cc rat.hh rat-templates.cc             \
	receiver.cc receiver.hh sendergang.cc sendergang.hh            \
    sendergangofgangs.cc sendergangofgangs.hh                      \
	utility.hh whisker.cc whisker.hh whiskertree.cc whiskertree.hh \
	aimd-templates.cc aimd.cc aimd.hh                              \
        configrange.hh configrange.cc

remy_SOURCES = $(common_source) remy.cc ratbreeder.cc ratbreeder.hh 

rat_runner_SOURCES = $(common_source) rat-runner.cc
