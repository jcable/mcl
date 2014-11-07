# $Id: Makefile,v 1.29 2005/01/11 13:12:24 roca Exp $
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
	# MCL library requires -lpthread
	# add MPATROL support if required
	#
	BINDIR = ../bin/linux
	LIBRARIES = -lc -lm -lpthread
endif

ifeq (${OS},solaris)
	# Solaris version: requires -lsocket -lnsl and -L/usr/ucblib -lucb
	# nb: to avoid LD_LIBRARY_PATH problems, specify /usr/ucblib/libucb.a
	# directly and use the static version!
	#
	BINDIR = ../bin/solaris
	#LIBRARIES = -lc -lm -lsocket -lnsl -L/usr/ucblib -lucb -lposix4 -lpthread
	LIBRARIES = -lc -lm -lsocket -lnsl /usr/ucblib/libucb.a -lposix4 -lpthread
endif

ifeq (${OS},freebsd)
	# FreeBSD version
	# MCL library requires -pthread
	#
	BINDIR = ../bin/freebsd
	LIBRARIES = -lm -pthread
endif



###### Other variables ######

# Local CFLAGS.

# gcc additional compilation flags
# -g to create additional debug info used by the gdb debugger
# -static to avoid dynamic libraries
# -pg for profiling with gprof
#
CFLAGS  = -O -g ${OSFLAGS} -D${RM_PROTOCOL}
#CFLAGS  = -O -g ${OSFLAGS} -D${RM_PROTOCOL} -pg
#CFLAGS  = -O -g ${OSFLAGS} -D${RM_PROTOCOL} -static

# add purify linker for memory debug (Solaris only).
# Purify 4.0 Solaris 2, Copyright (C) 1992-1996 Pure Software Inc.
#
#CCLINK_PURE = purify ${CC}
#CCLINK_PURE = purify -cache-dir=${HOME}/pcache ${CC}
CCLINK_PURE = purify -cache-dir=/tmp ${CC}
# to use pufigy, you only need to uncomment the following line which
# overrides ${CC}!
#${CC} = ${CCLINK_PURE}

#SRCFILES = mclrecv.c mclsend.c mclrecv2.c mclsend2.c
SRCFILES = mclrecv.c mclsend.c
OFILES	 = ${SRCFILES:.c=.o}


###### choose between the ALC / NORM versions ######

# compile either with ALC or NORM support.
# the RM_PROTOCOL and MCL_LIBRARY variables are target-specific 

alc:	RM_PROTOCOL = ALC
alc:	MCL_LIBRARY = ${BINDIR}/libmclalc.a
alc:	all

norm:	RM_PROTOCOL = NORM
norm:	MCL_LIBRARY = ${BINDIR}/libmclnorm.a
norm:	all


###### Dependancies ######

#all:	${BINDIR}/mclrecv ${BINDIR}/mclsend ${BINDIR}/mclrecv2 ${BINDIR}/mclsend2
all:	${BINDIR}/mclsend ${BINDIR}/mclrecv 

${BINDIR}/mclsend:	mclsend.o ${MCL_LIBRARY}
	@echo "** Compile mclsend using the MCL-${RM_PROTOCOL} library ${MCL_LIBRARY}..."
	#${CCLINK_PURE} ${CFLAGS} mclsend.o ${LIBRARIES} ${MCL_LIBRARY} -o ${BINDIR}/mclsend
	${CC} ${CFLAGS} mclsend.o ${LIBRARIES} ${MCL_LIBRARY} -o ${BINDIR}/mclsend

${BINDIR}/mclrecv:	mclrecv.o ${MCL_LIBRARY}
	@echo "** Compile mclrecv using the MCL-${RM_PROTOCOL} library ${MCL_LIBRARY}..."
	#${CCLINK_PURE} ${CFLAGS} mclrecv.o ${LIBRARIES} ${MCL_LIBRARY} -o ${BINDIR}/mclrecv
	${CC} ${CFLAGS} mclrecv.o ${LIBRARIES} ${MCL_LIBRARY} -o ${BINDIR}/mclrecv

#${BINDIR}/mclsend2:	mclsend2.o ${MCL_LIBRARY}
#	@echo "** Compile mclsend2 using the MCL-${RM_PROTOCOL} library..."
#	${CC} ${CFLAGS} mclsend2.o ${LIBRARIES} ${MCL_LIBRARY} -o ${BINDIR}/mclsend2

#${BINDIR}/mclrecv2:	mclrecv2.o ${MCL_LIBRARY}
#	@echo "** Compile mclrecv using the MCL-${RM_PROTOCOL} library..."
#	${CC} ${CFLAGS} mclrecv2.o ${LIBRARIES} ${MCL_LIBRARY} -o ${BINDIR}/mclrecv2


clean :
	@for f in core ${BINDIR}/core ${OFILES} ; do \
		if [ -f "$${f}" ] ; then \
			rm "$${f}"; \
		fi \
	done

