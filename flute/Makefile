
####### OS Dependant Definitions ######

# they are defined in the following file...
# edit Rules.make as appropriate
include ../Makefile.common


# libraries required
ifeq (${OS},linux)
	# Linux version
	#
	BINDIR = ../bin/linux
	LDPC_BINDIR = ../../ldpc/bin/linux
	XERCES_DIR = ../../xerces-c
	# Shared version (default)
	LIBRARIES = -lc -lm -lpthread 
	#-lcrypto
	ADD_INCLUDES = -I/usr/local/include/ -I${XERCES_DIR}/include
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
	BINDIR = ../bin/solaris
	LDPC_BINDIR = ../../ldpc/bin/solaris
	XERCES_DIR = ../../xerces-c
	OPENSSL_DIR = /usr/local/ssl
	LIBRARIES = -lc -lm -lsocket -lnsl -lposix4 -mt -lpthread -L/usr/ucblib -lucb -L${OPENSSL_DIR}/lib -lcrypto
	ADD_INCLUDES = -I/usr/local/include/ -I${XERCES_DIR}/include -I${OPENSSL_DIR}/include
endif

ifeq (${OS},freebsd)
	# FreeBSD version:
	# CC requires: -mt -lpthread for multithreaded applications
	#
	BINDIR = ../bin/freebsd
	LDPC_BINDIR = ../../ldpc/bin/freebsd
	XERCES_DIR = ../../xerces-c
	LIBRARIES = -lm -pthread -L/usr/local/lib -lcrypto
	#ADD_INCLUDES = -I/usr/local/include/ -I${XERCES_DIR}/include
	ADD_INCLUDES = -I${XERCES_DIR}/include
endif

ifeq (${OS},aix)
	# AIX version
	BINDIR = ../bin/aix
	LDPC_BINDIR = ../../ldpc/bin/aix
	XERCES_DIR = ../../xerces-c
	LIBRARIES = -lm -pthread -lc -lcrypto
	ADD_INCLUDES = -I/usr/local/include/ -I${XERCES_DIR}/include
endif

ifeq (${OS},hp-ux)
	# HP-UX version
	BINDIR = ../bin/hp-ux
	LDPC_BINDIR = ../../ldpc/bin/hp-ux
	XERCES_DIR = ../../xerces-c
	LIBRARIES = -lc -lm -pthread -lcrypto
	ADD_INCLUDES = -I/usr/local/include/ -I${XERCES_DIR}/include
endif

ifeq (${OS},irix)
	# IRIX version
	BINDIR = ../bin/irix
	LDPC_BINDIR = ../../ldpc/bin/irix
	XERCES_DIR = ../../xerces-c
	LIBRARIES = -lc -lm -pthread -lcrypto
	ADD_INCLUDES = -I/usr/local/include/ -I${XERCES_DIR}/include
endif


###### Other variables ######
# Local CFLAGS.

# gcc version
#CFLAGS  = -g -pg -O ${OSFLAGS} -DALC ${ADD_INCLUDES}
CFLAGS  = -g -O -DDEBUG ${OSFLAGS} -DALC ${ADD_INCLUDES}
#CFLAGS  = -O ${OSFLAGS} -DALC ${ADD_INCLUDES}
#static:	CFLAGS := ${CFLAGS} -static  -static-libgcc

# add purify linker for memory debug (Solaris only).
# Purify 4.0 Solaris 2, Copyright (C) 1992-1996 Pure Software Inc.
#
#CCLINK_PURE = purify $(CC)
CCLINK_PURE = purify  -cache-dir=/tmp $(CC)

# add quantify linker for profiling (Solaris only).
CCLINK_QUANTIFY = quantify $(CC)

# list of source and object files
SRCFILES = main.cpp fsend.cpp frecv.cpp getopt.cpp display.cpp file_tools.cpp
OFILES	 = ${SRCFILES:.cpp=.o}

#OS flag with upper case chars
OS_UPPER_CASE = `echo ${OS} | tr 'a-z' 'A-Z'`


