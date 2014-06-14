AM_CPPFLAGS = -I../protobufs -I../src
AM_CXXFLAGS = $(PICKY_CXXFLAGS)
AUTOMAKE_OPTIONS = subdir-objects
LDADD = ../protobufs/libremyprotos.a -lcurl -lm $(protobuf_LIBS)

bin_PROGRAMS = http_tester

http_tester_SOURCES = ../src/http_transmitter.cc ../src/http_transmitter.hh http_tester.cc
