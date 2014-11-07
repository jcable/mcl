# $Id: Makefile,v 1.17 2004/12/10 10:37:13 chneuman Exp $
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
	# MCL library requires -lpthread
	# add MPATROL support if required
	#
	BINDIR = ../../bin/linux
	LIBRARIES = -lc -lm -lpthread ${BINDIR}/libmclnorm.a
endif

ifeq (${OS},solaris)
	# Solaris version: requires -lsocket -lnsl and -L/usr/ucblib -lucb
	#
	BINDIR = ../../bin/solaris
	LIBRARIES = -lc -lm -lsocket -lnsl -L/usr/ucblib -lucb -lposix4 -lpthread ${BINDIR}/libmclnorm.a
endif

ifeq (${OS},freebsd)
	# FreeBSD version
	BINDIR = ../../bin/freebsd
	LDPC_BINDIR = ../../bin/freebsd
	LIBRARIES = -lc -lm -pthread ${BINDIR}/libmclnorm.a
endif



###### Other variables ######

# Local CFLAGS.

# gcc version
CFLAGS  = -O -g ${OSFLAGS} -DNORM


# add purify linker for memory debug (Solaris only).
# Purify 4.0 Solaris 2, Copyright (C) 1992-1996 Pure Software Inc.
#
#CCLINK_PURE = purify $(CC)
CCLINK_PURE = purify  -cache-dir=/tmp $(CC)


# List of source and object test files 
SRCFILES = test_tx.c test_multi_sessions1.c test_multi_sessions2.c
OFILES	 = ${SRCFILES:.c=.o}
TARGETS  = ${SRCFILES:.c=}



###### Dependancies ######

all:	${TARGETS}

test_tx:	test_tx.o ${BINDIR}/libmclnorm.a
	${CC} ${CFLAGS} test_tx.o ${LIBRARIES} -o test_tx
	#strip test_tx

test_multi_sessions1:	test_multi_sessions1.o ${BINDIR}/libmclnorm.a
	${CC} ${CFLAGS} test_multi_sessions1.o ${LIBRARIES} -o test_multi_sessions1
	#strip test_multi_sessions1

test_multi_sessions2:	test_multi_sessions2.o ${BINDIR}/libmclnorm.a
	${CC} ${CFLAGS} test_multi_sessions2.o ${LIBRARIES} -o test_multi_sessions2
	#strip test_multi_sessions2

pure:	test_tx.o ${BINDIR}/libmclnorm.a
	${CCLINK_PURE} ${CFLAGS} test_tx.o ${LIBRARIES} -o test_tx

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

test_tx.o: ../../src/common/mcl_lib_api.h ../../src/common/mcl_types.h
test_tx.o: ../../src/norm/mcl_lib_api_norm.h
test_multi_sessions1.o: ../../src/common/mcl_lib_api.h
test_multi_sessions1.o: ../../src/common/mcl_types.h
test_multi_sessions1.o: ../../src/norm/mcl_lib_api_norm.h
test_multi_sessions2.o: ../../src/common/mcl_lib_api.h
test_multi_sessions2.o: ../../src/common/mcl_types.h
test_multi_sessions2.o: ../../src/norm/mcl_lib_api_norm.h
