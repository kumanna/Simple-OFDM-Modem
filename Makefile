all: simulate

OBJS = simulate.o receive.o transmit.o

CXXFLAGS += -ggdb `itpp-config --cflags`
LDFLAGS +=  `itpp-config --libs`


simulate: $(OBJS)
simulate.o: simulate.cpp parameters.hpp receive.hpp transmit.hpp
receive.o: receive.cpp parameters.hpp receive.hpp
transmit.o: transmit.cpp transmit.hpp parameters.hpp

.PHONY: clean

clean:
	$(RM) simulate $(OBJS)
