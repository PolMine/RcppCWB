##   -*-Makefile-*-
## 
##  IMS Open Corpus Workbench (CWB)
##  Copyright (C) 1993-2006 by IMS, University of Stuttgart
##  Copyright (C) 2007-     by the respective contributers (see file AUTHORS)
## 
##  This program is free software; you can redistribute it and/or modify it
##  under the terms of the GNU General Public License as published by the
##  Free Software Foundation; either version 2, or (at your option) any later
##  version.
## 
##  This program is distributed in the hope that it will be useful, but
##  WITHOUT ANY WARRANTY; without even the implied warranty of
##  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General
##  Public License for more details (in the file "COPYING", or available via
##  WWW at http://www.gnu.org/copyleft/gpl.html).


##  This is the Makefile for the CWB Corpus Library. 



## Read configuration settings and standard definitions
TOP = $(R_PACKAGE_SOURCE)
include $(TOP)/config.mk

## The following targets are available:
#
#  all          create archive libcl.a
#  clean        remove object files and libcl.a
#  realclean    also deleted automatically generated parsers and dependencies
#  depend       update dependencies
#  install      install libcl.a and CL headers
#  uninstall    uninstall libcl.a and headers from chosen locations (currently not supported)
#  release      install to binary release dir
#  size         print size of source code (line counts)
#

.PHONY: all clean realclean depend install uninstall size

## Use bison/flex to compile parser for registry entries
PARSEH  = registry.tab.h
PARSES  = registry.tab.c lex.creg.c
PARSEG  = registry.l registry.y
PARSEO  = registry.tab.o lex.creg.o

## Compile and link-in windows-mmap functions only if we are in MinGW
ifdef __MINGW__
MMAPH = windows-mmap.h
MMAPS = windows-mmap.c
MMAPO = windows-mmap.o
else
MMAPH =
MMAPS =
MMAPO =
endif 

## cl.h header file will be installed in subdirectory cwb/
REALINCINSTDIR = $(INCINSTDIR)/cwb

## ----------------------------------------------------------------------
## CORPUS LIBRARY  headers / sources / objects 

HDRS = globals.h cwb-globals.h ui-helpers.c \
       bitfields.h storage.h fileutils.h \
       special-chars.h \
       corpus.h attributes.h makecomps.h \
       $(PARSEH) \
       bitio.h \
       endian2.h \
       compression.h \
       $(MMAPH)

SRCS = globals.c macros.c ui-helpers.c \
       list.c lexhash.c ngram-hash.c \
       bitfields.c storage.c fileutils.c \
       special-chars.c regopt.c \
       corpus.c attributes.c makecomps.c \
       $(PARSES) \
       cdaccess.c \
       bitio.c \
       endian.c \
       compression.c \
       $(MMAPS)
 
OBJS = globals.o macros.o ui-helpers.o \
       list.o lexhash.o ngram-hash.o \
       bitfields.o storage.o fileutils.o \
       special-chars.o regopt.o \
       corpus.o attributes.o makecomps.o \
       $(PARSEO) \
       cdaccess.o \
       bitio.o \
       endian.o \
       compression.o \
       $(MMAPO)

## Extra objects (will be compiled, but not included in libcl.a) 
#   - dl_stub.o is a dummy replacement for libdl.a, which is missing in Solaris
EXTRA_OBJS = dl_stub.o

# ----------------------------------------------------------------------

all: libcl.a $(EXTRA_OBJS)

libcl.a: $(OBJS)
	$(RM) $@
	$(AR) cq $@ $^

install: libcl.a
ifndef __MINGW__
	if [ ! -d "$(LIBINSTDIR)" ]; then $(INSTALL) $(INST_FLAGS_DIR) "$(LIBINSTDIR)" ; fi;
	$(INSTALL) $(INST_FLAGS_DATA) libcl.a "$(LIBINSTDIR)"
	if [ ! -d "$(REALINCINSTDIR)" ]; then $(INSTALL) $(INST_FLAGS_DIR) "$(REALINCINSTDIR)" ; fi;
	$(INSTALL) $(INST_FLAGS_DATA) cl.h "$(REALINCINSTDIR)"
else
endif

depend:
	-$(RM) depend.mk
	$(MAKE) depend.mk

# be sure to create bison/flex C code before making dependencies
depend.mk:	$(PARSEH) $(PARSES) $(SRCS)
	-$(RM) depend.mk
	$(DEPEND) $(DEPEND_CFLAGS_ALL) $(SRCS) > depend.mk

clean:
	-$(RM) *.o libcl.a registry.output *~

realclean: clean
	-$(RM) registry.tab.c registry.tab.c lex.creg.c registry.tab.h
	-$(RM) depend.mk

size: $(SRCS) $(HDRS)
# I am not sure if the following line will work to make these print even with .SILENT turned on ...
	@$(ECHO) `$(WC) $(SRCS) $(HDRS)` 
	$(WC) $(SRCS) $(HDRS) 


# -------- dependencies --------
include depend.mk

