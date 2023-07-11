# Voro++, a 3D cell-based Voronoi library
# By Chris H. Rycroft and the Rycroft Group

# This a common configuration file that includes definitions used by all
# the Makefiles.

# C++ compiler


# Flags for the C++ compiler
CFLAGS+=-Wall -ansi -pedantic -O3
CFLAGS+=-Wformat=0

# Relative include and library paths for compilation of the examples
E_INC=-I../../src
E_LIB=-L../../src

# Installation directory
PREFIX?=.

# Install command
INSTALL=install

# Flags for install command for executable
IFLAGS_EXEC=-m 0755

# Flags for install command for non-executable files
IFLAGS=-m 0644
