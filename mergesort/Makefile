CC = g++
CFLAGS = -Wall -O2
TARGET = merge_sort

all: clean $(TARGET)

$(TARGET): merge_sort.cpp
	$(CC) $(CFLAGS) merge_sort.cpp -o $(TARGET)

clean:
	rm -f $(TARGET)
