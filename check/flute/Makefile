# $Id: Makefile,v 1.2 2005/05/12 16:03:17 moi Exp $
#
# Makefile
#

####### OS Dependant Definitions ######

# they are defined in the following file...
# edit Makefile.common as appropriate
include ../../Makefile.common



# libraries required
ifeq (${OS},linux)
	# Linux version
	#
	BINDIR = ../../bin/linux
	LDPC_BINDIR = ../../bin/linux
	XERCES_DIR = /usr/include/xercesc
	# Shared version (default)
	LIBRARIES = -lc -lm -lpthread 
	#-lcrypto
	ADD_INCLUDES = -I${XERCES_DIR}
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
	BINDIR = ../../bin/solaris
	LDPC_BINDIR = ../../bin/solaris
	XERCES_DIR = ../../../xerces-c
	OPENSSL_DIR = /usr/local/ssl
	LIBRARIES = -lc -lm -lsocket -lnsl -lposix4 -mt -lpthread -L/usr/ucblib -lucb -L${OPENSSL_DIR}/lib -lcrypto
	ADD_INCLUDES = -I/usr/local/include/ -I${XERCES_DIR}/include -I${OPENSSL_DIR}/include
endif

ifeq (${OS},freebsd)
	# FreeBSD version:
	# CC requires: -mt -lpthread for multithreaded applications
	#
	BINDIR = ../../bin/freebsd
	LDPC_BINDIR = ../../bin/freebsd
	XERCES_DIR = ../../../xerces-c
	LIBRARIES = -lm -pthread -L/usr/local/lib -lcrypto
	#ADD_INCLUDES = -I/usr/local/include/ -I${XERCES_DIR}/include
	ADD_INCLUDES = -I${XERCES_DIR}/include
endif

ifeq (${OS},aix)
	# AIX version
	BINDIR = ../../bin/aix
	LDPC_BINDIR = ../../bin/aix
	XERCES_DIR = ../../../xerces-c
	LIBRARIES = -lm -pthread -lc -lcrypto
	ADD_INCLUDES = -I/usr/local/include/ -I${XERCES_DIR}/include
endif

ifeq (${OS},hp-ux)
	# HP-UX version
	BINDIR = ../../bin/hp-ux
	LDPC_BINDIR = ../../bin/hp-ux
	XERCES_DIR = ../../../xerces-c
	LIBRARIES = -lc -lm -pthread -lcrypto
	ADD_INCLUDES = -I/usr/local/include/ -I${XERCES_DIR}/include
endif

ifeq (${OS},irix)
	# IRIX version
	BINDIR = ../../bin/irix
	LDPC_BINDIR = ../../bin/irix
	XERCES_DIR = ../../../xerces-c
	LIBRARIES = -lc -lm -pthread -lcrypto
	ADD_INCLUDES = -I/usr/local/include/ -I${XERCES_DIR}/include
endif



###### Other variables ######

# Local CFLAGS.

# gcc version
CFLAGS  = -O -g -DALC ${OSFLAGS}

# cc version
# -fullwarn for IRIX c compiler
#CFLAGS  = -O -fullwarn -DALC ${OSFLAGS} 

# add purify linker for memory debug (Solaris only).
# Purify 4.0 Solaris 2, Copyright (C) 1992-1996 Pure Software Inc.
#
#CCLINK_PURE = purify $(CC)
CCLINK_PURE = purify  -cache-dir=/tmp $(CC)

# XML parsing library
# choose between the static / dynamic (default) xerces-c versions.
LIBRARIES := ${LIBRARIES} -lxerces-c
LIBRARIES := ${LIBRARIES} -lxerces-c

# Other libraries
MCL_LIBRARY = ${BINDIR}/libmclalc.a ${LDPC_BINDIR}/libldpc.a
SDP_LIBRARY = ${BINDIR}/libsinistersdp.a
FLUTE_LIBRARY = ${BINDIR}/libflute.a


# List of source and object test files 
SRCFILES = test_open_close.cpp
OFILES	 = ${SRCFILES:.cpp=.o}
TARGETS  = ${SRCFILES:.cpp=}



###### Dependancies ######

all:	${TARGETS}

# Rule for C++ .cpp extension
.cpp.o:
	$(CC) -c $(CFLAGS) -o $*.o $*.cpp

test_open_close:	test_open_close.o ${FLUTE_LIBRARY}
	${CC} ${CFLAGS} test_open_close.o ${LIBRARIES} ${FLUTE_LIBRARY} ${SDP_LIBRARY} ${MCL_LIBRARY} -o test_open_close
	#strip test_open_close

clean :
	@for f in core ${OFILES} ; do \
		if [ -f "$${f}" ] ; then \
			rm "$${f}"; \
		fi \
	done

cleanall : clean
	@for f in core ${TARGETS} ; do \
		if [ -f "$${f}" ] ; then \
			rm $${f} ; \
		fi \
	done

depend :
	# Use -Y to prevent including the system header files that won't
	# change. Send all error messages to /dev/null
	${DEPEND} -- ${CFLAGS} -Y -- ${SRCFILES} 2>/dev/null


# Dependencies automatically created by makedepend (see depend target above).
#
# DO NOT DELETE THIS LINE -- make depend depends on it.

test_open_close.o: ../../src/flute_lib/FluteAPI.h
test_open_close.o: ../../src/common/mcl_lib_api.h
test_open_close.o: ../../src/common/mcl_types.h
test_open_close.o: ../../src/alc/mcl_lib_api_alc.h
test_open_close.o: ../../src/flute_lib/Flute.h
test_open_close.o: ../../src/flute_lib/FluteReceiver.h
test_open_close.o: ../../src/flute_lib/FluteSender.h
test_open_close.o: ../../src/flute_lib/FluteFileInfo.h
test_open_close.o: ../../src/flute_lib/FluteFileDeliveryCarousel.h
