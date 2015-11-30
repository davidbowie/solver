CC=g++
CFLAGS=-g -c -Wall -std=c++11
LDFLAGS=
SOURCES=solver.cpp solverLib.cpp
OBJECTS=$(SOURCES:.cpp=.o)
EXECUTABLE=solver

all: $(SOURCES) $(EXECUTABLE)
	    
$(EXECUTABLE): $(OBJECTS) 
	$(CC) $(LDFLAGS) $(OBJECTS) -o $@

.cpp.o:
	$(CC) $(CFLAGS) $< -o $@
	
.PHONY: clean
clean:
	rm *o equation
