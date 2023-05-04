# Voro++ makefile
# By Chris H. Rycroft and the Rycroft Group

# Tell GNU Make that these are phony targets
.PHONY: all help clean install uninstall

include config.mk
 
# Build all of the executable files
all:
	$(MAKE) EULER=$(EULER) -C src

# Clean up the executable files
clean:
	$(MAKE) -C src clean

# Install the executable, man page, and shared library
install:
	$(MAKE) -C src
	$(INSTALL) -d $(IFLAGS_EXEC) $(PREFIX)/bin
	$(INSTALL) -d $(IFLAGS_EXEC) $(PREFIX)/lib
	$(INSTALL) -d $(IFLAGS_EXEC) $(PREFIX)/man
	$(INSTALL) -d $(IFLAGS_EXEC) $(PREFIX)/man/man1
	$(INSTALL) -d $(IFLAGS_EXEC) $(PREFIX)/include
	$(INSTALL) -d $(IFLAGS_EXEC) $(PREFIX)/include/voro++
	$(INSTALL) $(IFLAGS_EXEC) src/voro++ $(PREFIX)/bin
	$(INSTALL) $(IFLAGS) man/voro++.1 $(PREFIX)/man/man1
	$(INSTALL) $(IFLAGS) src/libvoro++.a $(PREFIX)/lib
	$(INSTALL) $(IFLAGS) src/*.hh $(PREFIX)/include/voro++

# Uninstall the executable, man page, and shared library
uninstall:
	rm -f $(PREFIX)/bin/voro++
	rm -f $(PREFIX)/man/man1/voro++.1
	rm -f $(PREFIX)/lib/libvoro++.a
	rm -f $(PREFIX)/include/voro++/voro++.hh
	rm -f $(PREFIX)/include/voro++/c_loops.hh
	rm -f $(PREFIX)/include/voro++/cell.hh
	rm -f $(PREFIX)/include/voro++/common.hh
	rm -f $(PREFIX)/include/voro++/config.hh
	rm -f $(PREFIX)/include/voro++/container.hh
	rm -f $(PREFIX)/include/voro++/container_prd.hh
	rm -f $(PREFIX)/include/voro++/pre_container.hh
	rm -f $(PREFIX)/include/voro++/rad_option.hh
	rm -f $(PREFIX)/include/voro++/unitcell.hh
	rm -f $(PREFIX)/include/voro++/v_base.hh
	rm -f $(PREFIX)/include/voro++/v_compute.hh
	rm -f $(PREFIX)/include/voro++/wall.hh
	rm -f $(PREFIX)/include/voro++/worklist.hh
	rm -r -f $(PREFIX)/include/voro++
