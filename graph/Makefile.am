AM_CPPFLAGS = $(GL_CFLAGS) $(GLFW_CFLAGS) $(GLEW_CFLAGS) $(GLU_CFLAGS) $(PANGOCAIRO_CFLAGS)
AM_CXXFLAGS = $(PICKY_CXXFLAGS) $(NODEBUG_CXXFLAGS)
#LDADD = $(GL_LIBS) $(GLFW_LIBS) $(GLEW_LIBS) $(GLU_LIBS) $(PANGOCAIRO_LIBS)
#bin_PROGRAMS = glfun

noinst_LIBRARIES = libremygraph.a

libremygraph_a_SOURCES = gl_objects.hh gl_objects.cc \
	display.hh display.cc \
	image.hh image.cc \
	cairo_objects.hh cairo_objects.cc \
	graph.hh graph.cc
