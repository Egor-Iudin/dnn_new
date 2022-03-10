# Define the flags to control make
CC = g++
INCLUDE = -Iinclude -I../libconfig/ -I$(shell root-config --incdir) -I${CMSSW_FWLITE_INCLUDE_PATH}/boost/
OPFLAGS = -lboost_filesystem -lboost_system
CFLAGS = -Wall -fopenmp -Wextra -std=c++17 $(INCLUDE) $(OPFLAGS)
LDFLAGS = $(shell root-config --libs) -lTreePlayer -L${CMSSW_FWLITE_INCLUDE_PATH}/../lib/ -fopenmp
EXECUTABLE = bnn-hep
SOURCES = $(shell ls src/ | grep .cpp)
OBJECTS = $(SOURCES:.cpp=.o)
# See http://www.gnu.org/software/make/manual/make.html#Substitution-Refs

# Define the search paths for the targets and dependencies. See
# http://www.gnu.org/software/make/manual/make.html#Directory-Search
vpath %.hpp include
vpath %.cpp src

# Define the phony targets
.PHONY: clean

# The default rule
all: $(EXECUTABLE)

$(EXECUTABLE): $(OBJECTS)
	$(CC) $(LDFLAGS) $+ ../libconfig/libconfig++.a   -o $@
# '$@' is expanded to the target, '$+' expanded to all the dependencies. See
# http://www.gnu.org/savannah-checkouts/gnu/make/manual/html_node/Automatic-Variables.html

%.o: %.cpp
	$(CC) $(CFLAGS) -c $< -o $@
# '$<' is expanded to the first dependency

clean:
	@rm -f *.o
# '@' prevents the command from being printed to stdout


