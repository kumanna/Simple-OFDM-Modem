all: simulate

OBJS = simulate.p receive.o

CXXFLAGS += -ggdb `itpp-config --cflags`
LDFLAGS +=  `itpp-config --libs`


simulate: simulate.o receive.o
simulate.o: parameters.hpp simulate.cpp
receive.o: receive.cpp parameters.hpp receive.hpp


.PHONY: clean

clean:
	$(RM) simulate $(OBJS)
