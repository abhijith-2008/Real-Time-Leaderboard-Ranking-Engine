CXX      ?= g++
CXXFLAGS ?= -std=c++17 -O2 -Wall -Wextra -Iinclude -Itests

LIB_SRC  := $(filter-out src/main.cpp,$(wildcard src/*.cpp))
LIB_OBJ  := $(patsubst src/%.cpp,build/%.o,$(LIB_SRC))
TESTS    := $(patsubst tests/%.cpp,build/%,$(wildcard tests/*.cpp))
BENCHES  := $(patsubst benchmarks/%.cpp,build/%,$(wildcard benchmarks/*.cpp))

all: build/leaderboard

build:
	mkdir -p build

build/%.o: src/%.cpp $(wildcard include/*.h) | build
	$(CXX) $(CXXFLAGS) -c $< -o $@

build/leaderboard: build/main.o $(LIB_OBJ)
	$(CXX) $(CXXFLAGS) $^ -o $@

build/main.o: src/main.cpp $(wildcard include/*.h) | build
	$(CXX) $(CXXFLAGS) -c $< -o $@

build/%: tests/%.cpp $(LIB_OBJ) $(wildcard include/*.h tests/*.h) | build
	$(CXX) $(CXXFLAGS) $< $(LIB_OBJ) -o $@

build/%: benchmarks/%.cpp $(LIB_OBJ) $(wildcard include/*.h tests/*.h) | build
	$(CXX) $(CXXFLAGS) $< $(LIB_OBJ) -o $@

# Runs every .cpp file in tests/ (each must have its own main).
test: $(TESTS)
	for t in $(TESTS); do echo "== $$t"; ./$$t || exit 1; done

bench: $(BENCHES)
	for b in $(BENCHES); do echo "== $$b"; ./$$b; done

clean:
	rm -rf build

.PHONY: all test bench clean
