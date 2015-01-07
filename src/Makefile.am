AM_CPPFLAGS = -I../protobufs -I../graph $(GL_CFLAGS) $(GLFW_CFLAGS) $(GLEW_CFLAGS) $(GLU_CFLAGS) $(PANGOCAIRO_CFLAGS)
AM_CXXFLAGS = $(PICKY_CXXFLAGS)
LDADD = ../protobufs/libremyprotos.a -ljemalloc -lm $(protobuf_LIBS)

bin_PROGRAMS = remy rat-runner scoring-example ratatouille-signals stability-graph

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

scoring_example_SOURCES = $(common_source) scoring-example.cc

ratatouille_LDADD = ../graph/libremygraph.a $(GL_LIBS) $(GLFW_LIBS) $(GLEW_LIBS) $(GLU_LIBS) $(PANGOCAIRO_LIBS) $(LDADD)

ratatouille_SOURCES = $(common_source) ratatouille.cc fader.hh fader.cc fader-templates.cc

ratatouille_signals_LDADD = $(ratatouille_LDADD)

ratatouille_signals_SOURCES = $(common_source) ratatouille-signals.cc fader.hh fader.cc fader-templates.cc

stability_graph_LDADD = $(ratatouille_LDADD)

stability_graph_SOURCES = $(common_source) stability-graph.cc fader.hh fader.cc fader-templates.cc