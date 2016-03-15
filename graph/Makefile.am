AM_CPPFLAGS = $(CXX11_FLAGS) -I$(srcdir)/../src -I../protobufs $(GL_CFLAGS) $(GLFW_CFLAGS) $(GLEW_CFLAGS) $(GLU_CFLAGS) $(PANGOCAIRO_CFLAGS) $(GTKMM_CFLAGS)

noinst_LIBRARIES = libfader.a
libfader_a_SOURCES = fader.cc
libfader_a_CXXFLAGS = $(PICKY_CXXFLAGS) -Wno-effc++ -Wno-deprecated-declarations

noinst_LIBRARIES += libgraphics.a
libgraphics_a_CXXFLAGS = $(PICKY_CXXFLAGS)

libgraphics_a_SOURCES = gl_objects.hh gl_objects.cc \
	display.hh display.cc \
	image.hh image.cc \
	cairo_objects.hh cairo_objects.cc \
	graph.hh graph.cc \
	fader.hh fader-templates.cc bb100.hh

bin_PROGRAMS =

if BUILD_GRAPH
bin_PROGRAMS += ratatouille
ratatouille_LDADD = libfader.a libgraphics.a ../src/libremycore.a ../protobufs/libremyprotos.a -lm $(protobuf_LIBS) $(GL_LIBS) $(GLFW_LIBS) $(GLEW_LIBS) $(GLU_LIBS) $(PANGOCAIRO_LIBS) $(GTKMM_LIBS)
ratatouille_CXXFLAGS = $(PICKY_CXXFLAGS)
ratatouille_SOURCES = ratatouille.cc 
AM_CXXFLAGS = $(PICKY_CXXFLAGS)
endif

