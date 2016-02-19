AM_CPPFLAGS = -I../protobufs
AM_CXXFLAGS = $(PICKY_CXXFLAGS)
LDADD = ../protobufs/libremyprotos.a -ljemalloc -lm $(protobuf_LIBS)

bin_PROGRAMS = remy rat-runner scoring-example configuration inspect-config buffer-config

common_source = delay.hh evaluator.cc evaluator.hh                     \
	exponential.hh link.hh link-templates.cc                       \
	memory.cc memory.hh memoryrange.cc memoryrange.hh              \
	network.cc network.hh packet.hh poisson.hh                     \
	random.cc random.hh rat.cc rat.hh rat-templates.cc             \
	receiver.cc receiver.hh sendergang.cc sendergang.hh            \
        sendergangofgangs.cc sendergangofgangs.hh                      \
	utility.hh whisker.cc whisker.hh whiskertree.cc whiskertree.hh \
	aimd-templates.cc aimd.cc aimd.hh                              \
        configrange.hh configrange.cc configvector.hh configvector.cc


remy_SOURCES = $(common_source) remy.cc ratbreeder.cc ratbreeder.hh 

rat_runner_SOURCES = $(common_source) rat-runner.cc

scoring_example_SOURCES = $(common_source) scoring-example.cc

configuration_SOURCES = input-configrange.cc

inspect_config_SOURCES = $(common_source) inspect-configrange.cc

buffer_config_SOURCES = buffer-inputrange.cc
