# $Id: Makefile,v 1.44 2004/05/26 12:21:09 roca Exp $
#
# Makefile
#

####### OS Dependant Definitions ######

# they are defined in the following file...
# edit Rules.make as appropriate
include ../Makefile.common


# libraries required
ifeq (${OS},linux)
	# Linux version
	# requires -lpthread
	# add MPATROL support if required
	#
	BINDIR = ../../bin/linux
	LIBRARIES = -lc -lm -lpthread
endif

ifeq (${OS},solaris)
	# simple_coder Solaris version:
	# requires -lsocket -lnsl and -L/usr/ucblib -lucb
	#
	BINDIR = ../../bin/solaris
	LIBRARIES = -lc -lm -lsocket -lnsl -L/usr/ucblib -lucb -lposix4 -lpthread
endif

ifeq (${OS},freebsd)
	# FreeBSD version
	BINDIR = ../../bin/freebsd
	LIBRARIES = -lm -pthread
endif

###### Other variables ######

# Local CFLAGS.

# gcc version
#CFLAGS  = -g -pg -O ${OSFLAGS}
#CFLAGS  = -g -O -DDEBUG ${OSFLAGS}
CFLAGS  = -O ${OSFLAGS}

# add purify linker for memory debug (Solaris only).
# Purify 4.0 Solaris 2, Copyright (C) 1992-1996 Pure Software Inc.
#
#CCLINK_PURE = purify $(CC)
CCLINK_PURE = purify  -cache-dir=/tmp $(CC)

# add quantify linker for profiling (Solaris only).
CCLINK_QUANTIFY = /softs/stow/rational/releases/quantify.sol.2002a.06.00/quantify $(CC)

# LDPC library
LDPC_LIBRARY = ${BINDIR}/libldpc.a

# list of source and object files
SRCFILES = simple_coder.cpp simple_decoder.cpp perf_tool.cpp
OFILES	 = ${SRCFILES:.cpp=.o}

###### choose between the LDPC / no-LDPC (default) versions ######

LDPC:	CFLAGS := -DLDPC ${CFLAGS} 

###### Dependancies ######

all: applis

LDPC: applis

# Rule for C++ .cpp extension
.cpp.o:
	$(CC) -c $(CFLAGS) -o $*.o $*.cpp

applis:	${OFILES}
	@echo "** Compiling simple_coder/decoder..."
	${CC} ${CFLAGS} simple_coder.o ${LIBRARIES} ${LDPC_LIBRARY} -o ${BINDIR}/simple_coder
	${CC} ${CFLAGS} simple_decoder.o ${LIBRARIES} ${LDPC_LIBRARY} -o ${BINDIR}/simple_decoder
	${CC} ${CFLAGS} perf_tool.o ${LIBRARIES} ${LDPC_LIBRARY} -o ${BINDIR}/perf_tool

pure:	${OFILES}
	@echo "** Compiling simple_coder/decoder for purify..."
	${CCLINK_PURE} ${CFLAGS} simple_coder.o ${LIBRARIES} ${LDPC_LIBRARY} -o ${BINDIR}/simple_coder
	${CCLINK_PURE} ${CFLAGS} simple_decoder.o ${LIBRARIES} ${LDPC_LIBRARY} -o ${BINDIR}/simple_decoder
	${CCLINK_PURE} ${CFLAGS} perf_tool.o ${LIBRARIES} ${LDPC_LIBRARY} -o ${BINDIR}/perf_tool
	
quantify:	${OFILES}
	@echo "** Compiling simple_coder/decoder for quantify..."
	${CCLINK_QUANTIFY} ${CFLAGS} simple_coder.o ${LIBRARIES} ${LDPC_LIBRARY} -o ${BINDIR}/simple_coder
	${CCLINK_QUANTIFY} ${CFLAGS} simple_decoder.o ${LIBRARIES} ${LDPC_LIBRARY} -o ${BINDIR}/simple_decoder
	${CCLINK_QUANTIFY} ${CFLAGS} perf_tool.o ${LIBRARIES} ${LDPC_LIBRARY} -o ${BINDIR}/perf_tool
	
clean :
	@for f in core ${OFILES} *_pure* *.pure_* .pur* *purify* ; do \
		if [ -f "$${f}" ] ; then \
			rm "$${f}"; \
		fi \
	done

cleanall : clean
	@for f in ${BINDIR}/simple_coder ${BINDIR}/simple_decoder ${BINDIR}/perf_tool; do \
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

simple_coder.o: simple_coder.h ../src/ldpc_fec.h ../src/ldpc_matrix_sparse.h
simple_coder.o: ../src/tools.h ../src/ldpc_create_pchk.h
simple_decoder.o: simple_coder.h ../src/ldpc_fec.h
simple_decoder.o: ../src/ldpc_matrix_sparse.h ../src/tools.h
simple_decoder.o: ../src/ldpc_create_pchk.h
perf_tool.o: perf_tool.h ../src/ldpc_fec.h ../src/ldpc_matrix_sparse.h
perf_tool.o: ../src/tools.h ../src/ldpc_create_pchk.h ../src/macros.h
