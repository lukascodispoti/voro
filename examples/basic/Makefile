# Voro++ makefile
# By Chris H. Rycroft and the Rycroft Group

# Load the common configuration file
include ../../config.mk

# List of executables
EXECUTABLES=single_cell platonic random_points import mthread convex_test

# Makefile rules
all: $(EXECUTABLES)

single_cell: single_cell.cc
	$(CXX) $(CFLAGS) $(E_INC) $(E_LIB) -o single_cell single_cell.cc -lvoro++

platonic: platonic.cc
	$(CXX) $(CFLAGS) $(E_INC) $(E_LIB) -o platonic platonic.cc -lvoro++

random_points: random_points.cc
	$(CXX) $(CFLAGS) $(E_INC) $(E_LIB) -o random_points random_points.cc -lvoro++

import: import.cc
	$(CXX) $(CFLAGS) $(E_INC) $(E_LIB) -o import import.cc -lvoro++

mthread: mthread.cc
	$(CXX) $(CFLAGS) $(E_INC) $(E_LIB) -o mthread mthread.cc -lvoro++

convex_test: convex_test.cc
	$(CXX) $(CFLAGS) $(E_INC) $(E_LIB) -o convex_test convex_test.cc -lvoro++

clean:
	rm -f $(EXECUTABLES)

.PHONY: all clean
