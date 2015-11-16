cmake_minimum_required(VERSION 2.8 FATAL_ERROR)


ADD_DEFINITIONS(-std=c++11 -pthread -pedantic -O2)

find_package(Protobuf REQUIRED) 
find_library(JemallocLib jemalloc)

SET(COMMON_SOURCES
    evaluator.cc
    link-templates.cc 
	memory.cc  
	memoryrange.cc
	network.cc
	random.cc
	rat.cc
	rat-templates.cc
    receiver.cc
	sendergang.cc
    sendergangofgangs.cc
	whisker.cc
	whiskertree.cc
    aimd-templates.cc
	aimd.cc
    configrange.cc
)

SET(COMMON_HEADERS 
    delay.hh
	evaluator.hh 
    exponential.hh	
    link.hh
    memory.hh
    memoryrange.hh
    network.hh
    packet.hh
    poisson.hh
    random.hh
    rat.hh
    receiver.hh
    sendergang.hh
    sendergangofgangs.hh
    utility.hh
    whisker.hh
    whiskertree.hh
    aimd.hh
    configrange.hh
)
          
SET(REMY_SOURCES ${COMMON_SOURCES} remy.cc ratbreeder.cc)
SET(REMY_HEADERS ${COMMON_HEADERS} ratbreeder.hh)

SET(RAT_RUNNER_SOURCES ${COMMON_SOURCES} rat-runner.cc)
SET(RAT_RUNNER_HEADERS ${COMMON_HEADERS})

SET(SCORING_EXAMPLES_SOURCES ${COMMON_SOURCES} scoring-example.cc)
SET(SCORING_EXAMPLES_HEADERS ${COMMON_HEADERS})

include_directories(${CMAKE_CURRENT_BINARY_DIR}/../protobufs)

add_executable(remy ${REMY_SOURCES} ${REMY_HEADERS})
add_executable(rat-runner ${RAT_RUNNER_SOURCES} ${RAT_RUNNER_HEADERS})
add_executable(scoring-example ${SCORING_EXAMPLES_SOURCES} ${SCORING_EXAMPLES_HEADERS})

SET(LIBREMYPROTOS ${CMAKE_CURRENT_BINARY_DIR}/../protobufs/libremyprotos.a)
SET(LIBS 
    ${JemallocLib}
    ${LIBREMYPROTOS}
    ${PROTOBUF_LIBRARIES}
)

target_link_libraries(remy ${LIBS})
target_link_libraries(rat-runner ${LIBS})
target_link_libraries(scoring-example ${LIBS})
