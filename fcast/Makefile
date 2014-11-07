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
	# add MPATROL support if required
	#
	BINDIR = ../bin/linux
	LDPC_BINDIR = ../bin/linux
	LIBRARIES = -lc -lm -lpthread
	LARGEFILE_FLAGS = -D_FILE_OFFSET_BITS=64 -D_LARGEFILE_SOURCE
endif

ifeq (${OS},solaris)
	# Solaris version:
	# Warning: SUN recommends to have -lucb at the end of list,
	# in order to priviledge SUN's libs to the BSD compatibility
	# lib (libucb).
	# CC requires: -mt -lpthread for multithreaded applications
	#
	BINDIR = ../bin/solaris
	LDPC_BINDIR = ../../ldpc/bin/solaris
	LIBRARIES = -lc -lm -lsocket -lnsl -lposix4 -mt -lpthread -L/usr/ucblib -lucb 
	LARGEFILE_FLAGS = -D_FILE_OFFSET_BITS=64 -D_LARGEFILE_SOURCE
endif

ifeq (${OS},freebsd)
	# FreeBSD version
	#
	BINDIR = ../bin/freebsd
	LDPC_BINDIR = ../../ldpc/bin/freebsd
	LIBRARIES = -lm -pthread
	# TBC
	LARGEFILE_FLAGS = -D_FILE_OFFSET_BITS=64 -D_LARGEFILE_SOURCE
endif

ifeq (${OS},aix)
	# AIX version
	BINDIR = ../../bin/aix
	LDPC_BINDIR = ../../bin/aix
	LIBRARIES = -lm -pthread -lc
	# TBC
	LARGEFILE_FLAGS = -D_FILE_OFFSET_BITS=64 -D_LARGEFILE_SOURCE
endif

ifeq (${OS},hp-ux)
	# HP-UX version
	BINDIR = ../../bin/hp-ux
	LDPC_BINDIR = ../../bin/hp-ux
	LIBRARIES = -lc -lm -pthread
	# TBC
	LARGEFILE_FLAGS = -D_FILE_OFFSET_BITS=64 -D_LARGEFILE_SOURCE
endif

ifeq (${OS},irix)
	# IRIX version
	BINDIR = ../../bin/irix
	LDPC_BINDIR = ../../bin/irix
	LIBRARIES = -lc -lm -pthread
	# TBC
	LARGEFILE_FLAGS = -D_FILE_OFFSET_BITS=64 -D_LARGEFILE_SOURCE
endif


###### Other variables ######

# Local CFLAGS.

# gcc version
#CFLAGS  = ${OSFLAGS} -D${RM_PROTOCOL} ${LARGEFILE_FLAGS} -g -pg -O
CFLAGS  = ${OSFLAGS} -D${RM_PROTOCOL} ${LARGEFILE_FLAGS} -g -O -DDEBUG
#CFLAGS  = ${OSFLAGS} -D${RM_PROTOCOL} ${LARGEFILE_FLAGS} -O

# add purify linker for memory debug (Solaris only).
# Purify 4.0 Solaris 2, Copyright (C) 1992-1996 Pure Software Inc.
#
#CCLINK_PURE = purify $(CC)
#CCLINK_PURE = purify  -cache-dir=/tmp $(CC)

# add quantify linker for profiling (Solaris only).
#CCLINK_QUANTIFY = quantify $(CC)

# list of source and object files
SRCFILES = checksum.c metadata.c filemanage.c fsend.c frecv.c fcast.c
OFILES	 = ${SRCFILES:.c=.o}


###### choose between the ALC / NORM versions ######

# Compile either with ALC or NORM support.
# The RM_PROTOCOL and MCL_LIBRARY variables are target-specific.
# In case of ALC, you may have to add libldpc.a depending on what
# FEC codec is used by MCL
# The tool is either fcast (ALC) or fcastn (NORM).

alc:	RM_PROTOCOL = ALC
# Edit the following two lines as appropriate, according to the FEC
# codec in use: if LDPC is used, then add the libbdpc.a library.
# If only RSE is used and if you don't have the LDPC archive, then
# remove the libbdpc.a library.
alc:	MCL_LIBRARY = ${BINDIR}/libmclalc.a
alc:	MCL_LIBRARY = ${BINDIR}/libmclalc.a ${LDPC_BINDIR}/libldpc.a
alc:	TARGET_FCAST = ${BINDIR}/fcast
alc:	TARGET_FCAST_STATIC = ${BINDIR}/fcast-static
#alc:	all

norm:	RM_PROTOCOL = NORM
norm:	MCL_LIBRARY = ${BINDIR}/libmclnorm.a
norm:	TARGET_FCAST = ${BINDIR}/fcastn
norm:	TARGET_FCAST_STATIC = ${BINDIR}/fcastn-static
#norm:	all

