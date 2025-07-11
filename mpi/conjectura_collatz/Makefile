CXX=mpic++
CXXFLAGS=-fopenmp -O2 -std=c++17
INCLUDES=-Iinclude

SRCS=$(wildcard src/*.cpp)
OBJS=$(SRCS:.cpp=.o)
EXEC=collatz_parallel

all: $(EXEC)

$(EXEC): $(SRCS)
	$(CXX) $(CXXFLAGS) $(INCLUDES) -o $@ $(SRCS)

clean:
	rm -f $(EXEC) src/*.o
