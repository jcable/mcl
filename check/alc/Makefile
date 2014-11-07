# $Id: Makefile,v 1.27 2005/02/25 14:13:09 moi Exp $
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
	# Edit the following two lines as appropriate, according to the FEC
	# codec in use: if LDPC is used, then add the libbdpc.a library.
	#
	#LIBRARIES = -lc -lm -lpthread ${BINDIR}/libmclalc.a
	LIBRARIES = -L${BINDIR} -lmclalc -lldpc -lc -lm -lpthread 
endif

ifeq (${OS},solaris)
	# Solaris version
	#
	BINDIR = ../../bin/solaris
	LDPC_BINDIR = ../../bin/solaris
	# Edit the following two lines as appropriate, according to the FEC
	# codec in use: if LDPC is used, then add the libbdpc.a library.
	#
	#LIBRARIES = -lc -lm -lsocket -lnsl -L/usr/ucblib -lucb -lposix4 -mt -lpthread ${BINDIR}/libmclalc.a
	LIBRARIES = -lc -lm -lsocket -lnsl -L/usr/ucblib -lucb -lposix4 -mt -lpthread ${BINDIR}/libmclalc.a ${LDPC_BINDIR}/libldpc.a
endif

ifeq (${OS},freebsd)
	# Linux version
	#
	BINDIR = ../../bin/freebsd
	LDPC_BINDIR = ../../bin/freebsd
	# Edit the following two lines as appropriate, according to the FEC
	# codec in use: if LDPC is used, then add the libbdpc.a library.
	#
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
SRCFILES = test_several_fec_codecs.c test_wait_tx.c test_wait_rx_sender.c test_wait_rx_receiver.c test_no_tx.c test_abort.c test_seq_of_tx_sessions.c test_multi_sessions1.c test_multi_sessions2.c test_demux_label1_tx.c test_demux_label1_rx.c test_demux_label2_tx.c test_demux_label2_rx.c test_dyn_tsi.c test_vtm_tx.c test_vtm_rx.c test_rebootable_client.c test_rebootable_server.c test_reset_transmission.c test_stop_tx_one_adu.c
OFILES	 = ${SRCFILES:.c=.o}
TARGETS  = ${SRCFILES:.c=}



###### Dependancies ######

all:	${TARGETS}

test_several_fec_codecs:	test_several_fec_codecs.o ${BINDIR}/libmclalc.a
	${CC} ${CFLAGS} test_several_fec_codecs.o ${LIBRARIES} -o test_several_fec_codecs
	strip test_several_fec_codecs

test_wait_tx:	test_wait_tx.o ${BINDIR}/libmclalc.a
	${CC} ${CFLAGS} test_wait_tx.o ${LIBRARIES} -o test_wait_tx
	strip test_wait_tx

test_wait_rx_sender:	test_wait_rx_sender.o ${BINDIR}/libmclalc.a
	${CC} ${CFLAGS} test_wait_rx_sender.o ${LIBRARIES} -o test_wait_rx_sender
	strip test_wait_rx_sender

test_wait_rx_receiver:	test_wait_rx_receiver.o ${BINDIR}/libmclalc.a
	${CC} ${CFLAGS} test_wait_rx_receiver.o ${LIBRARIES} -o test_wait_rx_receiver
	strip test_wait_rx_receiver

test_no_tx:	test_no_tx.o ${BINDIR}/libmclalc.a
	${CC} ${CFLAGS} test_no_tx.o ${LIBRARIES} -o test_no_tx
	strip test_no_tx

test_abort:	test_abort.o ${BINDIR}/libmclalc.a
	${CC} ${CFLAGS} test_abort.o ${LIBRARIES} -o test_abort
	strip test_abort

test_seq_of_tx_sessions:	test_seq_of_tx_sessions.o ${BINDIR}/libmclalc.a
	${CC} ${CFLAGS} test_seq_of_tx_sessions.o ${LIBRARIES} -o test_seq_of_tx_sessions
	strip test_seq_of_tx_sessions

