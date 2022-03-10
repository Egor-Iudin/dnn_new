# Define the flags to control make
CC = g++
INCLUDE = -Iinclude -I../bnn-hep/include/ -I$(shell root-config --incdir) -I${BOOST_ROOT=}include/boost/
OPFLAGS = 
CFLAGS = -Wall -Wextra -std=c++17 $(INCLUDE) $(OPFLAGS)
LDFLAGS = $(shell root-config --libs) -L${BOOST_ROOT=}lib/ -lboost_filesystem -lboost_system

vpath %.cpp src ../bnn-hep/src

# Define the phony targets
.PHONY: clean

# The default rule
all: eventIndexToID eventIDToIndex

eventIndexToID: eventIndexToID.o EventID.o TrainEventList.o
	$(CC) $(LDFLAGS) $+ -o $@
	@ cd ../bin; ln -sf ../convenience/$@; cd -

eventIDToIndex: eventIDToIndex.o EventID.o TrainEventList.o
	$(CC) $(LDFLAGS) $+ -o $@
	@ cd ../bin; ln -sf ../convenience/$@; cd -

%.o: %.cpp
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	@ rm -f *.o
	@ rm -f ../bin/eventIndexToID ../bin/eventIDToIndex
