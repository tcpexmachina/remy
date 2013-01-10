source = link.cc receiver.cc window-tester.cc random.cc window-sender.cc sendergang.cc rat.cc rat-tester.cc window-sender-templates.cc rat-templates.cc link-templates.cc network.cc emil-tester.cc emil.cc emil-templates.cc
objects = link.o receiver.o random.o window-sender.o sendergang.o rat.o window-sender-templates.o rat-templates.o link-templates.o network.o emil.o emil-templates.o
executables = window-tester rat-tester emil-tester
CXX = g++
LANGFLAGS = -std=c++0x
CXXFLAGS = -g -O3 $(LANGFLAGS) -ffast-math -pedantic -Werror -Wall -Wextra -Weffc++ -fno-default-inline -pipe
LIBS = -lm

all: $(executables)

window-tester: window-tester.o $(objects)
	$(CXX) $(CXXFLAGS) -o $@ $+ $(LIBS)

rat-tester: rat-tester.o $(objects)
	$(CXX) $(CXXFLAGS) -o $@ $+ $(LIBS)

emil-tester: emil-tester.o $(objects)
	$(CXX) $(CXXFLAGS) -o $@ $+ $(LIBS)

%.o: %.cc
	$(CXX) $(CXXFLAGS) -c -o $@ $<

-include depend

depend: $(source)
	$(CXX) $(LANGFLAGS) $(INCLUDES) -MM $(source) > depend

.PHONY: clean
clean:
	-rm -f $(executables) depend *.o *.rpo
