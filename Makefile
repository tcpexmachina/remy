source = link.cc receiver.cc window-tester.cc random.cc window-sender.cc sendergang.cc rat.cc rat-tester.cc window-sender-templates.cc rat-templates.cc link-templates.cc network.cc emil-tester.cc emil.cc emil-templates.cc whisker.cc memory.cc memoryrange.cc evaluator.cc dna.pb.cc ratbreeder.cc
objects = link.o receiver.o random.o window-sender.o sendergang.o rat.o window-sender-templates.o rat-templates.o link-templates.o network.o emil.o emil-templates.o whisker.o memory.o memoryrange.o evaluator.o whiskertree.o dna.pb.o ratbreeder.o
executables = window-tester rat-tester emil-tester
CXX = g++
LANGFLAGS = -std=c++0x
CXXFLAGS = -g -O3 $(LANGFLAGS) -ffast-math -pedantic -Werror -Wall -Wextra -Weffc++ -fno-default-inline -pipe
LIBS = -lm -lprotobuf
PROTOC = protoc

all: $(executables)

window-tester: window-tester.o $(objects)
	$(CXX) $(CXXFLAGS) -o $@ $+ $(LIBS)

rat-tester: rat-tester.o $(objects)
	$(CXX) $(CXXFLAGS) -o $@ $+ $(LIBS)

emil-tester: emil-tester.o $(objects)
	$(CXX) $(CXXFLAGS) -o $@ $+ $(LIBS)

%.pb.o: %.pb.cc
	$(CXX) $(CXXFLAGS) -Wno-effc++ -c -o $@ $<

%.o: %.cc
	$(CXX) $(CXXFLAGS) -c -o $@ $<

-include depend

%.pb.cc: %.proto
	$(PROTOC) --cpp_out=. $<

depend: $(source)
	$(CXX) $(LANGFLAGS) $(INCLUDES) -MM $(source) > depend

.PHONY: clean
clean:
	-rm -f $(executables) depend *.o *.rpo *.pb.cc *.pb.h
