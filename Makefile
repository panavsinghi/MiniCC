CXX ?= g++
CXXFLAGS ?= -std=c++17 -O2 -Wall -Wextra -Werror -Isrc

all: minicc tests/run_tests

minicc: src/main.cpp $(wildcard src/*.hpp)
	$(CXX) $(CXXFLAGS) -o $@ src/main.cpp

tests/run_tests: tests/tests.cpp $(wildcard src/*.hpp)
	$(CXX) $(CXXFLAGS) -o $@ tests/tests.cpp

test: tests/run_tests
	./tests/run_tests

clean:
	rm -f minicc tests/run_tests

.PHONY: all test clean
