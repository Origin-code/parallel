CXX = g++
CXXFLAGS = -O2 -Wall -Wextra -std=c++17

TARGET = nbody
SRC = nbody_simulation.cpp

all: $(TARGET)

$(TARGET): $(SRC)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(SRC)

clean:
	rm -f $(TARGET)

run:
	./$(TARGET) 100 1.0 10000 100

benchmark:
	./$(TARGET) 1000 1.0 10000 100

