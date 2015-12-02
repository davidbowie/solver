CC=g++
CFLAGS=-g -c -Wall -std=c++11
LDFLAGS=
SOURCES=solver.cpp solverLib.cpp solverFunc.cpp
OBJECTS=$(SOURCES:.cpp=.o)
EXECUTABLE=solver

all: $(SOURCES) $(EXECUTABLE)
	    
$(EXECUTABLE): $(OBJECTS) 
	$(CC) $(LDFLAGS) $(OBJECTS) -o $@

.cpp.o:
	$(CC) $(CFLAGS) $< -o $@
	
.PHONY: clean
clean:
	rm -f *o solver
