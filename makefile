CC = g++
CFLAGS = -Wall -O2
TARGET = merge_sort

all: $(TARGET)

$(TARGET): merge_sort.cpp
	$(CC) $(CFLAGS) merge_sort.cpp -o $(TARGET)

clean:
	rm -f $(TARGET)
