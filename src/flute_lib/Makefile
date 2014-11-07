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

Flute.o: flute_includes.h /usr/local/include/xercesc/util/PlatformUtils.hpp
Flute.o: /usr/local/include/xercesc/util/XMLException.hpp
Flute.o: /usr/local/include/xercesc/util/XMemory.hpp
Flute.o: /usr/local/include/xercesc/util/XercesDefs.hpp
Flute.o: /usr/local/include/xercesc/util/XercesVersion.hpp
Flute.o: /usr/local/include/xercesc/util/AutoSense.hpp
Flute.o: /usr/local/include/xercesc/util/XMLExceptMsgs.hpp
Flute.o: /usr/local/include/xercesc/dom/DOMError.hpp
Flute.o: /usr/local/include/xercesc/util/XMLUni.hpp
Flute.o: /usr/local/include/xercesc/framework/XMLErrorReporter.hpp
Flute.o: /usr/local/include/xercesc/util/PanicHandler.hpp
Flute.o: /usr/local/include/xercesc/dom/DOM.hpp
Flute.o: /usr/local/include/xercesc/dom/DOMAttr.hpp
Flute.o: /usr/local/include/xercesc/dom/DOMNode.hpp
Flute.o: /usr/local/include/xercesc/dom/DOMCDATASection.hpp
Flute.o: /usr/local/include/xercesc/dom/DOMText.hpp
Flute.o: /usr/local/include/xercesc/dom/DOMCharacterData.hpp
Flute.o: /usr/local/include/xercesc/dom/DOMComment.hpp
Flute.o: /usr/local/include/xercesc/dom/DOMDocument.hpp
Flute.o: /usr/local/include/xercesc/dom/DOMDocumentRange.hpp
Flute.o: /usr/local/include/xercesc/dom/DOMDocumentTraversal.hpp
Flute.o: /usr/local/include/xercesc/dom/DOMXPathEvaluator.hpp
Flute.o: /usr/local/include/xercesc/dom/DOMDocumentFragment.hpp
Flute.o: /usr/local/include/xercesc/dom/DOMDocumentType.hpp
Flute.o: /usr/local/include/xercesc/dom/DOMElement.hpp
Flute.o: /usr/local/include/xercesc/dom/DOMEntity.hpp
Flute.o: /usr/local/include/xercesc/dom/DOMEntityReference.hpp
Flute.o: /usr/local/include/xercesc/dom/DOMException.hpp
Flute.o: /usr/local/include/xercesc/dom/DOMImplementation.hpp
Flute.o: /usr/local/include/xercesc/dom/DOMImplementationLS.hpp
Flute.o: /usr/local/include/xercesc/dom/DOMRangeException.hpp
Flute.o: /usr/local/include/xercesc/dom/DOMNamedNodeMap.hpp
Flute.o: /usr/local/include/xercesc/dom/DOMNodeList.hpp
Flute.o: /usr/local/include/xercesc/dom/DOMNotation.hpp
Flute.o: /usr/local/include/xercesc/dom/DOMProcessingInstruction.hpp
Flute.o: /usr/local/include/xercesc/dom/DOMNodeFilter.hpp
Flute.o: /usr/local/include/xercesc/dom/DOMNodeIterator.hpp
Flute.o: /usr/local/include/xercesc/dom/DOMRange.hpp
Flute.o: /usr/local/include/xercesc/dom/DOMTreeWalker.hpp
Flute.o: /usr/local/include/xercesc/dom/DOMBuilder.hpp
Flute.o: /usr/local/include/xercesc/dom/DOMConfiguration.hpp
Flute.o: /usr/local/include/xercesc/util/XMLString.hpp
Flute.o: /usr/local/include/xercesc/util/BaseRefVectorOf.hpp
Flute.o: /usr/local/include/xercesc/util/ArrayIndexOutOfBoundsException.hpp
Flute.o: /usr/local/include/xercesc/util/XMLEnumerator.hpp
Flute.o: /usr/local/include/xercesc/framework/MemoryManager.hpp
Flute.o: /usr/local/include/xercesc/util/BaseRefVectorOf.c
Flute.o: /usr/local/include/xercesc/framework/XMLBuffer.hpp
Flute.o: /usr/local/include/xercesc/dom/DOMEntityResolver.hpp
Flute.o: /usr/local/include/xercesc/dom/DOMErrorHandler.hpp
Flute.o: /usr/local/include/xercesc/dom/DOMImplementationRegistry.hpp
Flute.o: /usr/local/include/xercesc/dom/DOMImplementationSource.hpp
Flute.o: /usr/local/include/xercesc/dom/DOMInputSource.hpp
Flute.o: /usr/local/include/xercesc/dom/DOMLocator.hpp
Flute.o: /usr/local/include/xercesc/dom/DOMTypeInfo.hpp
Flute.o: /usr/local/include/xercesc/dom/DOMUserDataHandler.hpp
Flute.o: /usr/local/include/xercesc/dom/DOMWriter.hpp
Flute.o: /usr/local/include/xercesc/dom/DOMWriterFilter.hpp
Flute.o: /usr/local/include/xercesc/framework/XMLFormatter.hpp
Flute.o: /usr/local/include/xercesc/dom/DOMXPathNSResolver.hpp
Flute.o: /usr/local/include/xercesc/dom/DOMXPathException.hpp
Flute.o: /usr/local/include/xercesc/dom/DOMXPathExpression.hpp
Flute.o: /usr/local/include/xercesc/dom/DOMXPathResult.hpp
Flute.o: /usr/local/include/xercesc/dom/DOMXPathNamespace.hpp
Flute.o: /usr/local/include/xercesc/framework/Wrapper4InputSource.hpp
Flute.o: /usr/local/include/xercesc/framework/MemBufInputSource.hpp
Flute.o: /usr/local/include/xercesc/sax/InputSource.hpp
Flute.o: /usr/local/include/xercesc/framework/MemBufFormatTarget.hpp
Flute.o: FluteAPI.h ../common/mcl_lib_api.h ../common/mcl_types.h
Flute.o: ../alc/mcl_lib_api_alc.h Flute.h FluteReceiver.h FluteSender.h
Flute.o: FluteFileInfo.h FluteFileDeliveryCarousel.h macros.h FluteTools.h
Flute.o: flute_lock.h FluteFDT.h FluteSDP.h ../sdp_lib/src/SDP_Parser.h
Flute.o: ../sdp_lib/src/SDP_Error.h ../sdp_lib/src/SDP_EventStreamParser.h
Flute.o: ../sdp_lib/src/SDP_StreamTokenizer.h ../sdp_lib/src/SDP_Utility.h
Flute.o: ../sdp_lib/src/SDP_Description.h ../sdp_lib/src/SDP_Str.h
Flute.o: ../sdp_lib/src/SDP_LinkedList.h ../sdp_lib/src/SDP_Generator.h
Flute.o: ../sdp_lib/src/SDP_Generator.h flute_md5.h flute_cb.h FluteFec.h
Flute.o: FluteFile.h FluteFileList.h
FluteReceiver.o: flute_includes.h
FluteReceiver.o: /usr/local/include/xercesc/util/PlatformUtils.hpp
FluteReceiver.o: /usr/local/include/xercesc/util/XMLException.hpp
FluteReceiver.o: /usr/local/include/xercesc/util/XMemory.hpp
FluteReceiver.o: /usr/local/include/xercesc/util/XercesDefs.hpp
FluteReceiver.o: /usr/local/include/xercesc/util/XercesVersion.hpp
FluteReceiver.o: /usr/local/include/xercesc/util/AutoSense.hpp
FluteReceiver.o: /usr/local/include/xercesc/util/XMLExceptMsgs.hpp
FluteReceiver.o: /usr/local/include/xercesc/dom/DOMError.hpp
FluteReceiver.o: /usr/local/include/xercesc/util/XMLUni.hpp
FluteReceiver.o: /usr/local/include/xercesc/framework/XMLErrorReporter.hpp
FluteReceiver.o: /usr/local/include/xercesc/util/PanicHandler.hpp
FluteReceiver.o: /usr/local/include/xercesc/dom/DOM.hpp
FluteReceiver.o: /usr/local/include/xercesc/dom/DOMAttr.hpp
FluteReceiver.o: /usr/local/include/xercesc/dom/DOMNode.hpp
FluteReceiver.o: /usr/local/include/xercesc/dom/DOMCDATASection.hpp
FluteReceiver.o: /usr/local/include/xercesc/dom/DOMText.hpp
FluteReceiver.o: /usr/local/include/xercesc/dom/DOMCharacterData.hpp
FluteReceiver.o: /usr/local/include/xercesc/dom/DOMComment.hpp
FluteReceiver.o: /usr/local/include/xercesc/dom/DOMDocument.hpp
FluteReceiver.o: /usr/local/include/xercesc/dom/DOMDocumentRange.hpp
FluteReceiver.o: /usr/local/include/xercesc/dom/DOMDocumentTraversal.hpp
FluteReceiver.o: /usr/local/include/xercesc/dom/DOMXPathEvaluator.hpp
FluteReceiver.o: /usr/local/include/xercesc/dom/DOMDocumentFragment.hpp
FluteReceiver.o: /usr/local/include/xercesc/dom/DOMDocumentType.hpp
FluteReceiver.o: /usr/local/include/xercesc/dom/DOMElement.hpp
FluteReceiver.o: /usr/local/include/xercesc/dom/DOMEntity.hpp
FluteReceiver.o: /usr/local/include/xercesc/dom/DOMEntityReference.hpp
FluteReceiver.o: /usr/local/include/xercesc/dom/DOMException.hpp
FluteReceiver.o: /usr/local/include/xercesc/dom/DOMImplementation.hpp
FluteReceiver.o: /usr/local/include/xercesc/dom/DOMImplementationLS.hpp
FluteReceiver.o: /usr/local/include/xercesc/dom/DOMRangeException.hpp
FluteReceiver.o: /usr/local/include/xercesc/dom/DOMNamedNodeMap.hpp
FluteReceiver.o: /usr/local/include/xercesc/dom/DOMNodeList.hpp
FluteReceiver.o: /usr/local/include/xercesc/dom/DOMNotation.hpp
FluteReceiver.o: /usr/local/include/xercesc/dom/DOMProcessingInstruction.hpp
FluteReceiver.o: /usr/local/include/xercesc/dom/DOMNodeFilter.hpp
FluteReceiver.o: /usr/local/include/xercesc/dom/DOMNodeIterator.hpp
FluteReceiver.o: /usr/local/include/xercesc/dom/DOMRange.hpp
FluteReceiver.o: /usr/local/include/xercesc/dom/DOMTreeWalker.hpp
FluteReceiver.o: /usr/local/include/xercesc/dom/DOMBuilder.hpp
FluteReceiver.o: /usr/local/include/xercesc/dom/DOMConfiguration.hpp
FluteReceiver.o: /usr/local/include/xercesc/util/XMLString.hpp
FluteReceiver.o: /usr/local/include/xercesc/util/BaseRefVectorOf.hpp
FluteReceiver.o: /usr/local/include/xercesc/util/ArrayIndexOutOfBoundsException.hpp
FluteReceiver.o: /usr/local/include/xercesc/util/XMLEnumerator.hpp
FluteReceiver.o: /usr/local/include/xercesc/framework/MemoryManager.hpp
FluteReceiver.o: /usr/local/include/xercesc/util/BaseRefVectorOf.c
FluteReceiver.o: /usr/local/include/xercesc/framework/XMLBuffer.hpp
FluteReceiver.o: /usr/local/include/xercesc/dom/DOMEntityResolver.hpp
FluteReceiver.o: /usr/local/include/xercesc/dom/DOMErrorHandler.hpp
FluteReceiver.o: /usr/local/include/xercesc/dom/DOMImplementationRegistry.hpp
FluteReceiver.o: /usr/local/include/xercesc/dom/DOMImplementationSource.hpp
FluteReceiver.o: /usr/local/include/xercesc/dom/DOMInputSource.hpp
FluteReceiver.o: /usr/local/include/xercesc/dom/DOMLocator.hpp
FluteReceiver.o: /usr/local/include/xercesc/dom/DOMTypeInfo.hpp
FluteReceiver.o: /usr/local/include/xercesc/dom/DOMUserDataHandler.hpp
FluteReceiver.o: /usr/local/include/xercesc/dom/DOMWriter.hpp
FluteReceiver.o: /usr/local/include/xercesc/dom/DOMWriterFilter.hpp
FluteReceiver.o: /usr/local/include/xercesc/framework/XMLFormatter.hpp
FluteReceiver.o: /usr/local/include/xercesc/dom/DOMXPathNSResolver.hpp
FluteReceiver.o: /usr/local/include/xercesc/dom/DOMXPathException.hpp
FluteReceiver.o: /usr/local/include/xercesc/dom/DOMXPathExpression.hpp
FluteReceiver.o: /usr/local/include/xercesc/dom/DOMXPathResult.hpp
FluteReceiver.o: /usr/local/include/xercesc/dom/DOMXPathNamespace.hpp
FluteReceiver.o: /usr/local/include/xercesc/framework/Wrapper4InputSource.hpp
FluteReceiver.o: /usr/local/include/xercesc/framework/MemBufInputSource.hpp
FluteReceiver.o: /usr/local/include/xercesc/sax/InputSource.hpp
FluteReceiver.o: /usr/local/include/xercesc/framework/MemBufFormatTarget.hpp
FluteReceiver.o: FluteAPI.h ../common/mcl_lib_api.h ../common/mcl_types.h
FluteReceiver.o: ../alc/mcl_lib_api_alc.h Flute.h FluteReceiver.h
FluteReceiver.o: FluteSender.h FluteFileInfo.h FluteFileDeliveryCarousel.h
FluteReceiver.o: macros.h FluteTools.h flute_lock.h FluteFDT.h FluteSDP.h
FluteReceiver.o: ../sdp_lib/src/SDP_Parser.h ../sdp_lib/src/SDP_Error.h
FluteReceiver.o: ../sdp_lib/src/SDP_EventStreamParser.h
FluteReceiver.o: ../sdp_lib/src/SDP_StreamTokenizer.h
FluteReceiver.o: ../sdp_lib/src/SDP_Utility.h
FluteReceiver.o: ../sdp_lib/src/SDP_Description.h ../sdp_lib/src/SDP_Str.h
FluteReceiver.o: ../sdp_lib/src/SDP_LinkedList.h
FluteReceiver.o: ../sdp_lib/src/SDP_Generator.h
FluteReceiver.o: ../sdp_lib/src/SDP_Generator.h flute_md5.h flute_cb.h
FluteReceiver.o: FluteFec.h FluteFile.h FluteFileList.h
FluteSender.o: flute_includes.h
FluteSender.o: /usr/local/include/xercesc/util/PlatformUtils.hpp
FluteSender.o: /usr/local/include/xercesc/util/XMLException.hpp
FluteSender.o: /usr/local/include/xercesc/util/XMemory.hpp
FluteSender.o: /usr/local/include/xercesc/util/XercesDefs.hpp
FluteSender.o: /usr/local/include/xercesc/util/XercesVersion.hpp
FluteSender.o: /usr/local/include/xercesc/util/AutoSense.hpp
FluteSender.o: /usr/local/include/xercesc/util/XMLExceptMsgs.hpp
FluteSender.o: /usr/local/include/xercesc/dom/DOMError.hpp
FluteSender.o: /usr/local/include/xercesc/util/XMLUni.hpp
FluteSender.o: /usr/local/include/xercesc/framework/XMLErrorReporter.hpp
FluteSender.o: /usr/local/include/xercesc/util/PanicHandler.hpp
FluteSender.o: /usr/local/include/xercesc/dom/DOM.hpp
FluteSender.o: /usr/local/include/xercesc/dom/DOMAttr.hpp
FluteSender.o: /usr/local/include/xercesc/dom/DOMNode.hpp
FluteSender.o: /usr/local/include/xercesc/dom/DOMCDATASection.hpp
FluteSender.o: /usr/local/include/xercesc/dom/DOMText.hpp
FluteSender.o: /usr/local/include/xercesc/dom/DOMCharacterData.hpp
FluteSender.o: /usr/local/include/xercesc/dom/DOMComment.hpp
FluteSender.o: /usr/local/include/xercesc/dom/DOMDocument.hpp
FluteSender.o: /usr/local/include/xercesc/dom/DOMDocumentRange.hpp
FluteSender.o: /usr/local/include/xercesc/dom/DOMDocumentTraversal.hpp
FluteSender.o: /usr/local/include/xercesc/dom/DOMXPathEvaluator.hpp
FluteSender.o: /usr/local/include/xercesc/dom/DOMDocumentFragment.hpp
FluteSender.o: /usr/local/include/xercesc/dom/DOMDocumentType.hpp
FluteSender.o: /usr/local/include/xercesc/dom/DOMElement.hpp
FluteSender.o: /usr/local/include/xercesc/dom/DOMEntity.hpp
FluteSender.o: /usr/local/include/xercesc/dom/DOMEntityReference.hpp
FluteSender.o: /usr/local/include/xercesc/dom/DOMException.hpp
FluteSender.o: /usr/local/include/xercesc/dom/DOMImplementation.hpp
FluteSender.o: /usr/local/include/xercesc/dom/DOMImplementationLS.hpp
FluteSender.o: /usr/local/include/xercesc/dom/DOMRangeException.hpp
FluteSender.o: /usr/local/include/xercesc/dom/DOMNamedNodeMap.hpp
FluteSender.o: /usr/local/include/xercesc/dom/DOMNodeList.hpp
FluteSender.o: /usr/local/include/xercesc/dom/DOMNotation.hpp
FluteSender.o: /usr/local/include/xercesc/dom/DOMProcessingInstruction.hpp
FluteSender.o: /usr/local/include/xercesc/dom/DOMNodeFilter.hpp
FluteSender.o: /usr/local/include/xercesc/dom/DOMNodeIterator.hpp
FluteSender.o: /usr/local/include/xercesc/dom/DOMRange.hpp
FluteSender.o: /usr/local/include/xercesc/dom/DOMTreeWalker.hpp
FluteSender.o: /usr/local/include/xercesc/dom/DOMBuilder.hpp
FluteSender.o: /usr/local/include/xercesc/dom/DOMConfiguration.hpp
FluteSender.o: /usr/local/include/xercesc/util/XMLString.hpp
FluteSender.o: /usr/local/include/xercesc/util/BaseRefVectorOf.hpp
FluteSender.o: /usr/local/include/xercesc/util/ArrayIndexOutOfBoundsException.hpp
FluteSender.o: /usr/local/include/xercesc/util/XMLEnumerator.hpp
FluteSender.o: /usr/local/include/xercesc/framework/MemoryManager.hpp
FluteSender.o: /usr/local/include/xercesc/util/BaseRefVectorOf.c
FluteSender.o: /usr/local/include/xercesc/framework/XMLBuffer.hpp
FluteSender.o: /usr/local/include/xercesc/dom/DOMEntityResolver.hpp
FluteSender.o: /usr/local/include/xercesc/dom/DOMErrorHandler.hpp
FluteSender.o: /usr/local/include/xercesc/dom/DOMImplementationRegistry.hpp
FluteSender.o: /usr/local/include/xercesc/dom/DOMImplementationSource.hpp
FluteSender.o: /usr/local/include/xercesc/dom/DOMInputSource.hpp
FluteSender.o: /usr/local/include/xercesc/dom/DOMLocator.hpp
FluteSender.o: /usr/local/include/xercesc/dom/DOMTypeInfo.hpp
FluteSender.o: /usr/local/include/xercesc/dom/DOMUserDataHandler.hpp
FluteSender.o: /usr/local/include/xercesc/dom/DOMWriter.hpp
FluteSender.o: /usr/local/include/xercesc/dom/DOMWriterFilter.hpp
FluteSender.o: /usr/local/include/xercesc/framework/XMLFormatter.hpp
FluteSender.o: /usr/local/include/xercesc/dom/DOMXPathNSResolver.hpp
FluteSender.o: /usr/local/include/xercesc/dom/DOMXPathException.hpp
FluteSender.o: /usr/local/include/xercesc/dom/DOMXPathExpression.hpp
FluteSender.o: /usr/local/include/xercesc/dom/DOMXPathResult.hpp
FluteSender.o: /usr/local/include/xercesc/dom/DOMXPathNamespace.hpp
FluteSender.o: /usr/local/include/xercesc/framework/Wrapper4InputSource.hpp
FluteSender.o: /usr/local/include/xercesc/framework/MemBufInputSource.hpp
FluteSender.o: /usr/local/include/xercesc/sax/InputSource.hpp
FluteSender.o: /usr/local/include/xercesc/framework/MemBufFormatTarget.hpp
FluteSender.o: FluteAPI.h ../common/mcl_lib_api.h ../common/mcl_types.h
FluteSender.o: ../alc/mcl_lib_api_alc.h Flute.h FluteReceiver.h FluteSender.h
FluteSender.o: FluteFileInfo.h FluteFileDeliveryCarousel.h macros.h
FluteSender.o: FluteTools.h flute_lock.h FluteFDT.h FluteSDP.h
FluteSender.o: ../sdp_lib/src/SDP_Parser.h ../sdp_lib/src/SDP_Error.h
FluteSender.o: ../sdp_lib/src/SDP_EventStreamParser.h
FluteSender.o: ../sdp_lib/src/SDP_StreamTokenizer.h
FluteSender.o: ../sdp_lib/src/SDP_Utility.h ../sdp_lib/src/SDP_Description.h
FluteSender.o: ../sdp_lib/src/SDP_Str.h ../sdp_lib/src/SDP_LinkedList.h
FluteSender.o: ../sdp_lib/src/SDP_Generator.h ../sdp_lib/src/SDP_Generator.h
FluteSender.o: flute_md5.h flute_cb.h FluteFec.h FluteFile.h FluteFileList.h
FluteFileInfo.o: flute_includes.h
FluteFileInfo.o: /usr/local/include/xercesc/util/PlatformUtils.hpp
FluteFileInfo.o: /usr/local/include/xercesc/util/XMLException.hpp
FluteFileInfo.o: /usr/local/include/xercesc/util/XMemory.hpp
FluteFileInfo.o: /usr/local/include/xercesc/util/XercesDefs.hpp
FluteFileInfo.o: /usr/local/include/xercesc/util/XercesVersion.hpp
FluteFileInfo.o: /usr/local/include/xercesc/util/AutoSense.hpp
FluteFileInfo.o: /usr/local/include/xercesc/util/XMLExceptMsgs.hpp
FluteFileInfo.o: /usr/local/include/xercesc/dom/DOMError.hpp
FluteFileInfo.o: /usr/local/include/xercesc/util/XMLUni.hpp
FluteFileInfo.o: /usr/local/include/xercesc/framework/XMLErrorReporter.hpp
FluteFileInfo.o: /usr/local/include/xercesc/util/PanicHandler.hpp
FluteFileInfo.o: /usr/local/include/xercesc/dom/DOM.hpp
FluteFileInfo.o: /usr/local/include/xercesc/dom/DOMAttr.hpp
FluteFileInfo.o: /usr/local/include/xercesc/dom/DOMNode.hpp
FluteFileInfo.o: /usr/local/include/xercesc/dom/DOMCDATASection.hpp
FluteFileInfo.o: /usr/local/include/xercesc/dom/DOMText.hpp
FluteFileInfo.o: /usr/local/include/xercesc/dom/DOMCharacterData.hpp
FluteFileInfo.o: /usr/local/include/xercesc/dom/DOMComment.hpp
FluteFileInfo.o: /usr/local/include/xercesc/dom/DOMDocument.hpp
FluteFileInfo.o: /usr/local/include/xercesc/dom/DOMDocumentRange.hpp
FluteFileInfo.o: /usr/local/include/xercesc/dom/DOMDocumentTraversal.hpp
FluteFileInfo.o: /usr/local/include/xercesc/dom/DOMXPathEvaluator.hpp
FluteFileInfo.o: /usr/local/include/xercesc/dom/DOMDocumentFragment.hpp
FluteFileInfo.o: /usr/local/include/xercesc/dom/DOMDocumentType.hpp
FluteFileInfo.o: /usr/local/include/xercesc/dom/DOMElement.hpp
FluteFileInfo.o: /usr/local/include/xercesc/dom/DOMEntity.hpp
FluteFileInfo.o: /usr/local/include/xercesc/dom/DOMEntityReference.hpp
FluteFileInfo.o: /usr/local/include/xercesc/dom/DOMException.hpp
FluteFileInfo.o: /usr/local/include/xercesc/dom/DOMImplementation.hpp
FluteFileInfo.o: /usr/local/include/xercesc/dom/DOMImplementationLS.hpp
FluteFileInfo.o: /usr/local/include/xercesc/dom/DOMRangeException.hpp
FluteFileInfo.o: /usr/local/include/xercesc/dom/DOMNamedNodeMap.hpp
FluteFileInfo.o: /usr/local/include/xercesc/dom/DOMNodeList.hpp
FluteFileInfo.o: /usr/local/include/xercesc/dom/DOMNotation.hpp
FluteFileInfo.o: /usr/local/include/xercesc/dom/DOMProcessingInstruction.hpp
FluteFileInfo.o: /usr/local/include/xercesc/dom/DOMNodeFilter.hpp
FluteFileInfo.o: /usr/local/include/xercesc/dom/DOMNodeIterator.hpp
FluteFileInfo.o: /usr/local/include/xercesc/dom/DOMRange.hpp
FluteFileInfo.o: /usr/local/include/xercesc/dom/DOMTreeWalker.hpp
FluteFileInfo.o: /usr/local/include/xercesc/dom/DOMBuilder.hpp
FluteFileInfo.o: /usr/local/include/xercesc/dom/DOMConfiguration.hpp
FluteFileInfo.o: /usr/local/include/xercesc/util/XMLString.hpp
FluteFileInfo.o: /usr/local/include/xercesc/util/BaseRefVectorOf.hpp
FluteFileInfo.o: /usr/local/include/xercesc/util/ArrayIndexOutOfBoundsException.hpp
FluteFileInfo.o: /usr/local/include/xercesc/util/XMLEnumerator.hpp
FluteFileInfo.o: /usr/local/include/xercesc/framework/MemoryManager.hpp
FluteFileInfo.o: /usr/local/include/xercesc/util/BaseRefVectorOf.c
FluteFileInfo.o: /usr/local/include/xercesc/framework/XMLBuffer.hpp
FluteFileInfo.o: /usr/local/include/xercesc/dom/DOMEntityResolver.hpp
FluteFileInfo.o: /usr/local/include/xercesc/dom/DOMErrorHandler.hpp
FluteFileInfo.o: /usr/local/include/xercesc/dom/DOMImplementationRegistry.hpp
FluteFileInfo.o: /usr/local/include/xercesc/dom/DOMImplementationSource.hpp
FluteFileInfo.o: /usr/local/include/xercesc/dom/DOMInputSource.hpp
FluteFileInfo.o: /usr/local/include/xercesc/dom/DOMLocator.hpp
FluteFileInfo.o: /usr/local/include/xercesc/dom/DOMTypeInfo.hpp
FluteFileInfo.o: /usr/local/include/xercesc/dom/DOMUserDataHandler.hpp
FluteFileInfo.o: /usr/local/include/xercesc/dom/DOMWriter.hpp
FluteFileInfo.o: /usr/local/include/xercesc/dom/DOMWriterFilter.hpp
FluteFileInfo.o: /usr/local/include/xercesc/framework/XMLFormatter.hpp
FluteFileInfo.o: /usr/local/include/xercesc/dom/DOMXPathNSResolver.hpp
FluteFileInfo.o: /usr/local/include/xercesc/dom/DOMXPathException.hpp
FluteFileInfo.o: /usr/local/include/xercesc/dom/DOMXPathExpression.hpp
FluteFileInfo.o: /usr/local/include/xercesc/dom/DOMXPathResult.hpp
FluteFileInfo.o: /usr/local/include/xercesc/dom/DOMXPathNamespace.hpp
FluteFileInfo.o: /usr/local/include/xercesc/framework/Wrapper4InputSource.hpp
FluteFileInfo.o: /usr/local/include/xercesc/framework/MemBufInputSource.hpp
FluteFileInfo.o: /usr/local/include/xercesc/sax/InputSource.hpp
FluteFileInfo.o: /usr/local/include/xercesc/framework/MemBufFormatTarget.hpp
FluteFileInfo.o: FluteAPI.h ../common/mcl_lib_api.h ../common/mcl_types.h
FluteFileInfo.o: ../alc/mcl_lib_api_alc.h Flute.h FluteReceiver.h
FluteFileInfo.o: FluteSender.h FluteFileInfo.h FluteFileDeliveryCarousel.h
FluteFileInfo.o: macros.h FluteTools.h flute_lock.h FluteFDT.h FluteSDP.h
FluteFileInfo.o: ../sdp_lib/src/SDP_Parser.h ../sdp_lib/src/SDP_Error.h
FluteFileInfo.o: ../sdp_lib/src/SDP_EventStreamParser.h
FluteFileInfo.o: ../sdp_lib/src/SDP_StreamTokenizer.h
FluteFileInfo.o: ../sdp_lib/src/SDP_Utility.h
FluteFileInfo.o: ../sdp_lib/src/SDP_Description.h ../sdp_lib/src/SDP_Str.h
FluteFileInfo.o: ../sdp_lib/src/SDP_LinkedList.h
FluteFileInfo.o: ../sdp_lib/src/SDP_Generator.h
FluteFileInfo.o: ../sdp_lib/src/SDP_Generator.h flute_md5.h flute_cb.h
FluteFileInfo.o: FluteFec.h FluteFile.h FluteFileList.h
FluteFileDeliveryCarousel.o: flute_includes.h
FluteFileDeliveryCarousel.o: /usr/local/include/xercesc/util/PlatformUtils.hpp
FluteFileDeliveryCarousel.o: /usr/local/include/xercesc/util/XMLException.hpp
FluteFileDeliveryCarousel.o: /usr/local/include/xercesc/util/XMemory.hpp
FluteFileDeliveryCarousel.o: /usr/local/include/xercesc/util/XercesDefs.hpp
FluteFileDeliveryCarousel.o: /usr/local/include/xercesc/util/XercesVersion.hpp
FluteFileDeliveryCarousel.o: /usr/local/include/xercesc/util/AutoSense.hpp
FluteFileDeliveryCarousel.o: /usr/local/include/xercesc/util/XMLExceptMsgs.hpp
FluteFileDeliveryCarousel.o: /usr/local/include/xercesc/dom/DOMError.hpp
FluteFileDeliveryCarousel.o: /usr/local/include/xercesc/util/XMLUni.hpp
FluteFileDeliveryCarousel.o: /usr/local/include/xercesc/framework/XMLErrorReporter.hpp
FluteFileDeliveryCarousel.o: /usr/local/include/xercesc/util/PanicHandler.hpp
FluteFileDeliveryCarousel.o: /usr/local/include/xercesc/dom/DOM.hpp
FluteFileDeliveryCarousel.o: /usr/local/include/xercesc/dom/DOMAttr.hpp
FluteFileDeliveryCarousel.o: /usr/local/include/xercesc/dom/DOMNode.hpp
FluteFileDeliveryCarousel.o: /usr/local/include/xercesc/dom/DOMCDATASection.hpp
FluteFileDeliveryCarousel.o: /usr/local/include/xercesc/dom/DOMText.hpp
FluteFileDeliveryCarousel.o: /usr/local/include/xercesc/dom/DOMCharacterData.hpp
FluteFileDeliveryCarousel.o: /usr/local/include/xercesc/dom/DOMComment.hpp
FluteFileDeliveryCarousel.o: /usr/local/include/xercesc/dom/DOMDocument.hpp
FluteFileDeliveryCarousel.o: /usr/local/include/xercesc/dom/DOMDocumentRange.hpp
FluteFileDeliveryCarousel.o: /usr/local/include/xercesc/dom/DOMDocumentTraversal.hpp
FluteFileDeliveryCarousel.o: /usr/local/include/xercesc/dom/DOMXPathEvaluator.hpp
FluteFileDeliveryCarousel.o: /usr/local/include/xercesc/dom/DOMDocumentFragment.hpp
FluteFileDeliveryCarousel.o: /usr/local/include/xercesc/dom/DOMDocumentType.hpp
FluteFileDeliveryCarousel.o: /usr/local/include/xercesc/dom/DOMElement.hpp
FluteFileDeliveryCarousel.o: /usr/local/include/xercesc/dom/DOMEntity.hpp
FluteFileDeliveryCarousel.o: /usr/local/include/xercesc/dom/DOMEntityReference.hpp
FluteFileDeliveryCarousel.o: /usr/local/include/xercesc/dom/DOMException.hpp
FluteFileDeliveryCarousel.o: /usr/local/include/xercesc/dom/DOMImplementation.hpp
FluteFileDeliveryCarousel.o: /usr/local/include/xercesc/dom/DOMImplementationLS.hpp
FluteFileDeliveryCarousel.o: /usr/local/include/xercesc/dom/DOMRangeException.hpp
FluteFileDeliveryCarousel.o: /usr/local/include/xercesc/dom/DOMNamedNodeMap.hpp
FluteFileDeliveryCarousel.o: /usr/local/include/xercesc/dom/DOMNodeList.hpp
FluteFileDeliveryCarousel.o: /usr/local/include/xercesc/dom/DOMNotation.hpp
FluteFileDeliveryCarousel.o: /usr/local/include/xercesc/dom/DOMProcessingInstruction.hpp
FluteFileDeliveryCarousel.o: /usr/local/include/xercesc/dom/DOMNodeFilter.hpp
FluteFileDeliveryCarousel.o: /usr/local/include/xercesc/dom/DOMNodeIterator.hpp
FluteFileDeliveryCarousel.o: /usr/local/include/xercesc/dom/DOMRange.hpp
FluteFileDeliveryCarousel.o: /usr/local/include/xercesc/dom/DOMTreeWalker.hpp
FluteFileDeliveryCarousel.o: /usr/local/include/xercesc/dom/DOMBuilder.hpp
FluteFileDeliveryCarousel.o: /usr/local/include/xercesc/dom/DOMConfiguration.hpp
FluteFileDeliveryCarousel.o: /usr/local/include/xercesc/util/XMLString.hpp
FluteFileDeliveryCarousel.o: /usr/local/include/xercesc/util/BaseRefVectorOf.hpp
FluteFileDeliveryCarousel.o: /usr/local/include/xercesc/util/ArrayIndexOutOfBoundsException.hpp
FluteFileDeliveryCarousel.o: /usr/local/include/xercesc/util/XMLEnumerator.hpp
FluteFileDeliveryCarousel.o: /usr/local/include/xercesc/framework/MemoryManager.hpp
FluteFileDeliveryCarousel.o: /usr/local/include/xercesc/util/BaseRefVectorOf.c
FluteFileDeliveryCarousel.o: /usr/local/include/xercesc/framework/XMLBuffer.hpp
FluteFileDeliveryCarousel.o: /usr/local/include/xercesc/dom/DOMEntityResolver.hpp
FluteFileDeliveryCarousel.o: /usr/local/include/xercesc/dom/DOMErrorHandler.hpp
FluteFileDeliveryCarousel.o: /usr/local/include/xercesc/dom/DOMImplementationRegistry.hpp
FluteFileDeliveryCarousel.o: /usr/local/include/xercesc/dom/DOMImplementationSource.hpp
FluteFileDeliveryCarousel.o: /usr/local/include/xercesc/dom/DOMInputSource.hpp
FluteFileDeliveryCarousel.o: /usr/local/include/xercesc/dom/DOMLocator.hpp
FluteFileDeliveryCarousel.o: /usr/local/include/xercesc/dom/DOMTypeInfo.hpp
FluteFileDeliveryCarousel.o: /usr/local/include/xercesc/dom/DOMUserDataHandler.hpp
FluteFileDeliveryCarousel.o: /usr/local/include/xercesc/dom/DOMWriter.hpp
FluteFileDeliveryCarousel.o: /usr/local/include/xercesc/dom/DOMWriterFilter.hpp
FluteFileDeliveryCarousel.o: /usr/local/include/xercesc/framework/XMLFormatter.hpp
FluteFileDeliveryCarousel.o: /usr/local/include/xercesc/dom/DOMXPathNSResolver.hpp
FluteFileDeliveryCarousel.o: /usr/local/include/xercesc/dom/DOMXPathException.hpp
FluteFileDeliveryCarousel.o: /usr/local/include/xercesc/dom/DOMXPathExpression.hpp
FluteFileDeliveryCarousel.o: /usr/local/include/xercesc/dom/DOMXPathResult.hpp
FluteFileDeliveryCarousel.o: /usr/local/include/xercesc/dom/DOMXPathNamespace.hpp
FluteFileDeliveryCarousel.o: /usr/local/include/xercesc/framework/Wrapper4InputSource.hpp
FluteFileDeliveryCarousel.o: /usr/local/include/xercesc/framework/MemBufInputSource.hpp
FluteFileDeliveryCarousel.o: /usr/local/include/xercesc/sax/InputSource.hpp
FluteFileDeliveryCarousel.o: /usr/local/include/xercesc/framework/MemBufFormatTarget.hpp
FluteFileDeliveryCarousel.o: FluteAPI.h ../common/mcl_lib_api.h
FluteFileDeliveryCarousel.o: ../common/mcl_types.h ../alc/mcl_lib_api_alc.h
FluteFileDeliveryCarousel.o: Flute.h FluteReceiver.h FluteSender.h
FluteFileDeliveryCarousel.o: FluteFileInfo.h FluteFileDeliveryCarousel.h
FluteFileDeliveryCarousel.o: macros.h FluteTools.h flute_lock.h FluteFDT.h
FluteFileDeliveryCarousel.o: FluteSDP.h ../sdp_lib/src/SDP_Parser.h
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
FluteFile.o: flute_includes.h
FluteFile.o: /usr/local/include/xercesc/util/PlatformUtils.hpp
FluteFile.o: /usr/local/include/xercesc/util/XMLException.hpp
FluteFile.o: /usr/local/include/xercesc/util/XMemory.hpp
FluteFile.o: /usr/local/include/xercesc/util/XercesDefs.hpp
FluteFile.o: /usr/local/include/xercesc/util/XercesVersion.hpp
FluteFile.o: /usr/local/include/xercesc/util/AutoSense.hpp
FluteFile.o: /usr/local/include/xercesc/util/XMLExceptMsgs.hpp
FluteFile.o: /usr/local/include/xercesc/dom/DOMError.hpp
FluteFile.o: /usr/local/include/xercesc/util/XMLUni.hpp
FluteFile.o: /usr/local/include/xercesc/framework/XMLErrorReporter.hpp
FluteFile.o: /usr/local/include/xercesc/util/PanicHandler.hpp
FluteFile.o: /usr/local/include/xercesc/dom/DOM.hpp
FluteFile.o: /usr/local/include/xercesc/dom/DOMAttr.hpp
FluteFile.o: /usr/local/include/xercesc/dom/DOMNode.hpp
FluteFile.o: /usr/local/include/xercesc/dom/DOMCDATASection.hpp
FluteFile.o: /usr/local/include/xercesc/dom/DOMText.hpp
FluteFile.o: /usr/local/include/xercesc/dom/DOMCharacterData.hpp
FluteFile.o: /usr/local/include/xercesc/dom/DOMComment.hpp
FluteFile.o: /usr/local/include/xercesc/dom/DOMDocument.hpp
FluteFile.o: /usr/local/include/xercesc/dom/DOMDocumentRange.hpp
FluteFile.o: /usr/local/include/xercesc/dom/DOMDocumentTraversal.hpp
FluteFile.o: /usr/local/include/xercesc/dom/DOMXPathEvaluator.hpp
FluteFile.o: /usr/local/include/xercesc/dom/DOMDocumentFragment.hpp
FluteFile.o: /usr/local/include/xercesc/dom/DOMDocumentType.hpp
FluteFile.o: /usr/local/include/xercesc/dom/DOMElement.hpp
FluteFile.o: /usr/local/include/xercesc/dom/DOMEntity.hpp
FluteFile.o: /usr/local/include/xercesc/dom/DOMEntityReference.hpp
FluteFile.o: /usr/local/include/xercesc/dom/DOMException.hpp
FluteFile.o: /usr/local/include/xercesc/dom/DOMImplementation.hpp
FluteFile.o: /usr/local/include/xercesc/dom/DOMImplementationLS.hpp
FluteFile.o: /usr/local/include/xercesc/dom/DOMRangeException.hpp
FluteFile.o: /usr/local/include/xercesc/dom/DOMNamedNodeMap.hpp
FluteFile.o: /usr/local/include/xercesc/dom/DOMNodeList.hpp
FluteFile.o: /usr/local/include/xercesc/dom/DOMNotation.hpp
FluteFile.o: /usr/local/include/xercesc/dom/DOMProcessingInstruction.hpp
FluteFile.o: /usr/local/include/xercesc/dom/DOMNodeFilter.hpp
FluteFile.o: /usr/local/include/xercesc/dom/DOMNodeIterator.hpp
FluteFile.o: /usr/local/include/xercesc/dom/DOMRange.hpp
FluteFile.o: /usr/local/include/xercesc/dom/DOMTreeWalker.hpp
FluteFile.o: /usr/local/include/xercesc/dom/DOMBuilder.hpp
FluteFile.o: /usr/local/include/xercesc/dom/DOMConfiguration.hpp
FluteFile.o: /usr/local/include/xercesc/util/XMLString.hpp
FluteFile.o: /usr/local/include/xercesc/util/BaseRefVectorOf.hpp
FluteFile.o: /usr/local/include/xercesc/util/ArrayIndexOutOfBoundsException.hpp
FluteFile.o: /usr/local/include/xercesc/util/XMLEnumerator.hpp
FluteFile.o: /usr/local/include/xercesc/framework/MemoryManager.hpp
FluteFile.o: /usr/local/include/xercesc/util/BaseRefVectorOf.c
FluteFile.o: /usr/local/include/xercesc/framework/XMLBuffer.hpp
FluteFile.o: /usr/local/include/xercesc/dom/DOMEntityResolver.hpp
FluteFile.o: /usr/local/include/xercesc/dom/DOMErrorHandler.hpp
FluteFile.o: /usr/local/include/xercesc/dom/DOMImplementationRegistry.hpp
FluteFile.o: /usr/local/include/xercesc/dom/DOMImplementationSource.hpp
FluteFile.o: /usr/local/include/xercesc/dom/DOMInputSource.hpp
FluteFile.o: /usr/local/include/xercesc/dom/DOMLocator.hpp
FluteFile.o: /usr/local/include/xercesc/dom/DOMTypeInfo.hpp
FluteFile.o: /usr/local/include/xercesc/dom/DOMUserDataHandler.hpp
FluteFile.o: /usr/local/include/xercesc/dom/DOMWriter.hpp
FluteFile.o: /usr/local/include/xercesc/dom/DOMWriterFilter.hpp
FluteFile.o: /usr/local/include/xercesc/framework/XMLFormatter.hpp
FluteFile.o: /usr/local/include/xercesc/dom/DOMXPathNSResolver.hpp
FluteFile.o: /usr/local/include/xercesc/dom/DOMXPathException.hpp
FluteFile.o: /usr/local/include/xercesc/dom/DOMXPathExpression.hpp
FluteFile.o: /usr/local/include/xercesc/dom/DOMXPathResult.hpp
FluteFile.o: /usr/local/include/xercesc/dom/DOMXPathNamespace.hpp
FluteFile.o: /usr/local/include/xercesc/framework/Wrapper4InputSource.hpp
FluteFile.o: /usr/local/include/xercesc/framework/MemBufInputSource.hpp
FluteFile.o: /usr/local/include/xercesc/sax/InputSource.hpp
FluteFile.o: /usr/local/include/xercesc/framework/MemBufFormatTarget.hpp
FluteFile.o: FluteAPI.h ../common/mcl_lib_api.h ../common/mcl_types.h
FluteFile.o: ../alc/mcl_lib_api_alc.h Flute.h FluteReceiver.h FluteSender.h
FluteFile.o: FluteFileInfo.h FluteFileDeliveryCarousel.h macros.h
FluteFile.o: FluteTools.h flute_lock.h FluteFDT.h FluteSDP.h
FluteFile.o: ../sdp_lib/src/SDP_Parser.h ../sdp_lib/src/SDP_Error.h
FluteFile.o: ../sdp_lib/src/SDP_EventStreamParser.h
FluteFile.o: ../sdp_lib/src/SDP_StreamTokenizer.h
FluteFile.o: ../sdp_lib/src/SDP_Utility.h ../sdp_lib/src/SDP_Description.h
FluteFile.o: ../sdp_lib/src/SDP_Str.h ../sdp_lib/src/SDP_LinkedList.h
FluteFile.o: ../sdp_lib/src/SDP_Generator.h ../sdp_lib/src/SDP_Generator.h
FluteFile.o: flute_md5.h flute_cb.h FluteFec.h FluteFile.h FluteFileList.h
FluteFileList.o: flute_includes.h
FluteFileList.o: /usr/local/include/xercesc/util/PlatformUtils.hpp
FluteFileList.o: /usr/local/include/xercesc/util/XMLException.hpp
FluteFileList.o: /usr/local/include/xercesc/util/XMemory.hpp
FluteFileList.o: /usr/local/include/xercesc/util/XercesDefs.hpp
FluteFileList.o: /usr/local/include/xercesc/util/XercesVersion.hpp
FluteFileList.o: /usr/local/include/xercesc/util/AutoSense.hpp
FluteFileList.o: /usr/local/include/xercesc/util/XMLExceptMsgs.hpp
FluteFileList.o: /usr/local/include/xercesc/dom/DOMError.hpp
FluteFileList.o: /usr/local/include/xercesc/util/XMLUni.hpp
FluteFileList.o: /usr/local/include/xercesc/framework/XMLErrorReporter.hpp
FluteFileList.o: /usr/local/include/xercesc/util/PanicHandler.hpp
FluteFileList.o: /usr/local/include/xercesc/dom/DOM.hpp
FluteFileList.o: /usr/local/include/xercesc/dom/DOMAttr.hpp
FluteFileList.o: /usr/local/include/xercesc/dom/DOMNode.hpp
FluteFileList.o: /usr/local/include/xercesc/dom/DOMCDATASection.hpp
FluteFileList.o: /usr/local/include/xercesc/dom/DOMText.hpp
FluteFileList.o: /usr/local/include/xercesc/dom/DOMCharacterData.hpp
FluteFileList.o: /usr/local/include/xercesc/dom/DOMComment.hpp
FluteFileList.o: /usr/local/include/xercesc/dom/DOMDocument.hpp
FluteFileList.o: /usr/local/include/xercesc/dom/DOMDocumentRange.hpp
FluteFileList.o: /usr/local/include/xercesc/dom/DOMDocumentTraversal.hpp
FluteFileList.o: /usr/local/include/xercesc/dom/DOMXPathEvaluator.hpp
FluteFileList.o: /usr/local/include/xercesc/dom/DOMDocumentFragment.hpp
FluteFileList.o: /usr/local/include/xercesc/dom/DOMDocumentType.hpp
FluteFileList.o: /usr/local/include/xercesc/dom/DOMElement.hpp
FluteFileList.o: /usr/local/include/xercesc/dom/DOMEntity.hpp
FluteFileList.o: /usr/local/include/xercesc/dom/DOMEntityReference.hpp
FluteFileList.o: /usr/local/include/xercesc/dom/DOMException.hpp
FluteFileList.o: /usr/local/include/xercesc/dom/DOMImplementation.hpp
FluteFileList.o: /usr/local/include/xercesc/dom/DOMImplementationLS.hpp
FluteFileList.o: /usr/local/include/xercesc/dom/DOMRangeException.hpp
FluteFileList.o: /usr/local/include/xercesc/dom/DOMNamedNodeMap.hpp
FluteFileList.o: /usr/local/include/xercesc/dom/DOMNodeList.hpp
FluteFileList.o: /usr/local/include/xercesc/dom/DOMNotation.hpp
FluteFileList.o: /usr/local/include/xercesc/dom/DOMProcessingInstruction.hpp
FluteFileList.o: /usr/local/include/xercesc/dom/DOMNodeFilter.hpp
FluteFileList.o: /usr/local/include/xercesc/dom/DOMNodeIterator.hpp
FluteFileList.o: /usr/local/include/xercesc/dom/DOMRange.hpp
FluteFileList.o: /usr/local/include/xercesc/dom/DOMTreeWalker.hpp
FluteFileList.o: /usr/local/include/xercesc/dom/DOMBuilder.hpp
FluteFileList.o: /usr/local/include/xercesc/dom/DOMConfiguration.hpp
FluteFileList.o: /usr/local/include/xercesc/util/XMLString.hpp
FluteFileList.o: /usr/local/include/xercesc/util/BaseRefVectorOf.hpp
FluteFileList.o: /usr/local/include/xercesc/util/ArrayIndexOutOfBoundsException.hpp
FluteFileList.o: /usr/local/include/xercesc/util/XMLEnumerator.hpp
FluteFileList.o: /usr/local/include/xercesc/framework/MemoryManager.hpp
FluteFileList.o: /usr/local/include/xercesc/util/BaseRefVectorOf.c
FluteFileList.o: /usr/local/include/xercesc/framework/XMLBuffer.hpp
FluteFileList.o: /usr/local/include/xercesc/dom/DOMEntityResolver.hpp
FluteFileList.o: /usr/local/include/xercesc/dom/DOMErrorHandler.hpp
FluteFileList.o: /usr/local/include/xercesc/dom/DOMImplementationRegistry.hpp
FluteFileList.o: /usr/local/include/xercesc/dom/DOMImplementationSource.hpp
FluteFileList.o: /usr/local/include/xercesc/dom/DOMInputSource.hpp
FluteFileList.o: /usr/local/include/xercesc/dom/DOMLocator.hpp
FluteFileList.o: /usr/local/include/xercesc/dom/DOMTypeInfo.hpp
FluteFileList.o: /usr/local/include/xercesc/dom/DOMUserDataHandler.hpp
FluteFileList.o: /usr/local/include/xercesc/dom/DOMWriter.hpp
FluteFileList.o: /usr/local/include/xercesc/dom/DOMWriterFilter.hpp
FluteFileList.o: /usr/local/include/xercesc/framework/XMLFormatter.hpp
FluteFileList.o: /usr/local/include/xercesc/dom/DOMXPathNSResolver.hpp
FluteFileList.o: /usr/local/include/xercesc/dom/DOMXPathException.hpp
FluteFileList.o: /usr/local/include/xercesc/dom/DOMXPathExpression.hpp
FluteFileList.o: /usr/local/include/xercesc/dom/DOMXPathResult.hpp
FluteFileList.o: /usr/local/include/xercesc/dom/DOMXPathNamespace.hpp
FluteFileList.o: /usr/local/include/xercesc/framework/Wrapper4InputSource.hpp
FluteFileList.o: /usr/local/include/xercesc/framework/MemBufInputSource.hpp
FluteFileList.o: /usr/local/include/xercesc/sax/InputSource.hpp
FluteFileList.o: /usr/local/include/xercesc/framework/MemBufFormatTarget.hpp
FluteFileList.o: FluteAPI.h ../common/mcl_lib_api.h ../common/mcl_types.h
FluteFileList.o: ../alc/mcl_lib_api_alc.h Flute.h FluteReceiver.h
FluteFileList.o: FluteSender.h FluteFileInfo.h FluteFileDeliveryCarousel.h
FluteFileList.o: macros.h FluteTools.h flute_lock.h FluteFDT.h FluteSDP.h
FluteFileList.o: ../sdp_lib/src/SDP_Parser.h ../sdp_lib/src/SDP_Error.h
FluteFileList.o: ../sdp_lib/src/SDP_EventStreamParser.h
FluteFileList.o: ../sdp_lib/src/SDP_StreamTokenizer.h
FluteFileList.o: ../sdp_lib/src/SDP_Utility.h
FluteFileList.o: ../sdp_lib/src/SDP_Description.h ../sdp_lib/src/SDP_Str.h
FluteFileList.o: ../sdp_lib/src/SDP_LinkedList.h
FluteFileList.o: ../sdp_lib/src/SDP_Generator.h
FluteFileList.o: ../sdp_lib/src/SDP_Generator.h flute_md5.h flute_cb.h
FluteFileList.o: FluteFec.h FluteFile.h FluteFileList.h
FluteFec.o: flute_includes.h
FluteFec.o: /usr/local/include/xercesc/util/PlatformUtils.hpp
FluteFec.o: /usr/local/include/xercesc/util/XMLException.hpp
FluteFec.o: /usr/local/include/xercesc/util/XMemory.hpp
FluteFec.o: /usr/local/include/xercesc/util/XercesDefs.hpp
FluteFec.o: /usr/local/include/xercesc/util/XercesVersion.hpp
FluteFec.o: /usr/local/include/xercesc/util/AutoSense.hpp
FluteFec.o: /usr/local/include/xercesc/util/XMLExceptMsgs.hpp
FluteFec.o: /usr/local/include/xercesc/dom/DOMError.hpp
FluteFec.o: /usr/local/include/xercesc/util/XMLUni.hpp
FluteFec.o: /usr/local/include/xercesc/framework/XMLErrorReporter.hpp
FluteFec.o: /usr/local/include/xercesc/util/PanicHandler.hpp
FluteFec.o: /usr/local/include/xercesc/dom/DOM.hpp
FluteFec.o: /usr/local/include/xercesc/dom/DOMAttr.hpp
FluteFec.o: /usr/local/include/xercesc/dom/DOMNode.hpp
FluteFec.o: /usr/local/include/xercesc/dom/DOMCDATASection.hpp
FluteFec.o: /usr/local/include/xercesc/dom/DOMText.hpp
FluteFec.o: /usr/local/include/xercesc/dom/DOMCharacterData.hpp
FluteFec.o: /usr/local/include/xercesc/dom/DOMComment.hpp
FluteFec.o: /usr/local/include/xercesc/dom/DOMDocument.hpp
FluteFec.o: /usr/local/include/xercesc/dom/DOMDocumentRange.hpp
FluteFec.o: /usr/local/include/xercesc/dom/DOMDocumentTraversal.hpp
FluteFec.o: /usr/local/include/xercesc/dom/DOMXPathEvaluator.hpp
FluteFec.o: /usr/local/include/xercesc/dom/DOMDocumentFragment.hpp
FluteFec.o: /usr/local/include/xercesc/dom/DOMDocumentType.hpp
FluteFec.o: /usr/local/include/xercesc/dom/DOMElement.hpp
FluteFec.o: /usr/local/include/xercesc/dom/DOMEntity.hpp
FluteFec.o: /usr/local/include/xercesc/dom/DOMEntityReference.hpp
FluteFec.o: /usr/local/include/xercesc/dom/DOMException.hpp
FluteFec.o: /usr/local/include/xercesc/dom/DOMImplementation.hpp
FluteFec.o: /usr/local/include/xercesc/dom/DOMImplementationLS.hpp
FluteFec.o: /usr/local/include/xercesc/dom/DOMRangeException.hpp
FluteFec.o: /usr/local/include/xercesc/dom/DOMNamedNodeMap.hpp
FluteFec.o: /usr/local/include/xercesc/dom/DOMNodeList.hpp
FluteFec.o: /usr/local/include/xercesc/dom/DOMNotation.hpp
FluteFec.o: /usr/local/include/xercesc/dom/DOMProcessingInstruction.hpp
FluteFec.o: /usr/local/include/xercesc/dom/DOMNodeFilter.hpp
FluteFec.o: /usr/local/include/xercesc/dom/DOMNodeIterator.hpp
FluteFec.o: /usr/local/include/xercesc/dom/DOMRange.hpp
FluteFec.o: /usr/local/include/xercesc/dom/DOMTreeWalker.hpp
FluteFec.o: /usr/local/include/xercesc/dom/DOMBuilder.hpp
FluteFec.o: /usr/local/include/xercesc/dom/DOMConfiguration.hpp
FluteFec.o: /usr/local/include/xercesc/util/XMLString.hpp
FluteFec.o: /usr/local/include/xercesc/util/BaseRefVectorOf.hpp
FluteFec.o: /usr/local/include/xercesc/util/ArrayIndexOutOfBoundsException.hpp
FluteFec.o: /usr/local/include/xercesc/util/XMLEnumerator.hpp
FluteFec.o: /usr/local/include/xercesc/framework/MemoryManager.hpp
FluteFec.o: /usr/local/include/xercesc/util/BaseRefVectorOf.c
FluteFec.o: /usr/local/include/xercesc/framework/XMLBuffer.hpp
FluteFec.o: /usr/local/include/xercesc/dom/DOMEntityResolver.hpp
FluteFec.o: /usr/local/include/xercesc/dom/DOMErrorHandler.hpp
FluteFec.o: /usr/local/include/xercesc/dom/DOMImplementationRegistry.hpp
FluteFec.o: /usr/local/include/xercesc/dom/DOMImplementationSource.hpp
FluteFec.o: /usr/local/include/xercesc/dom/DOMInputSource.hpp
FluteFec.o: /usr/local/include/xercesc/dom/DOMLocator.hpp
FluteFec.o: /usr/local/include/xercesc/dom/DOMTypeInfo.hpp
FluteFec.o: /usr/local/include/xercesc/dom/DOMUserDataHandler.hpp
FluteFec.o: /usr/local/include/xercesc/dom/DOMWriter.hpp
FluteFec.o: /usr/local/include/xercesc/dom/DOMWriterFilter.hpp
FluteFec.o: /usr/local/include/xercesc/framework/XMLFormatter.hpp
FluteFec.o: /usr/local/include/xercesc/dom/DOMXPathNSResolver.hpp
FluteFec.o: /usr/local/include/xercesc/dom/DOMXPathException.hpp
FluteFec.o: /usr/local/include/xercesc/dom/DOMXPathExpression.hpp
FluteFec.o: /usr/local/include/xercesc/dom/DOMXPathResult.hpp
FluteFec.o: /usr/local/include/xercesc/dom/DOMXPathNamespace.hpp
FluteFec.o: /usr/local/include/xercesc/framework/Wrapper4InputSource.hpp
FluteFec.o: /usr/local/include/xercesc/framework/MemBufInputSource.hpp
FluteFec.o: /usr/local/include/xercesc/sax/InputSource.hpp
FluteFec.o: /usr/local/include/xercesc/framework/MemBufFormatTarget.hpp
FluteFec.o: FluteAPI.h ../common/mcl_lib_api.h ../common/mcl_types.h
FluteFec.o: ../alc/mcl_lib_api_alc.h Flute.h FluteReceiver.h FluteSender.h
FluteFec.o: FluteFileInfo.h FluteFileDeliveryCarousel.h macros.h FluteTools.h
FluteFec.o: flute_lock.h FluteFDT.h FluteSDP.h ../sdp_lib/src/SDP_Parser.h
FluteFec.o: ../sdp_lib/src/SDP_Error.h ../sdp_lib/src/SDP_EventStreamParser.h
FluteFec.o: ../sdp_lib/src/SDP_StreamTokenizer.h ../sdp_lib/src/SDP_Utility.h
FluteFec.o: ../sdp_lib/src/SDP_Description.h ../sdp_lib/src/SDP_Str.h
FluteFec.o: ../sdp_lib/src/SDP_LinkedList.h ../sdp_lib/src/SDP_Generator.h
FluteFec.o: ../sdp_lib/src/SDP_Generator.h flute_md5.h flute_cb.h FluteFec.h
FluteFec.o: FluteFile.h FluteFileList.h
flute_cb.o: flute_includes.h
flute_cb.o: /usr/local/include/xercesc/util/PlatformUtils.hpp
flute_cb.o: /usr/local/include/xercesc/util/XMLException.hpp
flute_cb.o: /usr/local/include/xercesc/util/XMemory.hpp
flute_cb.o: /usr/local/include/xercesc/util/XercesDefs.hpp
flute_cb.o: /usr/local/include/xercesc/util/XercesVersion.hpp
flute_cb.o: /usr/local/include/xercesc/util/AutoSense.hpp
flute_cb.o: /usr/local/include/xercesc/util/XMLExceptMsgs.hpp
flute_cb.o: /usr/local/include/xercesc/dom/DOMError.hpp
flute_cb.o: /usr/local/include/xercesc/util/XMLUni.hpp
flute_cb.o: /usr/local/include/xercesc/framework/XMLErrorReporter.hpp
flute_cb.o: /usr/local/include/xercesc/util/PanicHandler.hpp
flute_cb.o: /usr/local/include/xercesc/dom/DOM.hpp
flute_cb.o: /usr/local/include/xercesc/dom/DOMAttr.hpp
flute_cb.o: /usr/local/include/xercesc/dom/DOMNode.hpp
flute_cb.o: /usr/local/include/xercesc/dom/DOMCDATASection.hpp
flute_cb.o: /usr/local/include/xercesc/dom/DOMText.hpp
flute_cb.o: /usr/local/include/xercesc/dom/DOMCharacterData.hpp
flute_cb.o: /usr/local/include/xercesc/dom/DOMComment.hpp
flute_cb.o: /usr/local/include/xercesc/dom/DOMDocument.hpp
flute_cb.o: /usr/local/include/xercesc/dom/DOMDocumentRange.hpp
flute_cb.o: /usr/local/include/xercesc/dom/DOMDocumentTraversal.hpp
flute_cb.o: /usr/local/include/xercesc/dom/DOMXPathEvaluator.hpp
flute_cb.o: /usr/local/include/xercesc/dom/DOMDocumentFragment.hpp
flute_cb.o: /usr/local/include/xercesc/dom/DOMDocumentType.hpp
flute_cb.o: /usr/local/include/xercesc/dom/DOMElement.hpp
flute_cb.o: /usr/local/include/xercesc/dom/DOMEntity.hpp
flute_cb.o: /usr/local/include/xercesc/dom/DOMEntityReference.hpp
flute_cb.o: /usr/local/include/xercesc/dom/DOMException.hpp
flute_cb.o: /usr/local/include/xercesc/dom/DOMImplementation.hpp
flute_cb.o: /usr/local/include/xercesc/dom/DOMImplementationLS.hpp
flute_cb.o: /usr/local/include/xercesc/dom/DOMRangeException.hpp
flute_cb.o: /usr/local/include/xercesc/dom/DOMNamedNodeMap.hpp
flute_cb.o: /usr/local/include/xercesc/dom/DOMNodeList.hpp
flute_cb.o: /usr/local/include/xercesc/dom/DOMNotation.hpp
flute_cb.o: /usr/local/include/xercesc/dom/DOMProcessingInstruction.hpp
flute_cb.o: /usr/local/include/xercesc/dom/DOMNodeFilter.hpp
flute_cb.o: /usr/local/include/xercesc/dom/DOMNodeIterator.hpp
flute_cb.o: /usr/local/include/xercesc/dom/DOMRange.hpp
flute_cb.o: /usr/local/include/xercesc/dom/DOMTreeWalker.hpp
flute_cb.o: /usr/local/include/xercesc/dom/DOMBuilder.hpp
flute_cb.o: /usr/local/include/xercesc/dom/DOMConfiguration.hpp
flute_cb.o: /usr/local/include/xercesc/util/XMLString.hpp
flute_cb.o: /usr/local/include/xercesc/util/BaseRefVectorOf.hpp
flute_cb.o: /usr/local/include/xercesc/util/ArrayIndexOutOfBoundsException.hpp
flute_cb.o: /usr/local/include/xercesc/util/XMLEnumerator.hpp
flute_cb.o: /usr/local/include/xercesc/framework/MemoryManager.hpp
flute_cb.o: /usr/local/include/xercesc/util/BaseRefVectorOf.c
flute_cb.o: /usr/local/include/xercesc/framework/XMLBuffer.hpp
flute_cb.o: /usr/local/include/xercesc/dom/DOMEntityResolver.hpp
flute_cb.o: /usr/local/include/xercesc/dom/DOMErrorHandler.hpp
flute_cb.o: /usr/local/include/xercesc/dom/DOMImplementationRegistry.hpp
flute_cb.o: /usr/local/include/xercesc/dom/DOMImplementationSource.hpp
flute_cb.o: /usr/local/include/xercesc/dom/DOMInputSource.hpp
flute_cb.o: /usr/local/include/xercesc/dom/DOMLocator.hpp
flute_cb.o: /usr/local/include/xercesc/dom/DOMTypeInfo.hpp
flute_cb.o: /usr/local/include/xercesc/dom/DOMUserDataHandler.hpp
flute_cb.o: /usr/local/include/xercesc/dom/DOMWriter.hpp
flute_cb.o: /usr/local/include/xercesc/dom/DOMWriterFilter.hpp
flute_cb.o: /usr/local/include/xercesc/framework/XMLFormatter.hpp
flute_cb.o: /usr/local/include/xercesc/dom/DOMXPathNSResolver.hpp
flute_cb.o: /usr/local/include/xercesc/dom/DOMXPathException.hpp
flute_cb.o: /usr/local/include/xercesc/dom/DOMXPathExpression.hpp
flute_cb.o: /usr/local/include/xercesc/dom/DOMXPathResult.hpp
flute_cb.o: /usr/local/include/xercesc/dom/DOMXPathNamespace.hpp
flute_cb.o: /usr/local/include/xercesc/framework/Wrapper4InputSource.hpp
flute_cb.o: /usr/local/include/xercesc/framework/MemBufInputSource.hpp
flute_cb.o: /usr/local/include/xercesc/sax/InputSource.hpp
flute_cb.o: /usr/local/include/xercesc/framework/MemBufFormatTarget.hpp
flute_cb.o: FluteAPI.h ../common/mcl_lib_api.h ../common/mcl_types.h
flute_cb.o: ../alc/mcl_lib_api_alc.h Flute.h FluteReceiver.h FluteSender.h
flute_cb.o: FluteFileInfo.h FluteFileDeliveryCarousel.h macros.h FluteTools.h
flute_cb.o: flute_lock.h FluteFDT.h FluteSDP.h ../sdp_lib/src/SDP_Parser.h
flute_cb.o: ../sdp_lib/src/SDP_Error.h ../sdp_lib/src/SDP_EventStreamParser.h
flute_cb.o: ../sdp_lib/src/SDP_StreamTokenizer.h ../sdp_lib/src/SDP_Utility.h
flute_cb.o: ../sdp_lib/src/SDP_Description.h ../sdp_lib/src/SDP_Str.h
flute_cb.o: ../sdp_lib/src/SDP_LinkedList.h ../sdp_lib/src/SDP_Generator.h
flute_cb.o: ../sdp_lib/src/SDP_Generator.h flute_md5.h flute_cb.h FluteFec.h
flute_cb.o: FluteFile.h FluteFileList.h
FluteFDT.o: flute_includes.h
FluteFDT.o: /usr/local/include/xercesc/util/PlatformUtils.hpp
FluteFDT.o: /usr/local/include/xercesc/util/XMLException.hpp
FluteFDT.o: /usr/local/include/xercesc/util/XMemory.hpp
FluteFDT.o: /usr/local/include/xercesc/util/XercesDefs.hpp
FluteFDT.o: /usr/local/include/xercesc/util/XercesVersion.hpp
FluteFDT.o: /usr/local/include/xercesc/util/AutoSense.hpp
FluteFDT.o: /usr/local/include/xercesc/util/XMLExceptMsgs.hpp
FluteFDT.o: /usr/local/include/xercesc/dom/DOMError.hpp
FluteFDT.o: /usr/local/include/xercesc/util/XMLUni.hpp
FluteFDT.o: /usr/local/include/xercesc/framework/XMLErrorReporter.hpp
FluteFDT.o: /usr/local/include/xercesc/util/PanicHandler.hpp
FluteFDT.o: /usr/local/include/xercesc/dom/DOM.hpp
FluteFDT.o: /usr/local/include/xercesc/dom/DOMAttr.hpp
FluteFDT.o: /usr/local/include/xercesc/dom/DOMNode.hpp
FluteFDT.o: /usr/local/include/xercesc/dom/DOMCDATASection.hpp
FluteFDT.o: /usr/local/include/xercesc/dom/DOMText.hpp
FluteFDT.o: /usr/local/include/xercesc/dom/DOMCharacterData.hpp
FluteFDT.o: /usr/local/include/xercesc/dom/DOMComment.hpp
FluteFDT.o: /usr/local/include/xercesc/dom/DOMDocument.hpp
FluteFDT.o: /usr/local/include/xercesc/dom/DOMDocumentRange.hpp
FluteFDT.o: /usr/local/include/xercesc/dom/DOMDocumentTraversal.hpp
FluteFDT.o: /usr/local/include/xercesc/dom/DOMXPathEvaluator.hpp
FluteFDT.o: /usr/local/include/xercesc/dom/DOMDocumentFragment.hpp
FluteFDT.o: /usr/local/include/xercesc/dom/DOMDocumentType.hpp
FluteFDT.o: /usr/local/include/xercesc/dom/DOMElement.hpp
FluteFDT.o: /usr/local/include/xercesc/dom/DOMEntity.hpp
FluteFDT.o: /usr/local/include/xercesc/dom/DOMEntityReference.hpp
FluteFDT.o: /usr/local/include/xercesc/dom/DOMException.hpp
FluteFDT.o: /usr/local/include/xercesc/dom/DOMImplementation.hpp
FluteFDT.o: /usr/local/include/xercesc/dom/DOMImplementationLS.hpp
FluteFDT.o: /usr/local/include/xercesc/dom/DOMRangeException.hpp
FluteFDT.o: /usr/local/include/xercesc/dom/DOMNamedNodeMap.hpp
FluteFDT.o: /usr/local/include/xercesc/dom/DOMNodeList.hpp
FluteFDT.o: /usr/local/include/xercesc/dom/DOMNotation.hpp
FluteFDT.o: /usr/local/include/xercesc/dom/DOMProcessingInstruction.hpp
FluteFDT.o: /usr/local/include/xercesc/dom/DOMNodeFilter.hpp
FluteFDT.o: /usr/local/include/xercesc/dom/DOMNodeIterator.hpp
FluteFDT.o: /usr/local/include/xercesc/dom/DOMRange.hpp
FluteFDT.o: /usr/local/include/xercesc/dom/DOMTreeWalker.hpp
FluteFDT.o: /usr/local/include/xercesc/dom/DOMBuilder.hpp
FluteFDT.o: /usr/local/include/xercesc/dom/DOMConfiguration.hpp
FluteFDT.o: /usr/local/include/xercesc/util/XMLString.hpp
FluteFDT.o: /usr/local/include/xercesc/util/BaseRefVectorOf.hpp
FluteFDT.o: /usr/local/include/xercesc/util/ArrayIndexOutOfBoundsException.hpp
FluteFDT.o: /usr/local/include/xercesc/util/XMLEnumerator.hpp
FluteFDT.o: /usr/local/include/xercesc/framework/MemoryManager.hpp
FluteFDT.o: /usr/local/include/xercesc/util/BaseRefVectorOf.c
FluteFDT.o: /usr/local/include/xercesc/framework/XMLBuffer.hpp
FluteFDT.o: /usr/local/include/xercesc/dom/DOMEntityResolver.hpp
FluteFDT.o: /usr/local/include/xercesc/dom/DOMErrorHandler.hpp
FluteFDT.o: /usr/local/include/xercesc/dom/DOMImplementationRegistry.hpp
FluteFDT.o: /usr/local/include/xercesc/dom/DOMImplementationSource.hpp
FluteFDT.o: /usr/local/include/xercesc/dom/DOMInputSource.hpp
FluteFDT.o: /usr/local/include/xercesc/dom/DOMLocator.hpp
FluteFDT.o: /usr/local/include/xercesc/dom/DOMTypeInfo.hpp
FluteFDT.o: /usr/local/include/xercesc/dom/DOMUserDataHandler.hpp
FluteFDT.o: /usr/local/include/xercesc/dom/DOMWriter.hpp
FluteFDT.o: /usr/local/include/xercesc/dom/DOMWriterFilter.hpp
FluteFDT.o: /usr/local/include/xercesc/framework/XMLFormatter.hpp
FluteFDT.o: /usr/local/include/xercesc/dom/DOMXPathNSResolver.hpp
FluteFDT.o: /usr/local/include/xercesc/dom/DOMXPathException.hpp
FluteFDT.o: /usr/local/include/xercesc/dom/DOMXPathExpression.hpp
FluteFDT.o: /usr/local/include/xercesc/dom/DOMXPathResult.hpp
FluteFDT.o: /usr/local/include/xercesc/dom/DOMXPathNamespace.hpp
FluteFDT.o: /usr/local/include/xercesc/framework/Wrapper4InputSource.hpp
FluteFDT.o: /usr/local/include/xercesc/framework/MemBufInputSource.hpp
FluteFDT.o: /usr/local/include/xercesc/sax/InputSource.hpp
FluteFDT.o: /usr/local/include/xercesc/framework/MemBufFormatTarget.hpp
FluteFDT.o: FluteAPI.h ../common/mcl_lib_api.h ../common/mcl_types.h
FluteFDT.o: ../alc/mcl_lib_api_alc.h Flute.h FluteReceiver.h FluteSender.h
FluteFDT.o: FluteFileInfo.h FluteFileDeliveryCarousel.h macros.h FluteTools.h
FluteFDT.o: flute_lock.h FluteFDT.h FluteSDP.h ../sdp_lib/src/SDP_Parser.h
FluteFDT.o: ../sdp_lib/src/SDP_Error.h ../sdp_lib/src/SDP_EventStreamParser.h
FluteFDT.o: ../sdp_lib/src/SDP_StreamTokenizer.h ../sdp_lib/src/SDP_Utility.h
FluteFDT.o: ../sdp_lib/src/SDP_Description.h ../sdp_lib/src/SDP_Str.h
FluteFDT.o: ../sdp_lib/src/SDP_LinkedList.h ../sdp_lib/src/SDP_Generator.h
FluteFDT.o: ../sdp_lib/src/SDP_Generator.h flute_md5.h flute_cb.h FluteFec.h
FluteFDT.o: FluteFile.h FluteFileList.h
flute_md5.o: flute_includes.h
flute_md5.o: /usr/local/include/xercesc/util/PlatformUtils.hpp
flute_md5.o: /usr/local/include/xercesc/util/XMLException.hpp
flute_md5.o: /usr/local/include/xercesc/util/XMemory.hpp
flute_md5.o: /usr/local/include/xercesc/util/XercesDefs.hpp
flute_md5.o: /usr/local/include/xercesc/util/XercesVersion.hpp
flute_md5.o: /usr/local/include/xercesc/util/AutoSense.hpp
flute_md5.o: /usr/local/include/xercesc/util/XMLExceptMsgs.hpp
flute_md5.o: /usr/local/include/xercesc/dom/DOMError.hpp
flute_md5.o: /usr/local/include/xercesc/util/XMLUni.hpp
flute_md5.o: /usr/local/include/xercesc/framework/XMLErrorReporter.hpp
flute_md5.o: /usr/local/include/xercesc/util/PanicHandler.hpp
flute_md5.o: /usr/local/include/xercesc/dom/DOM.hpp
flute_md5.o: /usr/local/include/xercesc/dom/DOMAttr.hpp
flute_md5.o: /usr/local/include/xercesc/dom/DOMNode.hpp
flute_md5.o: /usr/local/include/xercesc/dom/DOMCDATASection.hpp
flute_md5.o: /usr/local/include/xercesc/dom/DOMText.hpp
flute_md5.o: /usr/local/include/xercesc/dom/DOMCharacterData.hpp
flute_md5.o: /usr/local/include/xercesc/dom/DOMComment.hpp
flute_md5.o: /usr/local/include/xercesc/dom/DOMDocument.hpp
flute_md5.o: /usr/local/include/xercesc/dom/DOMDocumentRange.hpp
flute_md5.o: /usr/local/include/xercesc/dom/DOMDocumentTraversal.hpp
flute_md5.o: /usr/local/include/xercesc/dom/DOMXPathEvaluator.hpp
flute_md5.o: /usr/local/include/xercesc/dom/DOMDocumentFragment.hpp
flute_md5.o: /usr/local/include/xercesc/dom/DOMDocumentType.hpp
flute_md5.o: /usr/local/include/xercesc/dom/DOMElement.hpp
flute_md5.o: /usr/local/include/xercesc/dom/DOMEntity.hpp
flute_md5.o: /usr/local/include/xercesc/dom/DOMEntityReference.hpp
flute_md5.o: /usr/local/include/xercesc/dom/DOMException.hpp
flute_md5.o: /usr/local/include/xercesc/dom/DOMImplementation.hpp
flute_md5.o: /usr/local/include/xercesc/dom/DOMImplementationLS.hpp
flute_md5.o: /usr/local/include/xercesc/dom/DOMRangeException.hpp
flute_md5.o: /usr/local/include/xercesc/dom/DOMNamedNodeMap.hpp
flute_md5.o: /usr/local/include/xercesc/dom/DOMNodeList.hpp
flute_md5.o: /usr/local/include/xercesc/dom/DOMNotation.hpp
flute_md5.o: /usr/local/include/xercesc/dom/DOMProcessingInstruction.hpp
flute_md5.o: /usr/local/include/xercesc/dom/DOMNodeFilter.hpp
flute_md5.o: /usr/local/include/xercesc/dom/DOMNodeIterator.hpp
flute_md5.o: /usr/local/include/xercesc/dom/DOMRange.hpp
flute_md5.o: /usr/local/include/xercesc/dom/DOMTreeWalker.hpp
flute_md5.o: /usr/local/include/xercesc/dom/DOMBuilder.hpp
flute_md5.o: /usr/local/include/xercesc/dom/DOMConfiguration.hpp
flute_md5.o: /usr/local/include/xercesc/util/XMLString.hpp
flute_md5.o: /usr/local/include/xercesc/util/BaseRefVectorOf.hpp
flute_md5.o: /usr/local/include/xercesc/util/ArrayIndexOutOfBoundsException.hpp
flute_md5.o: /usr/local/include/xercesc/util/XMLEnumerator.hpp
flute_md5.o: /usr/local/include/xercesc/framework/MemoryManager.hpp
flute_md5.o: /usr/local/include/xercesc/util/BaseRefVectorOf.c
flute_md5.o: /usr/local/include/xercesc/framework/XMLBuffer.hpp
flute_md5.o: /usr/local/include/xercesc/dom/DOMEntityResolver.hpp
flute_md5.o: /usr/local/include/xercesc/dom/DOMErrorHandler.hpp
flute_md5.o: /usr/local/include/xercesc/dom/DOMImplementationRegistry.hpp
flute_md5.o: /usr/local/include/xercesc/dom/DOMImplementationSource.hpp
flute_md5.o: /usr/local/include/xercesc/dom/DOMInputSource.hpp
flute_md5.o: /usr/local/include/xercesc/dom/DOMLocator.hpp
flute_md5.o: /usr/local/include/xercesc/dom/DOMTypeInfo.hpp
flute_md5.o: /usr/local/include/xercesc/dom/DOMUserDataHandler.hpp
flute_md5.o: /usr/local/include/xercesc/dom/DOMWriter.hpp
flute_md5.o: /usr/local/include/xercesc/dom/DOMWriterFilter.hpp
flute_md5.o: /usr/local/include/xercesc/framework/XMLFormatter.hpp
flute_md5.o: /usr/local/include/xercesc/dom/DOMXPathNSResolver.hpp
flute_md5.o: /usr/local/include/xercesc/dom/DOMXPathException.hpp
flute_md5.o: /usr/local/include/xercesc/dom/DOMXPathExpression.hpp
flute_md5.o: /usr/local/include/xercesc/dom/DOMXPathResult.hpp
flute_md5.o: /usr/local/include/xercesc/dom/DOMXPathNamespace.hpp
flute_md5.o: /usr/local/include/xercesc/framework/Wrapper4InputSource.hpp
flute_md5.o: /usr/local/include/xercesc/framework/MemBufInputSource.hpp
flute_md5.o: /usr/local/include/xercesc/sax/InputSource.hpp
flute_md5.o: /usr/local/include/xercesc/framework/MemBufFormatTarget.hpp
flute_md5.o: FluteAPI.h ../common/mcl_lib_api.h ../common/mcl_types.h
flute_md5.o: ../alc/mcl_lib_api_alc.h Flute.h FluteReceiver.h FluteSender.h
flute_md5.o: FluteFileInfo.h FluteFileDeliveryCarousel.h macros.h
flute_md5.o: FluteTools.h flute_lock.h FluteFDT.h FluteSDP.h
flute_md5.o: ../sdp_lib/src/SDP_Parser.h ../sdp_lib/src/SDP_Error.h
flute_md5.o: ../sdp_lib/src/SDP_EventStreamParser.h
flute_md5.o: ../sdp_lib/src/SDP_StreamTokenizer.h
flute_md5.o: ../sdp_lib/src/SDP_Utility.h ../sdp_lib/src/SDP_Description.h
flute_md5.o: ../sdp_lib/src/SDP_Str.h ../sdp_lib/src/SDP_LinkedList.h
flute_md5.o: ../sdp_lib/src/SDP_Generator.h ../sdp_lib/src/SDP_Generator.h
flute_md5.o: flute_md5.h flute_cb.h FluteFec.h FluteFile.h FluteFileList.h
flute_lock.o: flute_includes.h
flute_lock.o: /usr/local/include/xercesc/util/PlatformUtils.hpp
flute_lock.o: /usr/local/include/xercesc/util/XMLException.hpp
flute_lock.o: /usr/local/include/xercesc/util/XMemory.hpp
flute_lock.o: /usr/local/include/xercesc/util/XercesDefs.hpp
flute_lock.o: /usr/local/include/xercesc/util/XercesVersion.hpp
flute_lock.o: /usr/local/include/xercesc/util/AutoSense.hpp
flute_lock.o: /usr/local/include/xercesc/util/XMLExceptMsgs.hpp
flute_lock.o: /usr/local/include/xercesc/dom/DOMError.hpp
flute_lock.o: /usr/local/include/xercesc/util/XMLUni.hpp
flute_lock.o: /usr/local/include/xercesc/framework/XMLErrorReporter.hpp
flute_lock.o: /usr/local/include/xercesc/util/PanicHandler.hpp
flute_lock.o: /usr/local/include/xercesc/dom/DOM.hpp
flute_lock.o: /usr/local/include/xercesc/dom/DOMAttr.hpp
flute_lock.o: /usr/local/include/xercesc/dom/DOMNode.hpp
flute_lock.o: /usr/local/include/xercesc/dom/DOMCDATASection.hpp
flute_lock.o: /usr/local/include/xercesc/dom/DOMText.hpp
flute_lock.o: /usr/local/include/xercesc/dom/DOMCharacterData.hpp
flute_lock.o: /usr/local/include/xercesc/dom/DOMComment.hpp
flute_lock.o: /usr/local/include/xercesc/dom/DOMDocument.hpp
flute_lock.o: /usr/local/include/xercesc/dom/DOMDocumentRange.hpp
flute_lock.o: /usr/local/include/xercesc/dom/DOMDocumentTraversal.hpp
flute_lock.o: /usr/local/include/xercesc/dom/DOMXPathEvaluator.hpp
flute_lock.o: /usr/local/include/xercesc/dom/DOMDocumentFragment.hpp
flute_lock.o: /usr/local/include/xercesc/dom/DOMDocumentType.hpp
flute_lock.o: /usr/local/include/xercesc/dom/DOMElement.hpp
flute_lock.o: /usr/local/include/xercesc/dom/DOMEntity.hpp
flute_lock.o: /usr/local/include/xercesc/dom/DOMEntityReference.hpp
flute_lock.o: /usr/local/include/xercesc/dom/DOMException.hpp
flute_lock.o: /usr/local/include/xercesc/dom/DOMImplementation.hpp
flute_lock.o: /usr/local/include/xercesc/dom/DOMImplementationLS.hpp
flute_lock.o: /usr/local/include/xercesc/dom/DOMRangeException.hpp
flute_lock.o: /usr/local/include/xercesc/dom/DOMNamedNodeMap.hpp
flute_lock.o: /usr/local/include/xercesc/dom/DOMNodeList.hpp
flute_lock.o: /usr/local/include/xercesc/dom/DOMNotation.hpp
flute_lock.o: /usr/local/include/xercesc/dom/DOMProcessingInstruction.hpp
flute_lock.o: /usr/local/include/xercesc/dom/DOMNodeFilter.hpp
flute_lock.o: /usr/local/include/xercesc/dom/DOMNodeIterator.hpp
flute_lock.o: /usr/local/include/xercesc/dom/DOMRange.hpp
flute_lock.o: /usr/local/include/xercesc/dom/DOMTreeWalker.hpp
flute_lock.o: /usr/local/include/xercesc/dom/DOMBuilder.hpp
flute_lock.o: /usr/local/include/xercesc/dom/DOMConfiguration.hpp
flute_lock.o: /usr/local/include/xercesc/util/XMLString.hpp
flute_lock.o: /usr/local/include/xercesc/util/BaseRefVectorOf.hpp
flute_lock.o: /usr/local/include/xercesc/util/ArrayIndexOutOfBoundsException.hpp
flute_lock.o: /usr/local/include/xercesc/util/XMLEnumerator.hpp
flute_lock.o: /usr/local/include/xercesc/framework/MemoryManager.hpp
flute_lock.o: /usr/local/include/xercesc/util/BaseRefVectorOf.c
flute_lock.o: /usr/local/include/xercesc/framework/XMLBuffer.hpp
flute_lock.o: /usr/local/include/xercesc/dom/DOMEntityResolver.hpp
flute_lock.o: /usr/local/include/xercesc/dom/DOMErrorHandler.hpp
flute_lock.o: /usr/local/include/xercesc/dom/DOMImplementationRegistry.hpp
flute_lock.o: /usr/local/include/xercesc/dom/DOMImplementationSource.hpp
flute_lock.o: /usr/local/include/xercesc/dom/DOMInputSource.hpp
flute_lock.o: /usr/local/include/xercesc/dom/DOMLocator.hpp
flute_lock.o: /usr/local/include/xercesc/dom/DOMTypeInfo.hpp
flute_lock.o: /usr/local/include/xercesc/dom/DOMUserDataHandler.hpp
flute_lock.o: /usr/local/include/xercesc/dom/DOMWriter.hpp
flute_lock.o: /usr/local/include/xercesc/dom/DOMWriterFilter.hpp
flute_lock.o: /usr/local/include/xercesc/framework/XMLFormatter.hpp
flute_lock.o: /usr/local/include/xercesc/dom/DOMXPathNSResolver.hpp
flute_lock.o: /usr/local/include/xercesc/dom/DOMXPathException.hpp
flute_lock.o: /usr/local/include/xercesc/dom/DOMXPathExpression.hpp
flute_lock.o: /usr/local/include/xercesc/dom/DOMXPathResult.hpp
flute_lock.o: /usr/local/include/xercesc/dom/DOMXPathNamespace.hpp
flute_lock.o: /usr/local/include/xercesc/framework/Wrapper4InputSource.hpp
flute_lock.o: /usr/local/include/xercesc/framework/MemBufInputSource.hpp
flute_lock.o: /usr/local/include/xercesc/sax/InputSource.hpp
flute_lock.o: /usr/local/include/xercesc/framework/MemBufFormatTarget.hpp
flute_lock.o: FluteAPI.h ../common/mcl_lib_api.h ../common/mcl_types.h
flute_lock.o: ../alc/mcl_lib_api_alc.h Flute.h FluteReceiver.h FluteSender.h
flute_lock.o: FluteFileInfo.h FluteFileDeliveryCarousel.h macros.h
flute_lock.o: FluteTools.h flute_lock.h FluteFDT.h FluteSDP.h
flute_lock.o: ../sdp_lib/src/SDP_Parser.h ../sdp_lib/src/SDP_Error.h
flute_lock.o: ../sdp_lib/src/SDP_EventStreamParser.h
flute_lock.o: ../sdp_lib/src/SDP_StreamTokenizer.h
flute_lock.o: ../sdp_lib/src/SDP_Utility.h ../sdp_lib/src/SDP_Description.h
flute_lock.o: ../sdp_lib/src/SDP_Str.h ../sdp_lib/src/SDP_LinkedList.h
flute_lock.o: ../sdp_lib/src/SDP_Generator.h ../sdp_lib/src/SDP_Generator.h
flute_lock.o: flute_md5.h flute_cb.h FluteFec.h FluteFile.h FluteFileList.h
FluteSDP.o: flute_includes.h
FluteSDP.o: /usr/local/include/xercesc/util/PlatformUtils.hpp
FluteSDP.o: /usr/local/include/xercesc/util/XMLException.hpp
FluteSDP.o: /usr/local/include/xercesc/util/XMemory.hpp
FluteSDP.o: /usr/local/include/xercesc/util/XercesDefs.hpp
FluteSDP.o: /usr/local/include/xercesc/util/XercesVersion.hpp
FluteSDP.o: /usr/local/include/xercesc/util/AutoSense.hpp
FluteSDP.o: /usr/local/include/xercesc/util/XMLExceptMsgs.hpp
FluteSDP.o: /usr/local/include/xercesc/dom/DOMError.hpp
FluteSDP.o: /usr/local/include/xercesc/util/XMLUni.hpp
FluteSDP.o: /usr/local/include/xercesc/framework/XMLErrorReporter.hpp
FluteSDP.o: /usr/local/include/xercesc/util/PanicHandler.hpp
FluteSDP.o: /usr/local/include/xercesc/dom/DOM.hpp
FluteSDP.o: /usr/local/include/xercesc/dom/DOMAttr.hpp
FluteSDP.o: /usr/local/include/xercesc/dom/DOMNode.hpp
FluteSDP.o: /usr/local/include/xercesc/dom/DOMCDATASection.hpp
FluteSDP.o: /usr/local/include/xercesc/dom/DOMText.hpp
FluteSDP.o: /usr/local/include/xercesc/dom/DOMCharacterData.hpp
FluteSDP.o: /usr/local/include/xercesc/dom/DOMComment.hpp
FluteSDP.o: /usr/local/include/xercesc/dom/DOMDocument.hpp
FluteSDP.o: /usr/local/include/xercesc/dom/DOMDocumentRange.hpp
FluteSDP.o: /usr/local/include/xercesc/dom/DOMDocumentTraversal.hpp
FluteSDP.o: /usr/local/include/xercesc/dom/DOMXPathEvaluator.hpp
FluteSDP.o: /usr/local/include/xercesc/dom/DOMDocumentFragment.hpp
FluteSDP.o: /usr/local/include/xercesc/dom/DOMDocumentType.hpp
FluteSDP.o: /usr/local/include/xercesc/dom/DOMElement.hpp
FluteSDP.o: /usr/local/include/xercesc/dom/DOMEntity.hpp
FluteSDP.o: /usr/local/include/xercesc/dom/DOMEntityReference.hpp
FluteSDP.o: /usr/local/include/xercesc/dom/DOMException.hpp
FluteSDP.o: /usr/local/include/xercesc/dom/DOMImplementation.hpp
FluteSDP.o: /usr/local/include/xercesc/dom/DOMImplementationLS.hpp
FluteSDP.o: /usr/local/include/xercesc/dom/DOMRangeException.hpp
FluteSDP.o: /usr/local/include/xercesc/dom/DOMNamedNodeMap.hpp
FluteSDP.o: /usr/local/include/xercesc/dom/DOMNodeList.hpp
FluteSDP.o: /usr/local/include/xercesc/dom/DOMNotation.hpp
FluteSDP.o: /usr/local/include/xercesc/dom/DOMProcessingInstruction.hpp
FluteSDP.o: /usr/local/include/xercesc/dom/DOMNodeFilter.hpp
FluteSDP.o: /usr/local/include/xercesc/dom/DOMNodeIterator.hpp
FluteSDP.o: /usr/local/include/xercesc/dom/DOMRange.hpp
FluteSDP.o: /usr/local/include/xercesc/dom/DOMTreeWalker.hpp
FluteSDP.o: /usr/local/include/xercesc/dom/DOMBuilder.hpp
FluteSDP.o: /usr/local/include/xercesc/dom/DOMConfiguration.hpp
FluteSDP.o: /usr/local/include/xercesc/util/XMLString.hpp
FluteSDP.o: /usr/local/include/xercesc/util/BaseRefVectorOf.hpp
FluteSDP.o: /usr/local/include/xercesc/util/ArrayIndexOutOfBoundsException.hpp
FluteSDP.o: /usr/local/include/xercesc/util/XMLEnumerator.hpp
FluteSDP.o: /usr/local/include/xercesc/framework/MemoryManager.hpp
FluteSDP.o: /usr/local/include/xercesc/util/BaseRefVectorOf.c
FluteSDP.o: /usr/local/include/xercesc/framework/XMLBuffer.hpp
FluteSDP.o: /usr/local/include/xercesc/dom/DOMEntityResolver.hpp
FluteSDP.o: /usr/local/include/xercesc/dom/DOMErrorHandler.hpp
FluteSDP.o: /usr/local/include/xercesc/dom/DOMImplementationRegistry.hpp
FluteSDP.o: /usr/local/include/xercesc/dom/DOMImplementationSource.hpp
FluteSDP.o: /usr/local/include/xercesc/dom/DOMInputSource.hpp
FluteSDP.o: /usr/local/include/xercesc/dom/DOMLocator.hpp
FluteSDP.o: /usr/local/include/xercesc/dom/DOMTypeInfo.hpp
FluteSDP.o: /usr/local/include/xercesc/dom/DOMUserDataHandler.hpp
FluteSDP.o: /usr/local/include/xercesc/dom/DOMWriter.hpp
FluteSDP.o: /usr/local/include/xercesc/dom/DOMWriterFilter.hpp
FluteSDP.o: /usr/local/include/xercesc/framework/XMLFormatter.hpp
FluteSDP.o: /usr/local/include/xercesc/dom/DOMXPathNSResolver.hpp
FluteSDP.o: /usr/local/include/xercesc/dom/DOMXPathException.hpp
FluteSDP.o: /usr/local/include/xercesc/dom/DOMXPathExpression.hpp
FluteSDP.o: /usr/local/include/xercesc/dom/DOMXPathResult.hpp
FluteSDP.o: /usr/local/include/xercesc/dom/DOMXPathNamespace.hpp
FluteSDP.o: /usr/local/include/xercesc/framework/Wrapper4InputSource.hpp
FluteSDP.o: /usr/local/include/xercesc/framework/MemBufInputSource.hpp
FluteSDP.o: /usr/local/include/xercesc/sax/InputSource.hpp
FluteSDP.o: /usr/local/include/xercesc/framework/MemBufFormatTarget.hpp
FluteSDP.o: FluteAPI.h ../common/mcl_lib_api.h ../common/mcl_types.h
FluteSDP.o: ../alc/mcl_lib_api_alc.h Flute.h FluteReceiver.h FluteSender.h
FluteSDP.o: FluteFileInfo.h FluteFileDeliveryCarousel.h macros.h FluteTools.h
FluteSDP.o: flute_lock.h FluteFDT.h FluteSDP.h ../sdp_lib/src/SDP_Parser.h
FluteSDP.o: ../sdp_lib/src/SDP_Error.h ../sdp_lib/src/SDP_EventStreamParser.h
FluteSDP.o: ../sdp_lib/src/SDP_StreamTokenizer.h ../sdp_lib/src/SDP_Utility.h
FluteSDP.o: ../sdp_lib/src/SDP_Description.h ../sdp_lib/src/SDP_Str.h
FluteSDP.o: ../sdp_lib/src/SDP_LinkedList.h ../sdp_lib/src/SDP_Generator.h
FluteSDP.o: ../sdp_lib/src/SDP_Generator.h flute_md5.h flute_cb.h FluteFec.h
FluteSDP.o: FluteFile.h FluteFileList.h
FluteTools.o: flute_includes.h
FluteTools.o: /usr/local/include/xercesc/util/PlatformUtils.hpp
FluteTools.o: /usr/local/include/xercesc/util/XMLException.hpp
FluteTools.o: /usr/local/include/xercesc/util/XMemory.hpp
FluteTools.o: /usr/local/include/xercesc/util/XercesDefs.hpp
FluteTools.o: /usr/local/include/xercesc/util/XercesVersion.hpp
FluteTools.o: /usr/local/include/xercesc/util/AutoSense.hpp
FluteTools.o: /usr/local/include/xercesc/util/XMLExceptMsgs.hpp
FluteTools.o: /usr/local/include/xercesc/dom/DOMError.hpp
FluteTools.o: /usr/local/include/xercesc/util/XMLUni.hpp
FluteTools.o: /usr/local/include/xercesc/framework/XMLErrorReporter.hpp
FluteTools.o: /usr/local/include/xercesc/util/PanicHandler.hpp
FluteTools.o: /usr/local/include/xercesc/dom/DOM.hpp
FluteTools.o: /usr/local/include/xercesc/dom/DOMAttr.hpp
FluteTools.o: /usr/local/include/xercesc/dom/DOMNode.hpp
FluteTools.o: /usr/local/include/xercesc/dom/DOMCDATASection.hpp
FluteTools.o: /usr/local/include/xercesc/dom/DOMText.hpp
FluteTools.o: /usr/local/include/xercesc/dom/DOMCharacterData.hpp
FluteTools.o: /usr/local/include/xercesc/dom/DOMComment.hpp
FluteTools.o: /usr/local/include/xercesc/dom/DOMDocument.hpp
FluteTools.o: /usr/local/include/xercesc/dom/DOMDocumentRange.hpp
FluteTools.o: /usr/local/include/xercesc/dom/DOMDocumentTraversal.hpp
FluteTools.o: /usr/local/include/xercesc/dom/DOMXPathEvaluator.hpp
FluteTools.o: /usr/local/include/xercesc/dom/DOMDocumentFragment.hpp
FluteTools.o: /usr/local/include/xercesc/dom/DOMDocumentType.hpp
FluteTools.o: /usr/local/include/xercesc/dom/DOMElement.hpp
FluteTools.o: /usr/local/include/xercesc/dom/DOMEntity.hpp
FluteTools.o: /usr/local/include/xercesc/dom/DOMEntityReference.hpp
FluteTools.o: /usr/local/include/xercesc/dom/DOMException.hpp
FluteTools.o: /usr/local/include/xercesc/dom/DOMImplementation.hpp
FluteTools.o: /usr/local/include/xercesc/dom/DOMImplementationLS.hpp
FluteTools.o: /usr/local/include/xercesc/dom/DOMRangeException.hpp
FluteTools.o: /usr/local/include/xercesc/dom/DOMNamedNodeMap.hpp
FluteTools.o: /usr/local/include/xercesc/dom/DOMNodeList.hpp
FluteTools.o: /usr/local/include/xercesc/dom/DOMNotation.hpp
FluteTools.o: /usr/local/include/xercesc/dom/DOMProcessingInstruction.hpp
FluteTools.o: /usr/local/include/xercesc/dom/DOMNodeFilter.hpp
FluteTools.o: /usr/local/include/xercesc/dom/DOMNodeIterator.hpp
FluteTools.o: /usr/local/include/xercesc/dom/DOMRange.hpp
FluteTools.o: /usr/local/include/xercesc/dom/DOMTreeWalker.hpp
FluteTools.o: /usr/local/include/xercesc/dom/DOMBuilder.hpp
FluteTools.o: /usr/local/include/xercesc/dom/DOMConfiguration.hpp
FluteTools.o: /usr/local/include/xercesc/util/XMLString.hpp
FluteTools.o: /usr/local/include/xercesc/util/BaseRefVectorOf.hpp
FluteTools.o: /usr/local/include/xercesc/util/ArrayIndexOutOfBoundsException.hpp
FluteTools.o: /usr/local/include/xercesc/util/XMLEnumerator.hpp
FluteTools.o: /usr/local/include/xercesc/framework/MemoryManager.hpp
FluteTools.o: /usr/local/include/xercesc/util/BaseRefVectorOf.c
FluteTools.o: /usr/local/include/xercesc/framework/XMLBuffer.hpp
FluteTools.o: /usr/local/include/xercesc/dom/DOMEntityResolver.hpp
FluteTools.o: /usr/local/include/xercesc/dom/DOMErrorHandler.hpp
FluteTools.o: /usr/local/include/xercesc/dom/DOMImplementationRegistry.hpp
FluteTools.o: /usr/local/include/xercesc/dom/DOMImplementationSource.hpp
FluteTools.o: /usr/local/include/xercesc/dom/DOMInputSource.hpp
FluteTools.o: /usr/local/include/xercesc/dom/DOMLocator.hpp
FluteTools.o: /usr/local/include/xercesc/dom/DOMTypeInfo.hpp
FluteTools.o: /usr/local/include/xercesc/dom/DOMUserDataHandler.hpp
FluteTools.o: /usr/local/include/xercesc/dom/DOMWriter.hpp
FluteTools.o: /usr/local/include/xercesc/dom/DOMWriterFilter.hpp
FluteTools.o: /usr/local/include/xercesc/framework/XMLFormatter.hpp
FluteTools.o: /usr/local/include/xercesc/dom/DOMXPathNSResolver.hpp
FluteTools.o: /usr/local/include/xercesc/dom/DOMXPathException.hpp
FluteTools.o: /usr/local/include/xercesc/dom/DOMXPathExpression.hpp
FluteTools.o: /usr/local/include/xercesc/dom/DOMXPathResult.hpp
FluteTools.o: /usr/local/include/xercesc/dom/DOMXPathNamespace.hpp
FluteTools.o: /usr/local/include/xercesc/framework/Wrapper4InputSource.hpp
FluteTools.o: /usr/local/include/xercesc/framework/MemBufInputSource.hpp
FluteTools.o: /usr/local/include/xercesc/sax/InputSource.hpp
FluteTools.o: /usr/local/include/xercesc/framework/MemBufFormatTarget.hpp
FluteTools.o: FluteAPI.h ../common/mcl_lib_api.h ../common/mcl_types.h
FluteTools.o: ../alc/mcl_lib_api_alc.h Flute.h FluteReceiver.h FluteSender.h
FluteTools.o: FluteFileInfo.h FluteFileDeliveryCarousel.h macros.h
FluteTools.o: FluteTools.h flute_lock.h FluteFDT.h FluteSDP.h
FluteTools.o: ../sdp_lib/src/SDP_Parser.h ../sdp_lib/src/SDP_Error.h
FluteTools.o: ../sdp_lib/src/SDP_EventStreamParser.h
FluteTools.o: ../sdp_lib/src/SDP_StreamTokenizer.h
FluteTools.o: ../sdp_lib/src/SDP_Utility.h ../sdp_lib/src/SDP_Description.h
FluteTools.o: ../sdp_lib/src/SDP_Str.h ../sdp_lib/src/SDP_LinkedList.h
FluteTools.o: ../sdp_lib/src/SDP_Generator.h ../sdp_lib/src/SDP_Generator.h
FluteTools.o: flute_md5.h flute_cb.h FluteFec.h FluteFile.h FluteFileList.h
