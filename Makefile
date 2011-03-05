all: simulate generate_frame

OBJS = simulate.o receive.o transmit.o unwrap.o

CXXFLAGS += -ggdb `itpp-config --cflags` -O0 -Wall
LDFLAGS +=  `itpp-config --libs`


simulate: $(OBJS)
generate_frame: generate_frame.o transmit.o
generate_frame.o: generate_frame.cpp parameters.hpp receive.hpp \
 transmit.hpp
receive.o: receive.cpp parameters.hpp receive.hpp unwrap.hpp
simulate.o: simulate.cpp parameters.hpp receive.hpp transmit.hpp
transmit.o: transmit.cpp transmit.hpp parameters.hpp
unwrap.o: unwrap.cpp unwrap.hpp

.PHONY: clean

clean:
	$(RM) simulate $(OBJS)
