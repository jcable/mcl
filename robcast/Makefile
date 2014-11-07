# $Id: Makefile,v 1.20 2004/10/14 14:21:50 roca Exp $
#
# Makefile
#

####### OS Dependant Definitions ######

# they are defined in the following file...
# edit Makefile.common as appropriate
include ../Makefile.common


# libraries required
ifeq (${OS},linux)
	# Linux version
	# MCL library requires -lpthread
	# add MPATROL support if required
	#
	BINDIR = ../bin/linux
	LDPC_BINDIR = ../bin/linux
	LIBRARIES = -L${BINDIR} -lmclalc -lldpc -lc -lm -lpthread
endif

ifeq (${OS},solaris)
	# Solaris version: requires -lsocket -lnsl and -L/usr/ucblib -lucb
	#
	BINDIR = ../bin/solaris
	LDPC_BINDIR = ../../ldpc/bin/solaris
	LIBRARIES = -lc -lm -lsocket -lnsl -L/usr/ucblib -lucb -lposix4 -lpthread ${BINDIR}/libmclalc.a ${LDPC_BINDIR}/libldpc.a
endif

ifeq (${OS},freebsd)
	# FreeBSD version
	# MCL library requires -pthread
	#
	BINDIR = ../bin/freebsd
	LDPC_BINDIR = ../../ldpc/bin/freebsd
	LIBRARIES = -lm -pthread ${BINDIR}/libmclalc.a
	LIBRARIES = -lm -pthread ${BINDIR}/libmclalc.a ${LDPC_BINDIR}/libldpc.a
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


# List of source and object test files 
SRCFILES =  robcast_client.c robcast_server.c
OFILES	 = ${SRCFILES:.c=.o}


###### Dependancies ######

all:	${BINDIR}/robcast_client ${BINDIR}/robcast_server

${BINDIR}/robcast_client:	robcast_client.o ${BINDIR}/libmclalc.a
	@echo "** Compiling robcast_client with ALC support..."
	${CC} ${CFLAGS} robcast_client.o ${LIBRARIES} -o ${BINDIR}/robcast_client
	#strip ${BINDIR}/robcast_client

${BINDIR}/robcast_server:	robcast_server.o ${BINDIR}/libmclalc.a
	@echo "** Compiling robcast_server with ALC support..."
	${CC} ${CFLAGS} robcast_server.o ${LIBRARIES} -o ${BINDIR}/robcast_server
	#strip ${BINDIR}/robcast_server

clean :
	@for f in core ${OFILES} ; do \
		if [ -f "$${f}" ] ; then \
			rm "$${f}"; \
		fi \
	done

cleanall : clean
	@for f in ${BINDIR}/robcast_client ${BINDIR}/robcast_server ${BINDIR}/robcast_server_repository.txt ; do \
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

robcast_client.o: robcast.h ../src/common/mcl_lib_api.h
robcast_client.o: ../src/common/mcl_types.h ../src/alc/mcl_lib_api_alc.h
robcast_server.o: robcast.h ../src/common/mcl_lib_api.h
robcast_server.o: ../src/common/mcl_types.h ../src/alc/mcl_lib_api_alc.h
