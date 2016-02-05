source = dna.proto problem.proto answer.proto simulationresults.proto

AM_CPPFLAGS = $(CXX11_FLAGS) $(protobuf_CFLAGS)

SUFFIXES = .proto .pb.cc _pb2.py

.proto.pb.cc:
	$(AM_V_GEN)$(PROTOC) --cpp_out=. -I$(srcdir) $<

.proto_pb2.py:
	$(AM_V_GEN)$(PROTOC) --python_out=../scripts/ -I$(srcdir) $<

noinst_LIBRARIES = libremyprotos.a

libremyprotos_a_SOURCES = $(source)
nodist_libremyprotos_a_SOURCES = $(source:.proto=.pb.cc) $(source:.proto=.pb.h)

BUILT_SOURCES = $(source:.proto=.pb.cc) $(source:.proto=_pb2.py)
CLEANFILES = $(source:.proto=.pb.cc) $(source:.proto=.pb.h) $(source:.proto=_pb2.py)
