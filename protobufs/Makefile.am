source = dna.proto problem.proto answer.proto simulationresults.proto

AM_CPPFLAGS = $(CXX11_FLAGS) $(protobuf_CFLAGS)

SUFFIXES = .proto .pb.cc _pb2.py

.proto.pb.cc:
	$(AM_V_GEN)$(PROTOC) --cpp_out=. -I$(srcdir) $<

.proto_pb2.py:
	$(AM_V_GEN)$(PROTOC) --python_out=. -I$(srcdir) $<

noinst_LIBRARIES = libremyprotos.a
noinst_SCRIPTS = $(source:.proto=_pb2.py)

libremyprotos_a_SOURCES = $(source)
nodist_libremyprotos_a_SOURCES = $(source:.proto=.pb.cc) $(source:.proto=.pb.h)

BUILT_SOURCES = $(source:.proto=.pb.cc)
CLEANFILES = $(source:.proto=.pb.cc) $(source:.proto=.pb.h) $(source:.proto=_pb2.py) $(source:.proto=_pb2.pyc)
