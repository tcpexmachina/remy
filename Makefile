source = network.cc receiver.cc network-tester.cc random.cc window-sender.cc sendergang.cc delay.cc
objects = network.o receiver.o random.o window-sender.o sendergang.o delay.o
executables = network-tester
CXX = g++
LANGFLAGS = -std=c++0x
CXXFLAGS = -g -O3 $(LANGFLAGS) -ffast-math -pedantic -Werror -Wall -Wextra -Weffc++ -fno-default-inline -pipe
LIBS = -lm

all: $(executables)

network-tester: network-tester.o $(objects)
	$(CXX) $(CXXFLAGS) -o $@ $+ $(LIBS)

%.o: %.cc
	$(CXX) $(CXXFLAGS) -c -o $@ $<

-include depend

depend: $(source)
	$(CXX) $(LANGFLAGS) $(INCLUDES) -MM $(source) > depend

.PHONY: clean
clean:
	-rm -f $(executables) depend *.o *.rpo
