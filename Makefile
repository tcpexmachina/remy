source = link.cc receiver.cc window-tester.cc random.cc window-sender.cc sendergang.cc rat.cc rat-tester.cc window-sender-templates.cc rat-templates.cc link-templates.cc network.cc emil-tester.cc emil.cc emil-templates.cc whisker.cc memory.cc memoryrange.cc evaluator.cc dna.pb.cc ratbreeder.cc rat-runner.cc
objects = link.o receiver.o random.o window-sender.o sendergang.o rat.o window-sender-templates.o rat-templates.o link-templates.o network.o emil.o emil-templates.o whisker.o memory.o memoryrange.o evaluator.o whiskertree.o dna.pb.o ratbreeder.o
executables = window-tester rat-tester emil-tester rat-runner
targetlibs = libremy.so
CXX = g++
LANGFLAGS = -std=c++0x
CXXFLAGS = -g -O3 $(LANGFLAGS) -ffast-math -pedantic -Werror -Wall -Wextra -Weffc++ -fno-default-inline -pipe -fPIC
LIBS = -L. -lremy -lm -lprotobuf
PROTOC = protoc

all: $(executables) $(targetlibs)

libremy.so: $(objects)
	$(CXX) -shared -Wl,-soname,libremy.so -o $@ $+

window-tester: window-tester.o $(targetlibs)
	$(CXX) $(CXXFLAGS) -o $@ window-tester.o $(LIBS)

rat-tester: rat-tester.o $(targetlibs)
	$(CXX) $(CXXFLAGS) -o $@ rat-tester.o $(LIBS)

rat-runner: rat-runner.o $(targetlibs)
	$(CXX) $(CXXFLAGS) -o $@ rat-runner.o $(LIBS)

emil-tester: emil-tester.o $(targetlibs)
	$(CXX) $(CXXFLAGS) -o $@ emil-tester.o $(LIBS)

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
