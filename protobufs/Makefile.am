source = dna.proto problem.proto answer.proto

AM_CPPFLAGS = $(protobuf_CFLAGS)

SUFFIXES = .proto .pb.cc

.proto.pb.cc:
	$(AM_V_GEN)$(PROTOC) --cpp_out=. -I$(srcdir) $<

noinst_LIBRARIES = libremyprotos.a

libremyprotos_a_SOURCES = $(source)
nodist_libremyprotos_a_SOURCES = $(source:.proto=.pb.cc) $(source:.proto=.pb.h)

BUILT_SOURCES = $(source:.proto=.pb.cc)
CLEANFILES = $(source:.proto=.pb.cc) $(source:.proto=.pb.h)
