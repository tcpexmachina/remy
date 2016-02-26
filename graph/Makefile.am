AM_CPPFLAGS = -I$(srcdir)/../src -I../protobufs $(GL_CFLAGS) $(GLFW_CFLAGS) $(GLEW_CFLAGS) $(GLU_CFLAGS) $(PANGOCAIRO_CFLAGS) $(GTKMM_CFLAGS)
AM_CXXFLAGS = $(PICKY_CXXFLAGS) $(NODEBUG_CXXFLAGS)
LDADD = ../protobufs/libremyprotos.a -lm $(protobuf_LIBS) ../src/libremycore.a $(GL_LIBS) $(GLFW_LIBS) $(GLEW_LIBS) $(GLU_LIBS) $(PANGOCAIRO_LIBS) $(GTKMM_LIBS)

common_source = gl_objects.hh gl_objects.cc \
	display.hh display.cc \
	image.hh image.cc \
	cairo_objects.hh cairo_objects.cc \
	graph.hh graph.cc \
	fader.hh fader.cc fader-templates.cc bb100.hh

bin_PROGRAMS =

if BUILD_GRAPH
bin_PROGRAMS += ratatouille
ratatouille_SOURCES = $(common_source) ratatouille.cc 
endif

