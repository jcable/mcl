####### OS Dependant Definitions ######

# they are defined in the following file...
# edit Rules.make as appropriate
include ../../../Makefile.common


# libraries required
ifeq (${OS},linux)
	# Linux version
	#
	BINDIR = ../../../bin/linux
endif

ifeq (${OS},solaris)
	# Solaris version:
	# Warning: SUN recommends to have -lucb at the end of list,
	# in order to priviledge SUN's libs to the BSD compatibility
	# lib (libucb).
	# CC requires: -mt -lpthread for multithreaded applications
	# OpenSSL is installed at /usr/local/ssl in our system. Update if
	# needed.
	#
	BINDIR = ../../../bin/solaris
endif

ifeq (${OS},freebsd)
	# FreeBSD version:
	# CC requires: -mt -lpthread for multithreaded applications
	#
	BINDIR = ../../../bin/freebsd
endif

ifeq (${OS},aix)
	# AIX version
	BINDIR = ../../../bin/aix
endif

ifeq (${OS},hp-ux)
	# HP-UX version
	BINDIR = ../../../bin/hp-ux
endif

ifeq (${OS},irix)
	# IRIX version
	BINDIR = ../../../bin/irix
endif


# just change these around to suit your compiler and OS:
CC      = gcc
CFLAGS  = -c -ansi -Wall
AR      = ar
ARFLAGS = rvs

# the C source, header, and object files used by the main library:
SDP_SOURCE_FILES = SDP_Parser.c \
                   SDP_EventStreamParser.c \
                   SDP_StreamTokenizer.c \
                   SDP_Description.c \
                   SDP_Generator.c \
                   SDP_Error.c \
                   SDP_Str.c \
                   SDP_Utility.c
SDP_HEADER_FIELS = SDP_Parser.h \
                   SDP_EventStreamParser.h \
                   SDP_StreamTokenizer.h \
                   SDP_Description.h \
                   SDP_Generator.h \
                   SDP_LinkedList.h \
                   SDP_Error.h \
                   SDP_Str.h \
                   SDP_Utility.h
SDP_OBJECT_FILES = SDP_Parser.o \
                   SDP_EventStreamParser.o \
                   SDP_StreamTokenizer.o \
                   SDP_Description.o \
                   SDP_Parser.o \
                   SDP_Generator.o \
                   SDP_Error.o \
                   SDP_Str.o \
                   SDP_Utility.o

# the source, header, and object files used just by the generator (to build it
# separately):
SDP_GENERATOR_SOURCE_FILES = SDP_Generator.c \
                             SDP_Description.c \
                             SDP_Error.c \
                             SDP_Str.c \
                             SDP_Utility.c
SDP_GENERATOR_HEADER_FILES = SDP_Generator.h \
                             SDP_Description.h \
                             SDP_Error.h \
                             SDP_LinkedList.h \
                             SDP_Str.h \
                             SDP_Utility.h
SDP_GENERATOR_OBJECT_FILES = SDP_Generator.o \
                             SDP_Description.o \
                             SDP_Error.o \
                             SDP_Str.o \
                             SDP_Utility.o





all: libsinistersdp.a

libsinistersdp.a: $(SDP_SOURCE_FILES) $(SDP_HEADER_FILES)
	$(CC) $(CFLAGS) $(SDP_SOURCE_FILES)
	$(AR) $(ARFLAGS) $@ $(SDP_OBJECT_FILES)
	mv *.a "$(BINDIR)"

# this can be used to build just the generator by itself:
libsdpgenerator.a: $(SDP_GENERATOR_SOURCE_FILES) $(SDP_GENERATOR_HEADER_FILES)
	$(CC) $(CFLAGS) $(SDP_GENERATOR_SOURCE_FILES)
	$(AR) $(ARFLAGS) $@ $(SDP_GENERATOR_OBJECT_FILES)
	mv *.a "$(BINDIR)"

clean:
	@for f in core ${SDP_OBJECT_FILES} ; do \
		if [ -f "$${f}" ] ; then \
			rm "$${f}"; \
		fi \
	done
		
cleanall : clean
	@for f in ${BINDIR}/libsinistersdp.a ${BINDIR}/libsdpgenerator.a; do \
		if [ -f "$${f}" ] ; then \
			rm $${f} ; \
		fi \
	done
