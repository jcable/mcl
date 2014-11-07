####### OS Dependant Definitions ######

# they are defined in the following file...
# edit Rules.make as appropriate
include ../../Makefile.common


# libraries required
ifeq (${OS},linux)
	# Linux version
	#
	BINDIR = ../../bin/linux
	LDPC_BINDIR = ../../../ldpc/bin/linux
	XERCES_DIR = ../../../xerces-c
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
	BINDIR = ../../bin/solaris
	LDPC_BINDIR = ../../../ldpc/bin/solaris
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
	LDPC_BINDIR = ../../../ldpc/bin/freebsd
	XERCES_DIR = ../../../xerces-c
	LIBRARIES = -lm -pthread -L/usr/local/lib -lcrypto
	#ADD_INCLUDES = -I/usr/local/include/ -I${XERCES_DIR}/include
	ADD_INCLUDES = -I${XERCES_DIR}/include
endif

ifeq (${OS},aix)
	# AIX version
	BINDIR = ../../bin/aix
	LDPC_BINDIR = ../../../ldpc/bin/aix
	XERCES_DIR = ../../../xerces-c
	LIBRARIES = -lm -pthread -lc -lcrypto
	ADD_INCLUDES = -I/usr/local/include/ -I${XERCES_DIR}/include
endif

ifeq (${OS},hp-ux)
	# HP-UX version
	BINDIR = ../../bin/hp-ux
	LDPC_BINDIR = ../../../ldpc/bin/hp-ux
	XERCES_DIR = ../../../xerces-c
	LIBRARIES = -lc -lm -pthread -lcrypto
	ADD_INCLUDES = -I/usr/local/include/ -I${XERCES_DIR}/include
endif

ifeq (${OS},irix)
	# IRIX version
	BINDIR = ../../bin/irix
	LDPC_BINDIR = ../../../ldpc/bin/irix
	XERCES_DIR = ../../../xerces-c
	LIBRARIES = -lc -lm -pthread -lcrypto
	ADD_INCLUDES = -I/usr/local/include/ -I${XERCES_DIR}/include
endif


###### Other variables ######

# Local CFLAGS.

# gcc version
#CFLAGS  = -g -pg -O ${OSFLAGS} -DALC ${ADD_INCLUDES}
CFLAGS  = -g -O -DDEBUG ${OSFLAGS} -DALC ${ADD_INCLUDES}
#CFLAGS  = -O ${OSFLAGS} -DALC ${ADD_INCLUDES}
#static:	CFLAGS = ${CFLAGS} -static  -static-libgcc

# add purify linker for memory debug (Solaris only).
# Purify 4.0 Solaris 2, Copyright (C) 1992-1996 Pure Software Inc.
#
#CCLINK_PURE = purify $(CC)
CCLINK_PURE = purify  -cache-dir=/tmp $(CC)

# add quantify linker for profiling (Solaris only).
CCLINK_QUANTIFY = quantify $(CC)

# list of source and object files
SRCFILES = Flute.cpp FluteReceiver.cpp FluteSender.cpp FluteFileInfo.cpp FluteFileDeliveryCarousel.cpp FluteFile.cpp FluteFileList.cpp FluteFec.cpp flute_cb.cpp FluteFDT.cpp flute_md5.cpp flute_lock.cpp FluteSDP.cpp FluteTools.cpp
OFILES	 = ${SRCFILES:.cpp=.o}

#OS flag with upper case chars
OS_UPPER_CASE = `echo ${OS} | tr 'a-z' 'A-Z'`

# XML parsing library
static:  XERCES_LIBRARY := ${XERCES_DIR}/lib/libxerces-c.a

###### Dependancies ######

# by default, build the dynamic FLUTE version...
all:	lib

static: xerces_lib all


# Rule for C++ .cpp extension
.cpp.o:
	$(CC) -c $(CFLAGS) -o $*.o $*.cpp