test_reset_transmission:	test_reset_transmission.o ${BINDIR}/libmclalc.a
	${CC} ${CFLAGS} test_reset_transmission.o ${LIBRARIES} -o test_reset_transmission
	strip test_reset_transmission

test_stop_tx_one_adu:	test_stop_tx_one_adu.o ${BINDIR}/libmclalc.a
	${CC} ${CFLAGS} test_stop_tx_one_adu.o ${LIBRARIES} -o test_stop_tx_one_adu
	strip test_stop_tx_one_adu


test_multi_sessions1:	test_multi_sessions1.o ${BINDIR}/libmclalc.a
	${CC} ${CFLAGS} test_multi_sessions1.o ${LIBRARIES} -o test_multi_sessions1
	strip test_multi_sessions1

test_multi_sessions2:	test_multi_sessions2.o ${BINDIR}/libmclalc.a
	${CC} ${CFLAGS} test_multi_sessions2.o ${LIBRARIES} -o test_multi_sessions2
	strip test_multi_sessions2

test_demux_label1_tx:	test_demux_label1_tx.o ${BINDIR}/libmclalc.a
	${CC} ${CFLAGS} test_demux_label1_tx.o ${LIBRARIES} -o test_demux_label1_tx
	strip test_demux_label1_tx

test_demux_label1_rx:	test_demux_label1_rx.o ${BINDIR}/libmclalc.a
	${CC} ${CFLAGS} test_demux_label1_rx.o ${LIBRARIES} -o test_demux_label1_rx
	strip test_demux_label1_rx

test_demux_label2_tx:	test_demux_label2_tx.o ${BINDIR}/libmclalc.a
	${CC} ${CFLAGS} test_demux_label2_tx.o ${LIBRARIES} -o test_demux_label2_tx
	strip test_demux_label2_tx

test_demux_label2_rx:	test_demux_label2_rx.o ${BINDIR}/libmclalc.a
	${CC} ${CFLAGS} test_demux_label2_rx.o ${LIBRARIES} -o test_demux_label2_rx
	strip test_demux_label2_rx


test_dyn_tsi:	test_dyn_tsi.o ${BINDIR}/libmclalc.a
	${CC} ${CFLAGS} test_dyn_tsi.o ${LIBRARIES} -o test_dyn_tsi
	#strip test_dyn_tsi

pure:	test_dyn_tsi.o ${BINDIR}/libmclalc.a
	${CCLINK_PURE} ${CFLAGS} test_dyn_tsi.o ${LIBRARIES} -o test_dyn_tsi

test_vtm_tx:	test_vtm_tx.o ${BINDIR}/libmclalc.a
	${CC} ${CFLAGS} test_vtm_tx.o ${LIBRARIES} -o test_vtm_tx
	strip test_vtm_tx

test_vtm_rx:	test_vtm_rx.o ${BINDIR}/libmclalc.a
	${CC} ${CFLAGS} test_vtm_rx.o ${LIBRARIES} -o test_vtm_rx
	strip test_vtm_rx

test_rebootable_client:	test_rebootable_client.o ${BINDIR}/libmclalc.a
	${CC} ${CFLAGS} test_rebootable_client.o ${LIBRARIES} -o test_rebootable_client
	strip test_rebootable_client

test_rebootable_server:	test_rebootable_server.o ${BINDIR}/libmclalc.a
	${CC} ${CFLAGS} test_rebootable_server.o ${LIBRARIES} -o test_rebootable_server
	strip test_rebootable_server

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

