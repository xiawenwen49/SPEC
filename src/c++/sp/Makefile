CXX = g++
CXXFLAGS = -std=c++11
INC = -I$(HOME)/workspace/ConstrainedSP/src/c++/ -I$(HOME)/workspace/ConstrainedSP/src/c++/third_libs/
OPT = -O3
THREAD = -pthread
DEBUG = -g2

all: bin bin/main bin/build_index

bin:
	mkdir -p bin

bin/main: main.cpp
	$(CXX) $(DEBUG) $(CXXFLAGS) -o $@ $^ $(INC) $(OPT) $(THREAD) 

bin/build_index: build_index.cpp
	$(CXX) $(DEBUG) $(CXXFLAGS) -o $@ $^ $(INC) $(OPT) $(THREAD)

clean:
	rm -rf bin
	