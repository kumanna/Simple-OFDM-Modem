all: simulate

CXXFLAGS += -ggdb `itpp-config --cflags`
LDFLAGS +=  `itpp-config --libs`


simulate: simulate.cpp

.PHONY: clean

clean:
	$(RM) simulate
