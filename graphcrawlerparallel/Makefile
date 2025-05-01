CXXFLAGS=-I$(HOME)/rapidjson/include -pthread
LDFLAGS=-lcurl -pthread
LD=g++
CC=g++

all: level_client

level_client: level_client.o
	$(LD) $< -o $@ $(LDFLAGS)

clean:
	-rm level_client level_client.o