# XML parsing library
# choose between the static / dynamic (default) xerces-c versions.
dynamic: LIBRARIES := ${LIBRARIES} -lxerces-c -lxerces-depdom
static:  LIBRARIES := ${LIBRARIES} ${XERCES_DIR}/lib/libxerces-c.a

# Other libraries
MCL_LIBRARY = ${BINDIR}/libmclalc.a ${LDPC_BINDIR}/libldpc.a
SDP_LIBRARY = ${BINDIR}/libsinistersdp.a
FLUTE_LIBRARY = ${BINDIR}/libflute.a

# executable is here:
dynamic: TARGET_FLUTE = ${BINDIR}/flute
static: TARGET_FLUTE = ${BINDIR}/flute-static


###### Dependancies ######

# by default, build the dynamic FLUTE version...
all:	dynamic


# Rule for C++ .cpp extension
.cpp.o:
	$(CC) -c $(CFLAGS) -o $*.o $*.cpp

dynamic:	${OFILES} ${FLUTE_LIBRARY}
	@echo "** Compiling flute-ALC using the MCL library..."
	${CC} ${CFLAGS} ${OFILES} ${LIBRARIES} ${FLUTE_LIBRARY} ${SDP_LIBRARY} ${MCL_LIBRARY} -o ${TARGET_FLUTE}


static: ${OFILES} ${FLUTE_LIBRARY}
	@echo "** Compiling flute-ALC using the MCL library..."
	${CC} -static  -static-libgcc ${CFLAGS} ${OFILES} ${FLUTE_LIBRARY} ${SDP_LIBRARY} ${MCL_LIBRARY} ${LIBRARIES} -o ${TARGET_FLUTE}

clean :
	@for f in core ${OFILES} *_pure* *.pure_* .pur* *purify* ; do \
		if [ -f "$${f}" ] ; then \
			rm "$${f}"; \
		fi \
	done

cleanall : clean
	@for f in ${BINDIR}/flute ${BINDIR}/flute-static; do \
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

main.o: ../src/flute_lib/FluteAPI.h ../src/common/mcl_lib_api.h
main.o: ../src/common/mcl_types.h ../src/alc/mcl_lib_api_alc.h
main.o: ../src/flute_lib/Flute.h ../src/flute_lib/FluteReceiver.h
main.o: ../src/flute_lib/FluteSender.h ../src/flute_lib/FluteFileInfo.h
main.o: ../src/flute_lib/FluteFileDeliveryCarousel.h fsend.h frecv.h
main.o: display.h macros.h
fsend.o: ../src/flute_lib/FluteAPI.h ../src/common/mcl_lib_api.h
fsend.o: ../src/common/mcl_types.h ../src/alc/mcl_lib_api_alc.h
fsend.o: ../src/flute_lib/Flute.h ../src/flute_lib/FluteReceiver.h
fsend.o: ../src/flute_lib/FluteSender.h ../src/flute_lib/FluteFileInfo.h
fsend.o: ../src/flute_lib/FluteFileDeliveryCarousel.h fsend.h file_tools.h
fsend.o: macros.h
frecv.o: ../src/flute_lib/FluteAPI.h ../src/common/mcl_lib_api.h
frecv.o: ../src/common/mcl_types.h ../src/alc/mcl_lib_api_alc.h
frecv.o: ../src/flute_lib/Flute.h ../src/flute_lib/FluteReceiver.h
frecv.o: ../src/flute_lib/FluteSender.h ../src/flute_lib/FluteFileInfo.h
frecv.o: ../src/flute_lib/FluteFileDeliveryCarousel.h frecv.h macros.h
getopt.o: getopt.h
display.o: ../src/flute_lib/FluteAPI.h ../src/common/mcl_lib_api.h
display.o: ../src/common/mcl_types.h ../src/alc/mcl_lib_api_alc.h
display.o: ../src/flute_lib/Flute.h ../src/flute_lib/FluteReceiver.h
display.o: ../src/flute_lib/FluteSender.h ../src/flute_lib/FluteFileInfo.h
display.o: ../src/flute_lib/FluteFileDeliveryCarousel.h display.h macros.h
file_tools.o: macros.h
