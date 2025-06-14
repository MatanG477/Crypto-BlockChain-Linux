# Compiler and flags
CXX = g++
CXXFLAGS = -Wall -std=c++11 -I./include

# Shared library target
LIBINFRA = libinfra.so

# Source files
INFRA_SRC = infra.cpp
INFRA_HDR = infra.h

# All .cpp files except infra.cpp (main programs)
SRCS = $(filter-out $(INFRA_SRC), $(wildcard *.cpp))
OUTS = $(SRCS:.cpp=.out)

# Default target: build all
all: $(OUTS)

# Build the shared library
$(LIBINFRA): $(INFRA_SRC) $(INFRA_HDR)
	$(CXX) -shared -fPIC $(INFRA_SRC) -o $(LIBINFRA)

# Build each executable from its .cpp file, link with the shared library
%.out: %.cpp $(LIBINFRA)
	$(CXX) $< -L. -linfra -Wl,-rpath=$(shell pwd) -o $@ $(CXXFLAGS)

# Clean rule
.PHONY: clean
clean:
	rm -f *.out *.so
