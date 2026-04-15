CXX = g++
CXXFLAGS = -Wall -pthread

all: server tezt122

server: text12.cpp text12.h
	$(CXX) text12.cpp -o server $(CXXFLAGS)

tezt122: tezt122.cpp text12.h
	$(CXX) tezt122.cpp -o tezt122 $(CXXFLAGS)

clean:
	rm -f server tezt122 a.out srver text text12

.PHONY: all clean
