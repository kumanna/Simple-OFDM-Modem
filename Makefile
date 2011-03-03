all: simulate

CXXFLAGS += -ggdb `itpp-config --cflags`
LDFLAGS +=  `itpp-config --libs`


simulate: simulate.cpp parameters.hpp

.PHONY: clean

clean:
	$(RM) simulate
