CXX ?= c++
CXXFLAGS ?= -std=c++11 -O2 -Wall
TARGET = bin/upp-bootstrap

all: $(TARGET)

$(TARGET): src/main.cpp
	mkdir -p bin
	$(CXX) $(CXXFLAGS) -o $(TARGET) src/main.cpp

bootstrap: $(TARGET)
	./$(TARGET) --bootstrap --methods-dir methods

clean:
	rm -rf bin/

.PHONY: all bootstrap clean