cleanall : clean
	@for f in ${BINDIR}/mclsend ${BINDIR}/mclrecv ${BINDIR}/mclsend2 ${BINDIR}/mclrecv2 ; do \
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

mclrecv.o: mclftp.h /usr/include/stdio.h /usr/include/features.h
mclrecv.o: /usr/include/sys/cdefs.h /usr/include/gnu/stubs.h
mclrecv.o: /usr/lib/gcc/i386-redhat-linux/3.4.2/include/stddef.h
mclrecv.o: /usr/include/bits/types.h /usr/include/bits/wordsize.h
mclrecv.o: /usr/include/bits/typesizes.h /usr/include/libio.h
mclrecv.o: /usr/include/_G_config.h /usr/include/wchar.h
mclrecv.o: /usr/include/bits/wchar.h /usr/include/gconv.h
mclrecv.o: /usr/lib/gcc/i386-redhat-linux/3.4.2/include/stdarg.h
mclrecv.o: /usr/include/bits/stdio_lim.h /usr/include/bits/sys_errlist.h
mclrecv.o: /usr/include/stdlib.h /usr/include/errno.h
mclrecv.o: /usr/include/bits/errno.h /usr/include/linux/errno.h
mclrecv.o: /usr/include/asm/errno.h /usr/include/ctype.h
mclrecv.o: /usr/include/endian.h /usr/include/bits/endian.h
mclrecv.o: /usr/include/string.h /usr/include/sys/types.h /usr/include/time.h
mclrecv.o: /usr/include/sys/stat.h /usr/include/bits/stat.h
mclrecv.o: /usr/include/fcntl.h /usr/include/bits/fcntl.h
mclrecv.o: /usr/include/unistd.h /usr/include/bits/posix_opt.h
mclrecv.o: /usr/include/bits/confname.h /usr/include/netdb.h
mclrecv.o: /usr/include/netinet/in.h /usr/include/stdint.h
mclrecv.o: /usr/include/sys/socket.h /usr/include/sys/uio.h
mclrecv.o: /usr/include/bits/uio.h /usr/include/bits/socket.h
mclrecv.o: /usr/include/limits.h
mclrecv.o: /usr/lib/gcc/i386-redhat-linux/3.4.2/include/limits.h
mclrecv.o: /usr/include/bits/sockaddr.h /usr/include/asm/socket.h
mclrecv.o: /usr/include/asm/sockios.h /usr/include/bits/in.h
mclrecv.o: /usr/include/bits/byteswap.h /usr/include/bits/netdb.h
mclrecv.o: /usr/include/arpa/inet.h ../src/common/mcl_lib_api.h
mclrecv.o: ../src/common/mcl_types.h
mclsend.o: mclftp.h /usr/include/stdio.h /usr/include/features.h
mclsend.o: /usr/include/sys/cdefs.h /usr/include/gnu/stubs.h
mclsend.o: /usr/lib/gcc/i386-redhat-linux/3.4.2/include/stddef.h
mclsend.o: /usr/include/bits/types.h /usr/include/bits/wordsize.h
mclsend.o: /usr/include/bits/typesizes.h /usr/include/libio.h
mclsend.o: /usr/include/_G_config.h /usr/include/wchar.h
mclsend.o: /usr/include/bits/wchar.h /usr/include/gconv.h
mclsend.o: /usr/lib/gcc/i386-redhat-linux/3.4.2/include/stdarg.h
mclsend.o: /usr/include/bits/stdio_lim.h /usr/include/bits/sys_errlist.h
mclsend.o: /usr/include/stdlib.h /usr/include/errno.h
mclsend.o: /usr/include/bits/errno.h /usr/include/linux/errno.h
mclsend.o: /usr/include/asm/errno.h /usr/include/ctype.h
mclsend.o: /usr/include/endian.h /usr/include/bits/endian.h
mclsend.o: /usr/include/string.h /usr/include/sys/types.h /usr/include/time.h
mclsend.o: /usr/include/sys/stat.h /usr/include/bits/stat.h
mclsend.o: /usr/include/fcntl.h /usr/include/bits/fcntl.h
mclsend.o: /usr/include/unistd.h /usr/include/bits/posix_opt.h
mclsend.o: /usr/include/bits/confname.h /usr/include/netdb.h
mclsend.o: /usr/include/netinet/in.h /usr/include/stdint.h
mclsend.o: /usr/include/sys/socket.h /usr/include/sys/uio.h
mclsend.o: /usr/include/bits/uio.h /usr/include/bits/socket.h
mclsend.o: /usr/include/limits.h
mclsend.o: /usr/lib/gcc/i386-redhat-linux/3.4.2/include/limits.h
mclsend.o: /usr/include/bits/sockaddr.h /usr/include/asm/socket.h
mclsend.o: /usr/include/asm/sockios.h /usr/include/bits/in.h
mclsend.o: /usr/include/bits/byteswap.h /usr/include/bits/netdb.h
mclsend.o: /usr/include/arpa/inet.h ../src/common/mcl_lib_api.h
mclsend.o: ../src/common/mcl_types.h
