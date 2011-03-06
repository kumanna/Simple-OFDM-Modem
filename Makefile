all: simulate generate_frame read_frame

OBJS = simulate.o receive.o transmit.o unwrap.o

CXXFLAGS += -ggdb `itpp-config --cflags` -O0 -Wall
LDFLAGS +=  `itpp-config --libs`


simulate: $(OBJS)
	g++ -o simulate $(OBJS) $(LDFLAGS)
generate_frame: generate_frame.o transmit.o
	g++ -o generate_frame generate_frame.o transmit.o $(LDFLAGS)
read_frame: read_frame.o transmit.o receive.o
	g++ -o read_frame read_frame.o transmit.o receive.o $(LDFLAGS)
generate_frame.o: generate_frame.cpp parameters.hpp receive.hpp transmit.hpp
read_frame.o: read_frame.cpp parameters.hpp receive.hpp transmit.hpp
receive.o: receive.cpp parameters.hpp receive.hpp unwrap.hpp
simulate.o: simulate.cpp parameters.hpp receive.hpp transmit.hpp
transmit.o: transmit.cpp transmit.hpp parameters.hpp
unwrap.o: unwrap.cpp unwrap.hpp


.PHONY: clean

clean:
	$(RM) simulate $(OBJS)