lib : ${OFILES} ${MCL_LIBRARY} ${SDP_LIBRARY}
	@echo "** Now create the FLUTE library..."
	#
	# add the libldpc.a the xerces and the sdp library
	${AR} -rcsv ${BINDIR}/libflute.a ${OFILES}
	${RANLIB} ${BINDIR}/libflute.a	
	@echo "done"


xerces_lib : ${XERCES_LIBRARY}
	@echo "** Now create the Xerces library..."
	@for f in ${XERCES_DIR}/lib/libxerces-c.a; do \
		if [ -f "$${f}" ] ; then \
			echo "libxerces-c.a already exists (I will remove it and rebuilt it)"; \
			rm $${f} ; \
                        ${AR} -rcsv ${XERCES_DIR}/lib/libxerces-c.a ${XERCES_DIR}/obj/${OS_UPPER_CASE}/*.o; \
                        ${RANLIB} ${XERCES_DIR}/lib/libxerces-c.a; \
		else \
			${AR} -rcsv ${XERCES_DIR}/lib/libxerces-c.a ${XERCES_DIR}/obj/${OS_UPPER_CASE}/*.o; \
			${RANLIB} ${XERCES_DIR}/lib/libxerces-c.a; \
		fi \
	done

	
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

Flute.o: flute_includes.h FluteAPI.h ../common/mcl_lib_api.h
Flute.o: ../common/mcl_types.h ../alc/mcl_lib_api_alc.h Flute.h
Flute.o: FluteReceiver.h FluteSender.h FluteFileInfo.h
Flute.o: FluteFileDeliveryCarousel.h macros.h FluteTools.h flute_lock.h
Flute.o: FluteFDT.h FluteSDP.h ../sdp_lib/src/SDP_Parser.h
Flute.o: ../sdp_lib/src/SDP_Error.h ../sdp_lib/src/SDP_EventStreamParser.h
Flute.o: ../sdp_lib/src/SDP_StreamTokenizer.h ../sdp_lib/src/SDP_Utility.h
Flute.o: ../sdp_lib/src/SDP_Description.h ../sdp_lib/src/SDP_Str.h
Flute.o: ../sdp_lib/src/SDP_LinkedList.h ../sdp_lib/src/SDP_Generator.h
Flute.o: ../sdp_lib/src/SDP_Generator.h flute_md5.h flute_cb.h FluteFec.h
Flute.o: FluteFile.h FluteFileList.h
FluteReceiver.o: flute_includes.h FluteAPI.h ../common/mcl_lib_api.h
FluteReceiver.o: ../common/mcl_types.h ../alc/mcl_lib_api_alc.h Flute.h
FluteReceiver.o: FluteReceiver.h FluteSender.h FluteFileInfo.h
FluteReceiver.o: FluteFileDeliveryCarousel.h macros.h FluteTools.h
FluteReceiver.o: flute_lock.h FluteFDT.h FluteSDP.h
FluteReceiver.o: ../sdp_lib/src/SDP_Parser.h ../sdp_lib/src/SDP_Error.h
FluteReceiver.o: ../sdp_lib/src/SDP_EventStreamParser.h
FluteReceiver.o: ../sdp_lib/src/SDP_StreamTokenizer.h
FluteReceiver.o: ../sdp_lib/src/SDP_Utility.h
FluteReceiver.o: ../sdp_lib/src/SDP_Description.h ../sdp_lib/src/SDP_Str.h
FluteReceiver.o: ../sdp_lib/src/SDP_LinkedList.h
FluteReceiver.o: ../sdp_lib/src/SDP_Generator.h
FluteReceiver.o: ../sdp_lib/src/SDP_Generator.h flute_md5.h flute_cb.h
FluteReceiver.o: FluteFec.h FluteFile.h FluteFileList.h
FluteSender.o: flute_includes.h FluteAPI.h ../common/mcl_lib_api.h
FluteSender.o: ../common/mcl_types.h ../alc/mcl_lib_api_alc.h Flute.h
FluteSender.o: FluteReceiver.h FluteSender.h FluteFileInfo.h
FluteSender.o: FluteFileDeliveryCarousel.h macros.h FluteTools.h flute_lock.h
FluteSender.o: FluteFDT.h FluteSDP.h ../sdp_lib/src/SDP_Parser.h
FluteSender.o: ../sdp_lib/src/SDP_Error.h
FluteSender.o: ../sdp_lib/src/SDP_EventStreamParser.h
FluteSender.o: ../sdp_lib/src/SDP_StreamTokenizer.h
FluteSender.o: ../sdp_lib/src/SDP_Utility.h ../sdp_lib/src/SDP_Description.h
FluteSender.o: ../sdp_lib/src/SDP_Str.h ../sdp_lib/src/SDP_LinkedList.h
FluteSender.o: ../sdp_lib/src/SDP_Generator.h ../sdp_lib/src/SDP_Generator.h
FluteSender.o: flute_md5.h flute_cb.h FluteFec.h FluteFile.h FluteFileList.h
FluteFileInfo.o: flute_includes.h FluteAPI.h ../common/mcl_lib_api.h
FluteFileInfo.o: ../common/mcl_types.h ../alc/mcl_lib_api_alc.h Flute.h
FluteFileInfo.o: FluteReceiver.h FluteSender.h FluteFileInfo.h
FluteFileInfo.o: FluteFileDeliveryCarousel.h macros.h FluteTools.h
FluteFileInfo.o: flute_lock.h FluteFDT.h FluteSDP.h
FluteFileInfo.o: ../sdp_lib/src/SDP_Parser.h ../sdp_lib/src/SDP_Error.h
FluteFileInfo.o: ../sdp_lib/src/SDP_EventStreamParser.h
FluteFileInfo.o: ../sdp_lib/src/SDP_StreamTokenizer.h
FluteFileInfo.o: ../sdp_lib/src/SDP_Utility.h
FluteFileInfo.o: ../sdp_lib/src/SDP_Description.h ../sdp_lib/src/SDP_Str.h
FluteFileInfo.o: ../sdp_lib/src/SDP_LinkedList.h
FluteFileInfo.o: ../sdp_lib/src/SDP_Generator.h
FluteFileInfo.o: ../sdp_lib/src/SDP_Generator.h flute_md5.h flute_cb.h
FluteFileInfo.o: FluteFec.h FluteFile.h FluteFileList.h
FluteFileDeliveryCarousel.o: flute_includes.h FluteAPI.h
FluteFileDeliveryCarousel.o: ../common/mcl_lib_api.h ../common/mcl_types.h
FluteFileDeliveryCarousel.o: ../alc/mcl_lib_api_alc.h Flute.h FluteReceiver.h
FluteFileDeliveryCarousel.o: FluteSender.h FluteFileInfo.h
FluteFileDeliveryCarousel.o: FluteFileDeliveryCarousel.h macros.h
FluteFileDeliveryCarousel.o: FluteTools.h flute_lock.h FluteFDT.h FluteSDP.h
FluteFileDeliveryCarousel.o: ../sdp_lib/src/SDP_Parser.h
FluteFileDeliveryCarousel.o: ../sdp_lib/src/SDP_Error.h
FluteFileDeliveryCarousel.o: ../sdp_lib/src/SDP_EventStreamParser.h
FluteFileDeliveryCarousel.o: ../sdp_lib/src/SDP_StreamTokenizer.h
FluteFileDeliveryCarousel.o: ../sdp_lib/src/SDP_Utility.h
FluteFileDeliveryCarousel.o: ../sdp_lib/src/SDP_Description.h
FluteFileDeliveryCarousel.o: ../sdp_lib/src/SDP_Str.h
FluteFileDeliveryCarousel.o: ../sdp_lib/src/SDP_LinkedList.h
FluteFileDeliveryCarousel.o: ../sdp_lib/src/SDP_Generator.h
FluteFileDeliveryCarousel.o: ../sdp_lib/src/SDP_Generator.h flute_md5.h
FluteFileDeliveryCarousel.o: flute_cb.h FluteFec.h FluteFile.h
FluteFileDeliveryCarousel.o: FluteFileList.h
FluteFile.o: flute_includes.h FluteAPI.h ../common/mcl_lib_api.h
FluteFile.o: ../common/mcl_types.h ../alc/mcl_lib_api_alc.h Flute.h
FluteFile.o: FluteReceiver.h FluteSender.h FluteFileInfo.h
FluteFile.o: FluteFileDeliveryCarousel.h macros.h FluteTools.h flute_lock.h
FluteFile.o: FluteFDT.h FluteSDP.h ../sdp_lib/src/SDP_Parser.h
FluteFile.o: ../sdp_lib/src/SDP_Error.h
FluteFile.o: ../sdp_lib/src/SDP_EventStreamParser.h
FluteFile.o: ../sdp_lib/src/SDP_StreamTokenizer.h
FluteFile.o: ../sdp_lib/src/SDP_Utility.h ../sdp_lib/src/SDP_Description.h
FluteFile.o: ../sdp_lib/src/SDP_Str.h ../sdp_lib/src/SDP_LinkedList.h
FluteFile.o: ../sdp_lib/src/SDP_Generator.h ../sdp_lib/src/SDP_Generator.h
FluteFile.o: flute_md5.h flute_cb.h FluteFec.h FluteFile.h FluteFileList.h
FluteFileList.o: flute_includes.h FluteAPI.h ../common/mcl_lib_api.h
FluteFileList.o: ../common/mcl_types.h ../alc/mcl_lib_api_alc.h Flute.h
FluteFileList.o: FluteReceiver.h FluteSender.h FluteFileInfo.h
FluteFileList.o: FluteFileDeliveryCarousel.h macros.h FluteTools.h
FluteFileList.o: flute_lock.h FluteFDT.h FluteSDP.h
FluteFileList.o: ../sdp_lib/src/SDP_Parser.h ../sdp_lib/src/SDP_Error.h
FluteFileList.o: ../sdp_lib/src/SDP_EventStreamParser.h
FluteFileList.o: ../sdp_lib/src/SDP_StreamTokenizer.h
FluteFileList.o: ../sdp_lib/src/SDP_Utility.h
FluteFileList.o: ../sdp_lib/src/SDP_Description.h ../sdp_lib/src/SDP_Str.h
FluteFileList.o: ../sdp_lib/src/SDP_LinkedList.h
FluteFileList.o: ../sdp_lib/src/SDP_Generator.h
FluteFileList.o: ../sdp_lib/src/SDP_Generator.h flute_md5.h flute_cb.h
FluteFileList.o: FluteFec.h FluteFile.h FluteFileList.h
FluteFec.o: flute_includes.h FluteAPI.h ../common/mcl_lib_api.h
FluteFec.o: ../common/mcl_types.h ../alc/mcl_lib_api_alc.h Flute.h
FluteFec.o: FluteReceiver.h FluteSender.h FluteFileInfo.h
FluteFec.o: FluteFileDeliveryCarousel.h macros.h FluteTools.h flute_lock.h
FluteFec.o: FluteFDT.h FluteSDP.h ../sdp_lib/src/SDP_Parser.h
FluteFec.o: ../sdp_lib/src/SDP_Error.h ../sdp_lib/src/SDP_EventStreamParser.h
FluteFec.o: ../sdp_lib/src/SDP_StreamTokenizer.h ../sdp_lib/src/SDP_Utility.h
FluteFec.o: ../sdp_lib/src/SDP_Description.h ../sdp_lib/src/SDP_Str.h
FluteFec.o: ../sdp_lib/src/SDP_LinkedList.h ../sdp_lib/src/SDP_Generator.h
FluteFec.o: ../sdp_lib/src/SDP_Generator.h flute_md5.h flute_cb.h FluteFec.h
FluteFec.o: FluteFile.h FluteFileList.h
flute_cb.o: flute_includes.h FluteAPI.h ../common/mcl_lib_api.h
flute_cb.o: ../common/mcl_types.h ../alc/mcl_lib_api_alc.h Flute.h
flute_cb.o: FluteReceiver.h FluteSender.h FluteFileInfo.h
flute_cb.o: FluteFileDeliveryCarousel.h macros.h FluteTools.h flute_lock.h
flute_cb.o: FluteFDT.h FluteSDP.h ../sdp_lib/src/SDP_Parser.h
flute_cb.o: ../sdp_lib/src/SDP_Error.h ../sdp_lib/src/SDP_EventStreamParser.h
flute_cb.o: ../sdp_lib/src/SDP_StreamTokenizer.h ../sdp_lib/src/SDP_Utility.h
flute_cb.o: ../sdp_lib/src/SDP_Description.h ../sdp_lib/src/SDP_Str.h
flute_cb.o: ../sdp_lib/src/SDP_LinkedList.h ../sdp_lib/src/SDP_Generator.h
flute_cb.o: ../sdp_lib/src/SDP_Generator.h flute_md5.h flute_cb.h FluteFec.h
flute_cb.o: FluteFile.h FluteFileList.h
FluteFDT.o: flute_includes.h FluteAPI.h ../common/mcl_lib_api.h
FluteFDT.o: ../common/mcl_types.h ../alc/mcl_lib_api_alc.h Flute.h
FluteFDT.o: FluteReceiver.h FluteSender.h FluteFileInfo.h
FluteFDT.o: FluteFileDeliveryCarousel.h macros.h FluteTools.h flute_lock.h
FluteFDT.o: FluteFDT.h FluteSDP.h ../sdp_lib/src/SDP_Parser.h
FluteFDT.o: ../sdp_lib/src/SDP_Error.h ../sdp_lib/src/SDP_EventStreamParser.h
FluteFDT.o: ../sdp_lib/src/SDP_StreamTokenizer.h ../sdp_lib/src/SDP_Utility.h
FluteFDT.o: ../sdp_lib/src/SDP_Description.h ../sdp_lib/src/SDP_Str.h
FluteFDT.o: ../sdp_lib/src/SDP_LinkedList.h ../sdp_lib/src/SDP_Generator.h
FluteFDT.o: ../sdp_lib/src/SDP_Generator.h flute_md5.h flute_cb.h FluteFec.h
FluteFDT.o: FluteFile.h FluteFileList.h
flute_md5.o: flute_includes.h FluteAPI.h ../common/mcl_lib_api.h
flute_md5.o: ../common/mcl_types.h ../alc/mcl_lib_api_alc.h Flute.h
flute_md5.o: FluteReceiver.h FluteSender.h FluteFileInfo.h
flute_md5.o: FluteFileDeliveryCarousel.h macros.h FluteTools.h flute_lock.h
flute_md5.o: FluteFDT.h FluteSDP.h ../sdp_lib/src/SDP_Parser.h
flute_md5.o: ../sdp_lib/src/SDP_Error.h
flute_md5.o: ../sdp_lib/src/SDP_EventStreamParser.h
flute_md5.o: ../sdp_lib/src/SDP_StreamTokenizer.h
flute_md5.o: ../sdp_lib/src/SDP_Utility.h ../sdp_lib/src/SDP_Description.h
flute_md5.o: ../sdp_lib/src/SDP_Str.h ../sdp_lib/src/SDP_LinkedList.h
flute_md5.o: ../sdp_lib/src/SDP_Generator.h ../sdp_lib/src/SDP_Generator.h
flute_md5.o: flute_md5.h flute_cb.h FluteFec.h FluteFile.h FluteFileList.h
flute_lock.o: flute_includes.h FluteAPI.h ../common/mcl_lib_api.h
flute_lock.o: ../common/mcl_types.h ../alc/mcl_lib_api_alc.h Flute.h
flute_lock.o: FluteReceiver.h FluteSender.h FluteFileInfo.h
flute_lock.o: FluteFileDeliveryCarousel.h macros.h FluteTools.h flute_lock.h
flute_lock.o: FluteFDT.h FluteSDP.h ../sdp_lib/src/SDP_Parser.h
flute_lock.o: ../sdp_lib/src/SDP_Error.h
flute_lock.o: ../sdp_lib/src/SDP_EventStreamParser.h
flute_lock.o: ../sdp_lib/src/SDP_StreamTokenizer.h
flute_lock.o: ../sdp_lib/src/SDP_Utility.h ../sdp_lib/src/SDP_Description.h
flute_lock.o: ../sdp_lib/src/SDP_Str.h ../sdp_lib/src/SDP_LinkedList.h
flute_lock.o: ../sdp_lib/src/SDP_Generator.h ../sdp_lib/src/SDP_Generator.h
flute_lock.o: flute_md5.h flute_cb.h FluteFec.h FluteFile.h FluteFileList.h
FluteSDP.o: flute_includes.h FluteAPI.h ../common/mcl_lib_api.h
FluteSDP.o: ../common/mcl_types.h ../alc/mcl_lib_api_alc.h Flute.h
FluteSDP.o: FluteReceiver.h FluteSender.h FluteFileInfo.h
FluteSDP.o: FluteFileDeliveryCarousel.h macros.h FluteTools.h flute_lock.h
FluteSDP.o: FluteFDT.h FluteSDP.h ../sdp_lib/src/SDP_Parser.h
FluteSDP.o: ../sdp_lib/src/SDP_Error.h ../sdp_lib/src/SDP_EventStreamParser.h
FluteSDP.o: ../sdp_lib/src/SDP_StreamTokenizer.h ../sdp_lib/src/SDP_Utility.h
FluteSDP.o: ../sdp_lib/src/SDP_Description.h ../sdp_lib/src/SDP_Str.h
FluteSDP.o: ../sdp_lib/src/SDP_LinkedList.h ../sdp_lib/src/SDP_Generator.h
FluteSDP.o: ../sdp_lib/src/SDP_Generator.h flute_md5.h flute_cb.h FluteFec.h
FluteSDP.o: FluteFile.h FluteFileList.h
FluteTools.o: flute_includes.h FluteAPI.h ../common/mcl_lib_api.h
FluteTools.o: ../common/mcl_types.h ../alc/mcl_lib_api_alc.h Flute.h
FluteTools.o: FluteReceiver.h FluteSender.h FluteFileInfo.h
FluteTools.o: FluteFileDeliveryCarousel.h macros.h FluteTools.h flute_lock.h
FluteTools.o: FluteFDT.h FluteSDP.h ../sdp_lib/src/SDP_Parser.h
FluteTools.o: ../sdp_lib/src/SDP_Error.h
FluteTools.o: ../sdp_lib/src/SDP_EventStreamParser.h
FluteTools.o: ../sdp_lib/src/SDP_StreamTokenizer.h
FluteTools.o: ../sdp_lib/src/SDP_Utility.h ../sdp_lib/src/SDP_Description.h
FluteTools.o: ../sdp_lib/src/SDP_Str.h ../sdp_lib/src/SDP_LinkedList.h
FluteTools.o: ../sdp_lib/src/SDP_Generator.h ../sdp_lib/src/SDP_Generator.h
FluteTools.o: flute_md5.h flute_cb.h FluteFec.h FluteFile.h FluteFileList.h
