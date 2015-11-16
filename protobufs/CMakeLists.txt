cmake_minimum_required(VERSION 2.8 FATAL_ERROR)

find_package( Protobuf REQUIRED ) 
include_directories(${PROTOBUF_INCLUDE_DIR})

PROTOBUF_GENERATE_CPP (
    PROTO_SRCS
    PROTO_HDRS
    answer.proto
    dna.proto
    problem.proto 
)

add_library( remyprotos
 ${PROTO_SRCS} 
 ${PROTO_HDRS} )

target_link_libraries( remyprotos ${EXTRA_LIBS} )