test_several_fec_codecs.o: ../../src/common/mcl_lib_api.h
test_several_fec_codecs.o: ../../src/common/mcl_types.h
test_several_fec_codecs.o: ../../src/alc/mcl_lib_api_alc.h
test_wait_tx.o: ../../src/common/mcl_lib_api.h ../../src/common/mcl_types.h
test_wait_tx.o: ../../src/alc/mcl_lib_api_alc.h
test_wait_rx_sender.o: ../../src/common/mcl_lib_api.h
test_wait_rx_sender.o: ../../src/common/mcl_types.h
test_wait_rx_sender.o: ../../src/alc/mcl_lib_api_alc.h
test_wait_rx_receiver.o: ../../src/common/mcl_lib_api.h
test_wait_rx_receiver.o: ../../src/common/mcl_types.h
test_wait_rx_receiver.o: ../../src/alc/mcl_lib_api_alc.h
test_no_tx.o: ../../src/common/mcl_lib_api.h ../../src/common/mcl_types.h
test_no_tx.o: ../../src/alc/mcl_lib_api_alc.h
test_abort.o: ../../src/common/mcl_lib_api.h ../../src/common/mcl_types.h
test_abort.o: ../../src/alc/mcl_lib_api_alc.h
test_seq_of_tx_sessions.o: ../../src/common/mcl_lib_api.h
test_seq_of_tx_sessions.o: ../../src/common/mcl_types.h
test_seq_of_tx_sessions.o: ../../src/alc/mcl_lib_api_alc.h
test_multi_sessions1.o: ../../src/common/mcl_lib_api.h
test_multi_sessions1.o: ../../src/common/mcl_types.h
test_multi_sessions1.o: ../../src/alc/mcl_lib_api_alc.h
test_multi_sessions2.o: ../../src/common/mcl_lib_api.h
test_multi_sessions2.o: ../../src/common/mcl_types.h
test_multi_sessions2.o: ../../src/alc/mcl_lib_api_alc.h
test_demux_label1_tx.o: ../../src/common/mcl_lib_api.h
test_demux_label1_tx.o: ../../src/common/mcl_types.h
test_demux_label1_tx.o: ../../src/alc/mcl_lib_api_alc.h
test_demux_label1_rx.o: ../../src/common/mcl_lib_api.h
test_demux_label1_rx.o: ../../src/common/mcl_types.h
test_demux_label1_rx.o: ../../src/alc/mcl_lib_api_alc.h
test_demux_label2_tx.o: ../../src/common/mcl_lib_api.h
test_demux_label2_tx.o: ../../src/common/mcl_types.h
test_demux_label2_tx.o: ../../src/alc/mcl_lib_api_alc.h
test_demux_label2_rx.o: ../../src/common/mcl_lib_api.h
test_demux_label2_rx.o: ../../src/common/mcl_types.h
test_demux_label2_rx.o: ../../src/alc/mcl_lib_api_alc.h
test_dyn_tsi.o: ../../src/common/mcl_lib_api.h ../../src/common/mcl_types.h
test_dyn_tsi.o: ../../src/alc/mcl_lib_api_alc.h
test_vtm_tx.o: ../../src/common/mcl_lib_api.h ../../src/common/mcl_types.h
test_vtm_tx.o: ../../src/alc/mcl_lib_api_alc.h
test_vtm_rx.o: ../../src/common/mcl_lib_api.h ../../src/common/mcl_types.h
test_vtm_rx.o: ../../src/alc/mcl_lib_api_alc.h
test_rebootable_client.o: ../../src/common/mcl_lib_api.h
test_rebootable_client.o: ../../src/common/mcl_types.h
test_rebootable_client.o: ../../src/alc/mcl_lib_api_alc.h
test_rebootable_server.o: ../../src/common/mcl_lib_api.h
test_rebootable_server.o: ../../src/common/mcl_types.h
test_rebootable_server.o: ../../src/alc/mcl_lib_api_alc.h
test_reset_transmission.o: ../../src/common/mcl_lib_api.h
test_reset_transmission.o: ../../src/common/mcl_types.h
test_reset_transmission.o: ../../src/alc/mcl_lib_api_alc.h
test_stop_tx_one_adu.o: ../../src/common/mcl_lib_api.h
test_stop_tx_one_adu.o: ../../src/common/mcl_types.h
test_stop_tx_one_adu.o: ../../src/alc/mcl_lib_api_alc.h