#alc_pure:	RM_PROTOCOL = ALC
# Edit the following two lines as appropriate, according to the FEC
# codec in use: if LDPC is used, then add the libbdpc.a library.
#alc_pure:	MCL_LIBRARY = ${BINDIR}/libmclalc.a
#alc_pure:	MCL_LIBRARY = ${BINDIR}/libmclalc.a ${LDPC_BINDIR}/libldpc.a
#alc_pure:	TARGET_FCAST = ${BINDIR}/fcast
#alc_pure:	pure

#alc_quantify:	RM_PROTOCOL = ALC
# Edit the following two lines as appropriate, according to the FEC
# codec in use: if LDPC is used, then add the libbdpc.a library.
#alc_quantify:	MCL_LIBRARY = ${BINDIR}/libmclalc.a
#alc_quantify:	MCL_LIBRARY = ${BINDIR}/libmclalc.a ${LDPC_BINDIR}/libldpc.a
#alc_quantify:	TARGET_FCAST = ${BINDIR}/fcast
#alc_quantify:	quantify

#norm_pure:	RM_PROTOCOL = NORM
#norm_pure:	MCL_LIBRARY = ${BINDIR}/libmclnorm.a
#norm_pure:	TARGET_FCAST = ${BINDIR}/fcastn
#norm_pure:	pure

#norm_quantify:	RM_PROTOCOL = NORM
#norm_quantify:	MCL_LIBRARY = ${BINDIR}/libmclnorm.a
#norm_quantify:	TARGET_FCAST = ${BINDIR}/fcastn
#norm_quantify:	quantify

###### Dependancies ######

all:	alc norm

#${TARGET_FCAST}:	${OFILES} ${MCL_LIBRARY}

alc:	${OFILES} ${MCL_LIBRARY}
	@echo "** Compiling fcast-ALC using the MCL library..."
	${CC} ${CFLAGS} ${OFILES} ${MCL_LIBRARY} ${LIBRARIES} -o ${TARGET_FCAST}
	${CC} -static  -static-libgcc ${CFLAGS} ${OFILES} ${MCL_LIBRARY} ${LIBRARIES} -o ${TARGET_FCAST_STATIC}

norm:	${OFILES} ${MCL_LIBRARY}
	@echo "** Compiling fcast-ALC using the MCL library..."
	${CC} ${CFLAGS} ${OFILES} ${MCL_LIBRARY} ${LIBRARIES} -o ${TARGET_FCAST}
	${CC} -static  -static-libgcc ${CFLAGS} ${OFILES} ${MCL_LIBRARY} ${LIBRARIES} -o ${TARGET_FCAST_STATIC}

#pure:	${OFILES} ${MCL_LIBRARY}
#	@echo "** Compiling fcast using the MCL library..."
#	${CCLINK_PURE} ${CFLAGS} ${OFILES} ${LIBRARIES} ${MCL_LIBRARY} -o ${TARGET_FCAST}

#quantify:	${OFILES} ${MCL_LIBRARY}
#	@echo "** Compiling fcast using the MCL library..."
#	${CCLINK_QUANTIFY} ${CFLAGS} ${OFILES} ${LIBRARIES} ${MCL_LIBRARY} -o ${TARGET_FCAST}

clean :
	@for f in core ${OFILES} *_pure* *.pure_* .pur* *purify* ; do \
		if [ -f "$${f}" ] ; then \
			rm "$${f}"; \
		fi \
	done

cleanall_alc : clean
	@for f in ${BINDIR}/fcast; do \
		if [ -f "$${f}" ] ; then \
			rm $${f} ; \
		fi \
	done


cleanall_norm : clean
	@for f in ${BINDIR}/fcastn; do \
		if [ -f "$${f}" ] ; then \
			rm $${f} ; \
		fi \
	done


cleanall : clean
	@for f in ${BINDIR}/fcast ${BINDIR}/fcastn; do \
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

checksum.o: checksum.h
metadata.o: fcast.h ../src/common/mcl_lib_api.h ../src/common/mcl_types.h
metadata.o: checksum.h metadata.h macros.h filemanage.h fsend.h frecv.h
filemanage.o: fcast.h ../src/common/mcl_lib_api.h ../src/common/mcl_types.h
filemanage.o: checksum.h metadata.h macros.h filemanage.h fsend.h frecv.h
fsend.o: fcast.h ../src/common/mcl_lib_api.h ../src/common/mcl_types.h
fsend.o: checksum.h metadata.h macros.h filemanage.h fsend.h frecv.h
frecv.o: fcast.h ../src/common/mcl_lib_api.h ../src/common/mcl_types.h
frecv.o: checksum.h metadata.h macros.h filemanage.h fsend.h frecv.h
fcast.o: fcast.h ../src/common/mcl_lib_api.h ../src/common/mcl_types.h
fcast.o: checksum.h metadata.h macros.h filemanage.h fsend.h frecv.h
