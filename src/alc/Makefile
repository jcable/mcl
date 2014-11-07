# $Id: Makefile,v 1.64 2005/05/24 15:43:21 roca Exp $
#
# Makefile
#

###### OS Dependant Definitions ######

# they are defined in the following file...
# edit Makefile.common as appropriate
include ../../Makefile.common


ifeq (${OS},linux)
	# Linux version
	BINDIR = ../../bin/linux
	INCDIR = ../../include
	LDPC_BINDIR = ../../bin/linux
	LIBRARIES = -lc -lm -lpthread
	LARGEFILE_FLAGS = -D_FILE_OFFSET_BITS=64 -D_LARGEFILE_SOURCE
endif

ifeq (${OS},solaris)
	# Solaris version
	BINDIR = ../../bin/solaris
	LDPC_BINDIR = ../../bin/solaris
	LIBRARIES = -lc -lm -lsocket -lnsl -L/usr/ucblib -lucb -lpthread
	LARGEFILE_FLAGS = -D_FILE_OFFSET_BITS=64 -D_LARGEFILE_SOURCE
endif

ifeq (${OS},freebsd)
	# FreeBSD version
	BINDIR = ../../bin/freebsd
	LDPC_BINDIR = ../../bin/freebsd
	LIBRARIES = -lm -pthread
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


###### Other Variables ######

# Common source files are here...
COMMON_DIR = ../common/

# List of source, object and include files
#
SRCFILES = ${COMMON_DIR}/mcl_addr.cpp ${COMMON_DIR}/mcl_socket_lib.cpp ${COMMON_DIR}/mcl_blocking_struct.cpp ${COMMON_DIR}/mcl_debug.cpp ${COMMON_DIR}/mcl_osdep.cpp ${COMMON_DIR}/rse_fec.cpp ${COMMON_DIR}/mcl_itime.cpp ${COMMON_DIR}/mcl_periodic_timer.cpp ${COMMON_DIR}/mcl_file_io.cpp mcl_cb.cpp mcl_periodic_proc.cpp mcl_vtmem.cpp mcl_vrmem.cpp mcl_rx_storage.cpp mcl_layer.cpp mcl_socket.cpp mcl_lct_hdr.cpp mcl_alc_hdr.cpp mcl_tx_tab.cpp mcl_rlc.cpp mcl_flid_sl.cpp mcl.cpp mcl_lib.cpp mcl_tx_prof.cpp mcl_sched.cpp mcl_tx.cpp mcl_rx.cpp mcl_rx_window.cpp mcl_sig.cpp mcl_network.cpp mcl_du.cpp mcl_adu.cpp mcl_stats.cpp mcl_fec.cpp mcl_fsm.cpp mcl_flute.cpp mcl_meta_object.cpp mcl_meta_object_layer.cpp
OFILES = ${SRCFILES:.cpp=.o}


# General purpose local FLAGS.
# WARNING: with gcc/g++, never use -O3 optimisation (compiler bugs)
#
CFLAGS	= -I${INCDIR} -DALC ${OSFLAGS} ${LARGEFILE_FLAGS} -DDEBUG -g	# debug version
#CFLAGS	= -DALC ${OSFLAGS} ${LARGEFILE_FLAGS} -O -g 		# optimized 
#CFLAGS	= -DALC ${OSFLAGS} ${LARGEFILE_FLAGS} -O -g -pg 	# optimized with profiling

LDFLAGS  =


###### Dependancies ######
# by default, create the MCL lib
all: lib

# Rule for C++ .cpp extension
.cpp.o:
	$(CC) -c $(CFLAGS) -o $*.o $*.cpp

lib : ${OFILES}
	@echo "** Now create the MCL-ALC library..."
	# Choose between the following two versions, according to
	# the FEC in use...
	#
	# RSE_FEC version
	${AR} -rcsv ${BINDIR}/libmclalc.a ${OFILES}
	#
	# LDPC FEC version: add the libldpc.a library
	#${AR} -rcsv ${BINDIR}/libmclalc.a ${OFILES} ${LDPC_BINDIR}/libldpc.a
	${RANLIB} ${BINDIR}/libmclalc.a
	@echo "done"

clean :
	@for f in core ${OFILES} ; do \
		if [ -f "$${f}" ] ; then \
			rm "$${f}"; \
		fi \
	done

cleanall : clean
	@for f in ${BINDIR}/libmclalc*.a ; do \
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

../common//mcl_addr.o: ../alc/mcl_includes.h ../common/mcl_types.h
../common//mcl_addr.o: ../common/mcl_osdep.h mcl_profile.h
../common//mcl_addr.o: ../common/mcl_defines.h ../common/mcl_lib_api.h
../common//mcl_addr.o: ../common/mcl_types.h ../alc/mcl_lib_api_alc.h
../common//mcl_addr.o: ../common/mcl_version.h ../common/mcl_addr.h
../common//mcl_addr.o: ../common/ssm_for_linux.h ../common/mcl_socket_lib.h
../common//mcl_addr.o: ../common/mcl_debug.h ../common/mcl_itime.h
../common//mcl_addr.o: ../common/mcl_blocking_struct.h
../common//mcl_addr.o: ../common/mcl_periodic_timer.h ../common/mcl_file_io.h
../common//mcl_addr.o: mcl.h mcl_error.h ../../include/rse_fec.h
../common//mcl_addr.o: ../../include/ldpc_fec.h
../common//mcl_addr.o: ../../include/ldpc_matrix_sparse.h
../common//mcl_addr.o: ../../include/tools.h ../../include/ldpc_create_pchk.h
../common//mcl_addr.o: mcl_lib.h mcl_data.h mcl_vrmem.h mcl_rx_storage.h
../common//mcl_addr.o: mcl_tx_tab.h mcl_rlc.h mcl_flid_sl.h mcl_socket.h
../common//mcl_addr.o: mcl_periodic_proc.h mcl_lct_hdr.h mcl_alc_hdr.h
../common//mcl_addr.o: mcl_stats.h mcl_fsm.h mcl_proto.h mcl_rx.h
../common//mcl_addr.o: mcl_rx_window.h mcl_flute.h mcl_tx.h mcl_cb.h
../common//mcl_addr.o: mcl_fec.h mcl_meta_object.h mcl_meta_object_layer.h
../common//mcl_socket_lib.o: ../alc/mcl_includes.h ../common/mcl_types.h
../common//mcl_socket_lib.o: ../common/mcl_osdep.h mcl_profile.h
../common//mcl_socket_lib.o: ../common/mcl_defines.h ../common/mcl_lib_api.h
../common//mcl_socket_lib.o: ../common/mcl_types.h ../alc/mcl_lib_api_alc.h
../common//mcl_socket_lib.o: ../common/mcl_version.h ../common/mcl_addr.h
../common//mcl_socket_lib.o: ../common/ssm_for_linux.h
../common//mcl_socket_lib.o: ../common/mcl_socket_lib.h ../common/mcl_debug.h
../common//mcl_socket_lib.o: ../common/mcl_itime.h
../common//mcl_socket_lib.o: ../common/mcl_blocking_struct.h
../common//mcl_socket_lib.o: ../common/mcl_periodic_timer.h
../common//mcl_socket_lib.o: ../common/mcl_file_io.h mcl.h mcl_error.h
../common//mcl_socket_lib.o: ../../include/rse_fec.h ../../include/ldpc_fec.h
../common//mcl_socket_lib.o: ../../include/ldpc_matrix_sparse.h
../common//mcl_socket_lib.o: ../../include/tools.h
../common//mcl_socket_lib.o: ../../include/ldpc_create_pchk.h mcl_lib.h
../common//mcl_socket_lib.o: mcl_data.h mcl_vrmem.h mcl_rx_storage.h
../common//mcl_socket_lib.o: mcl_tx_tab.h mcl_rlc.h mcl_flid_sl.h
../common//mcl_socket_lib.o: mcl_socket.h mcl_periodic_proc.h mcl_lct_hdr.h
../common//mcl_socket_lib.o: mcl_alc_hdr.h mcl_stats.h mcl_fsm.h mcl_proto.h
../common//mcl_socket_lib.o: mcl_rx.h mcl_rx_window.h mcl_flute.h mcl_tx.h
../common//mcl_socket_lib.o: mcl_cb.h mcl_fec.h mcl_meta_object.h
../common//mcl_socket_lib.o: mcl_meta_object_layer.h
../common//mcl_blocking_struct.o: ../alc/mcl_includes.h ../common/mcl_types.h
../common//mcl_blocking_struct.o: ../common/mcl_osdep.h mcl_profile.h
../common//mcl_blocking_struct.o: ../common/mcl_defines.h
../common//mcl_blocking_struct.o: ../common/mcl_lib_api.h
../common//mcl_blocking_struct.o: ../common/mcl_types.h
../common//mcl_blocking_struct.o: ../alc/mcl_lib_api_alc.h
../common//mcl_blocking_struct.o: ../common/mcl_version.h
../common//mcl_blocking_struct.o: ../common/mcl_addr.h
../common//mcl_blocking_struct.o: ../common/ssm_for_linux.h
../common//mcl_blocking_struct.o: ../common/mcl_socket_lib.h
../common//mcl_blocking_struct.o: ../common/mcl_debug.h ../common/mcl_itime.h
../common//mcl_blocking_struct.o: ../common/mcl_blocking_struct.h
../common//mcl_blocking_struct.o: ../common/mcl_periodic_timer.h
../common//mcl_blocking_struct.o: ../common/mcl_file_io.h mcl.h mcl_error.h
../common//mcl_blocking_struct.o: ../../include/rse_fec.h
../common//mcl_blocking_struct.o: ../../include/ldpc_fec.h
../common//mcl_blocking_struct.o: ../../include/ldpc_matrix_sparse.h
../common//mcl_blocking_struct.o: ../../include/tools.h
../common//mcl_blocking_struct.o: ../../include/ldpc_create_pchk.h mcl_lib.h
../common//mcl_blocking_struct.o: mcl_data.h mcl_vrmem.h mcl_rx_storage.h
../common//mcl_blocking_struct.o: mcl_tx_tab.h mcl_rlc.h mcl_flid_sl.h
../common//mcl_blocking_struct.o: mcl_socket.h mcl_periodic_proc.h
../common//mcl_blocking_struct.o: mcl_lct_hdr.h mcl_alc_hdr.h mcl_stats.h
../common//mcl_blocking_struct.o: mcl_fsm.h mcl_proto.h mcl_rx.h
../common//mcl_blocking_struct.o: mcl_rx_window.h mcl_flute.h mcl_tx.h
../common//mcl_blocking_struct.o: mcl_cb.h mcl_fec.h mcl_meta_object.h
../common//mcl_blocking_struct.o: mcl_meta_object_layer.h
../common//mcl_debug.o: ../alc/mcl_includes.h ../common/mcl_types.h
../common//mcl_debug.o: ../common/mcl_osdep.h mcl_profile.h
../common//mcl_debug.o: ../common/mcl_defines.h ../common/mcl_lib_api.h
../common//mcl_debug.o: ../common/mcl_types.h ../alc/mcl_lib_api_alc.h
../common//mcl_debug.o: ../common/mcl_version.h ../common/mcl_addr.h
../common//mcl_debug.o: ../common/ssm_for_linux.h ../common/mcl_socket_lib.h
../common//mcl_debug.o: ../common/mcl_debug.h ../common/mcl_itime.h
../common//mcl_debug.o: ../common/mcl_blocking_struct.h
../common//mcl_debug.o: ../common/mcl_periodic_timer.h
../common//mcl_debug.o: ../common/mcl_file_io.h mcl.h mcl_error.h
../common//mcl_debug.o: ../../include/rse_fec.h ../../include/ldpc_fec.h
../common//mcl_debug.o: ../../include/ldpc_matrix_sparse.h
../common//mcl_debug.o: ../../include/tools.h
../common//mcl_debug.o: ../../include/ldpc_create_pchk.h mcl_lib.h mcl_data.h
../common//mcl_debug.o: mcl_vrmem.h mcl_rx_storage.h mcl_tx_tab.h mcl_rlc.h
../common//mcl_debug.o: mcl_flid_sl.h mcl_socket.h mcl_periodic_proc.h
../common//mcl_debug.o: mcl_lct_hdr.h mcl_alc_hdr.h mcl_stats.h mcl_fsm.h
../common//mcl_debug.o: mcl_proto.h mcl_rx.h mcl_rx_window.h mcl_flute.h
../common//mcl_debug.o: mcl_tx.h mcl_cb.h mcl_fec.h mcl_meta_object.h
../common//mcl_debug.o: mcl_meta_object_layer.h
../common//mcl_osdep.o: ../alc/mcl_includes.h ../common/mcl_types.h
../common//mcl_osdep.o: ../common/mcl_osdep.h mcl_profile.h
../common//mcl_osdep.o: ../common/mcl_defines.h ../common/mcl_lib_api.h
../common//mcl_osdep.o: ../common/mcl_types.h ../alc/mcl_lib_api_alc.h
../common//mcl_osdep.o: ../common/mcl_version.h ../common/mcl_addr.h
../common//mcl_osdep.o: ../common/ssm_for_linux.h ../common/mcl_socket_lib.h
../common//mcl_osdep.o: ../common/mcl_debug.h ../common/mcl_itime.h
../common//mcl_osdep.o: ../common/mcl_blocking_struct.h
../common//mcl_osdep.o: ../common/mcl_periodic_timer.h
../common//mcl_osdep.o: ../common/mcl_file_io.h mcl.h mcl_error.h
../common//mcl_osdep.o: ../../include/rse_fec.h ../../include/ldpc_fec.h
../common//mcl_osdep.o: ../../include/ldpc_matrix_sparse.h
../common//mcl_osdep.o: ../../include/tools.h
../common//mcl_osdep.o: ../../include/ldpc_create_pchk.h mcl_lib.h mcl_data.h
../common//mcl_osdep.o: mcl_vrmem.h mcl_rx_storage.h mcl_tx_tab.h mcl_rlc.h
../common//mcl_osdep.o: mcl_flid_sl.h mcl_socket.h mcl_periodic_proc.h
../common//mcl_osdep.o: mcl_lct_hdr.h mcl_alc_hdr.h mcl_stats.h mcl_fsm.h
../common//mcl_osdep.o: mcl_proto.h mcl_rx.h mcl_rx_window.h mcl_flute.h
../common//mcl_osdep.o: mcl_tx.h mcl_cb.h mcl_fec.h mcl_meta_object.h
../common//mcl_osdep.o: mcl_meta_object_layer.h
../common//rse_fec.o: ../common/mcl_debug.h
../common//mcl_itime.o: ../alc/mcl_includes.h ../common/mcl_types.h
../common//mcl_itime.o: ../common/mcl_osdep.h mcl_profile.h
../common//mcl_itime.o: ../common/mcl_defines.h ../common/mcl_lib_api.h
../common//mcl_itime.o: ../common/mcl_types.h ../alc/mcl_lib_api_alc.h
../common//mcl_itime.o: ../common/mcl_version.h ../common/mcl_addr.h
../common//mcl_itime.o: ../common/ssm_for_linux.h ../common/mcl_socket_lib.h
../common//mcl_itime.o: ../common/mcl_debug.h ../common/mcl_itime.h
../common//mcl_itime.o: ../common/mcl_blocking_struct.h
../common//mcl_itime.o: ../common/mcl_periodic_timer.h
../common//mcl_itime.o: ../common/mcl_file_io.h mcl.h mcl_error.h
../common//mcl_itime.o: ../../include/rse_fec.h ../../include/ldpc_fec.h
../common//mcl_itime.o: ../../include/ldpc_matrix_sparse.h
../common//mcl_itime.o: ../../include/tools.h
../common//mcl_itime.o: ../../include/ldpc_create_pchk.h mcl_lib.h mcl_data.h
../common//mcl_itime.o: mcl_vrmem.h mcl_rx_storage.h mcl_tx_tab.h mcl_rlc.h
../common//mcl_itime.o: mcl_flid_sl.h mcl_socket.h mcl_periodic_proc.h
../common//mcl_itime.o: mcl_lct_hdr.h mcl_alc_hdr.h mcl_stats.h mcl_fsm.h
../common//mcl_itime.o: mcl_proto.h mcl_rx.h mcl_rx_window.h mcl_flute.h
../common//mcl_itime.o: mcl_tx.h mcl_cb.h mcl_fec.h mcl_meta_object.h
../common//mcl_itime.o: mcl_meta_object_layer.h
../common//mcl_periodic_timer.o: ../alc/mcl_includes.h ../common/mcl_types.h
../common//mcl_periodic_timer.o: ../common/mcl_osdep.h mcl_profile.h
../common//mcl_periodic_timer.o: ../common/mcl_defines.h
../common//mcl_periodic_timer.o: ../common/mcl_lib_api.h
../common//mcl_periodic_timer.o: ../common/mcl_types.h
../common//mcl_periodic_timer.o: ../alc/mcl_lib_api_alc.h
../common//mcl_periodic_timer.o: ../common/mcl_version.h ../common/mcl_addr.h
../common//mcl_periodic_timer.o: ../common/ssm_for_linux.h
../common//mcl_periodic_timer.o: ../common/mcl_socket_lib.h
../common//mcl_periodic_timer.o: ../common/mcl_debug.h ../common/mcl_itime.h
../common//mcl_periodic_timer.o: ../common/mcl_blocking_struct.h
../common//mcl_periodic_timer.o: ../common/mcl_periodic_timer.h
../common//mcl_periodic_timer.o: ../common/mcl_file_io.h mcl.h mcl_error.h
../common//mcl_periodic_timer.o: ../../include/rse_fec.h
../common//mcl_periodic_timer.o: ../../include/ldpc_fec.h
../common//mcl_periodic_timer.o: ../../include/ldpc_matrix_sparse.h
../common//mcl_periodic_timer.o: ../../include/tools.h
../common//mcl_periodic_timer.o: ../../include/ldpc_create_pchk.h mcl_lib.h
../common//mcl_periodic_timer.o: mcl_data.h mcl_vrmem.h mcl_rx_storage.h
../common//mcl_periodic_timer.o: mcl_tx_tab.h mcl_rlc.h mcl_flid_sl.h
../common//mcl_periodic_timer.o: mcl_socket.h mcl_periodic_proc.h
../common//mcl_periodic_timer.o: mcl_lct_hdr.h mcl_alc_hdr.h mcl_stats.h
../common//mcl_periodic_timer.o: mcl_fsm.h mcl_proto.h mcl_rx.h
../common//mcl_periodic_timer.o: mcl_rx_window.h mcl_flute.h mcl_tx.h
../common//mcl_periodic_timer.o: mcl_cb.h mcl_fec.h mcl_meta_object.h
../common//mcl_periodic_timer.o: mcl_meta_object_layer.h
../common//mcl_file_io.o: ../alc/mcl_includes.h ../common/mcl_types.h
../common//mcl_file_io.o: ../common/mcl_osdep.h mcl_profile.h
../common//mcl_file_io.o: ../common/mcl_defines.h ../common/mcl_lib_api.h
../common//mcl_file_io.o: ../common/mcl_types.h ../alc/mcl_lib_api_alc.h
../common//mcl_file_io.o: ../common/mcl_version.h ../common/mcl_addr.h
../common//mcl_file_io.o: ../common/ssm_for_linux.h
../common//mcl_file_io.o: ../common/mcl_socket_lib.h ../common/mcl_debug.h
../common//mcl_file_io.o: ../common/mcl_itime.h
../common//mcl_file_io.o: ../common/mcl_blocking_struct.h
../common//mcl_file_io.o: ../common/mcl_periodic_timer.h
../common//mcl_file_io.o: ../common/mcl_file_io.h mcl.h mcl_error.h
../common//mcl_file_io.o: ../../include/rse_fec.h ../../include/ldpc_fec.h
../common//mcl_file_io.o: ../../include/ldpc_matrix_sparse.h
../common//mcl_file_io.o: ../../include/tools.h
../common//mcl_file_io.o: ../../include/ldpc_create_pchk.h mcl_lib.h
../common//mcl_file_io.o: mcl_data.h mcl_vrmem.h mcl_rx_storage.h
../common//mcl_file_io.o: mcl_tx_tab.h mcl_rlc.h mcl_flid_sl.h mcl_socket.h
../common//mcl_file_io.o: mcl_periodic_proc.h mcl_lct_hdr.h mcl_alc_hdr.h
../common//mcl_file_io.o: mcl_stats.h mcl_fsm.h mcl_proto.h mcl_rx.h
../common//mcl_file_io.o: mcl_rx_window.h mcl_flute.h mcl_tx.h mcl_cb.h
../common//mcl_file_io.o: mcl_fec.h mcl_meta_object.h mcl_meta_object_layer.h
mcl_cb.o: mcl_includes.h ../common/mcl_types.h ../common/mcl_osdep.h
mcl_cb.o: mcl_profile.h ../common/mcl_defines.h ../common/mcl_lib_api.h
mcl_cb.o: ../common/mcl_types.h ../alc/mcl_lib_api_alc.h
mcl_cb.o: ../common/mcl_version.h ../common/mcl_addr.h
mcl_cb.o: ../common/ssm_for_linux.h ../common/mcl_socket_lib.h
mcl_cb.o: ../common/mcl_debug.h ../common/mcl_itime.h
mcl_cb.o: ../common/mcl_blocking_struct.h ../common/mcl_periodic_timer.h
mcl_cb.o: ../common/mcl_file_io.h mcl.h mcl_error.h ../../include/rse_fec.h
mcl_cb.o: ../../include/ldpc_fec.h ../../include/ldpc_matrix_sparse.h
mcl_cb.o: ../../include/tools.h ../../include/ldpc_create_pchk.h mcl_lib.h
mcl_cb.o: mcl_data.h mcl_vrmem.h mcl_rx_storage.h mcl_tx_tab.h mcl_rlc.h
mcl_cb.o: mcl_flid_sl.h mcl_socket.h mcl_periodic_proc.h mcl_lct_hdr.h
mcl_cb.o: mcl_alc_hdr.h mcl_stats.h mcl_fsm.h mcl_proto.h mcl_rx.h
mcl_cb.o: mcl_rx_window.h mcl_flute.h mcl_tx.h mcl_cb.h mcl_fec.h
mcl_cb.o: mcl_meta_object.h mcl_meta_object_layer.h
mcl_periodic_proc.o: mcl_includes.h ../common/mcl_types.h
mcl_periodic_proc.o: ../common/mcl_osdep.h mcl_profile.h
mcl_periodic_proc.o: ../common/mcl_defines.h ../common/mcl_lib_api.h
mcl_periodic_proc.o: ../common/mcl_types.h ../alc/mcl_lib_api_alc.h
mcl_periodic_proc.o: ../common/mcl_version.h ../common/mcl_addr.h
mcl_periodic_proc.o: ../common/ssm_for_linux.h ../common/mcl_socket_lib.h
mcl_periodic_proc.o: ../common/mcl_debug.h ../common/mcl_itime.h
mcl_periodic_proc.o: ../common/mcl_blocking_struct.h
mcl_periodic_proc.o: ../common/mcl_periodic_timer.h ../common/mcl_file_io.h
mcl_periodic_proc.o: mcl.h mcl_error.h ../../include/rse_fec.h
mcl_periodic_proc.o: ../../include/ldpc_fec.h
mcl_periodic_proc.o: ../../include/ldpc_matrix_sparse.h ../../include/tools.h
mcl_periodic_proc.o: ../../include/ldpc_create_pchk.h mcl_lib.h mcl_data.h
mcl_periodic_proc.o: mcl_vrmem.h mcl_rx_storage.h mcl_tx_tab.h mcl_rlc.h
mcl_periodic_proc.o: mcl_flid_sl.h mcl_socket.h mcl_periodic_proc.h
mcl_periodic_proc.o: mcl_lct_hdr.h mcl_alc_hdr.h mcl_stats.h mcl_fsm.h
mcl_periodic_proc.o: mcl_proto.h mcl_rx.h mcl_rx_window.h mcl_flute.h
mcl_periodic_proc.o: mcl_tx.h mcl_cb.h mcl_fec.h mcl_meta_object.h
mcl_periodic_proc.o: mcl_meta_object_layer.h
mcl_vtmem.o: mcl_includes.h ../common/mcl_types.h ../common/mcl_osdep.h
mcl_vtmem.o: mcl_profile.h ../common/mcl_defines.h ../common/mcl_lib_api.h
mcl_vtmem.o: ../common/mcl_types.h ../alc/mcl_lib_api_alc.h
mcl_vtmem.o: ../common/mcl_version.h ../common/mcl_addr.h
mcl_vtmem.o: ../common/ssm_for_linux.h ../common/mcl_socket_lib.h
mcl_vtmem.o: ../common/mcl_debug.h ../common/mcl_itime.h
mcl_vtmem.o: ../common/mcl_blocking_struct.h ../common/mcl_periodic_timer.h
mcl_vtmem.o: ../common/mcl_file_io.h mcl.h mcl_error.h
mcl_vtmem.o: ../../include/rse_fec.h ../../include/ldpc_fec.h
mcl_vtmem.o: ../../include/ldpc_matrix_sparse.h ../../include/tools.h
mcl_vtmem.o: ../../include/ldpc_create_pchk.h mcl_lib.h mcl_data.h
mcl_vtmem.o: mcl_vrmem.h mcl_rx_storage.h mcl_tx_tab.h mcl_rlc.h
mcl_vtmem.o: mcl_flid_sl.h mcl_socket.h mcl_periodic_proc.h mcl_lct_hdr.h
mcl_vtmem.o: mcl_alc_hdr.h mcl_stats.h mcl_fsm.h mcl_proto.h mcl_rx.h
mcl_vtmem.o: mcl_rx_window.h mcl_flute.h mcl_tx.h mcl_cb.h mcl_fec.h
mcl_vtmem.o: mcl_meta_object.h mcl_meta_object_layer.h
mcl_vrmem.o: mcl_includes.h ../common/mcl_types.h ../common/mcl_osdep.h
mcl_vrmem.o: mcl_profile.h ../common/mcl_defines.h ../common/mcl_lib_api.h
mcl_vrmem.o: ../common/mcl_types.h ../alc/mcl_lib_api_alc.h
mcl_vrmem.o: ../common/mcl_version.h ../common/mcl_addr.h
mcl_vrmem.o: ../common/ssm_for_linux.h ../common/mcl_socket_lib.h
mcl_vrmem.o: ../common/mcl_debug.h ../common/mcl_itime.h
mcl_vrmem.o: ../common/mcl_blocking_struct.h ../common/mcl_periodic_timer.h
mcl_vrmem.o: ../common/mcl_file_io.h mcl.h mcl_error.h
mcl_vrmem.o: ../../include/rse_fec.h ../../include/ldpc_fec.h
mcl_vrmem.o: ../../include/ldpc_matrix_sparse.h ../../include/tools.h
mcl_vrmem.o: ../../include/ldpc_create_pchk.h mcl_lib.h mcl_data.h
mcl_vrmem.o: mcl_vrmem.h mcl_rx_storage.h mcl_tx_tab.h mcl_rlc.h
mcl_vrmem.o: mcl_flid_sl.h mcl_socket.h mcl_periodic_proc.h mcl_lct_hdr.h
mcl_vrmem.o: mcl_alc_hdr.h mcl_stats.h mcl_fsm.h mcl_proto.h mcl_rx.h
mcl_vrmem.o: mcl_rx_window.h mcl_flute.h mcl_tx.h mcl_cb.h mcl_fec.h
mcl_vrmem.o: mcl_meta_object.h mcl_meta_object_layer.h
mcl_rx_storage.o: mcl_includes.h ../common/mcl_types.h ../common/mcl_osdep.h
mcl_rx_storage.o: mcl_profile.h ../common/mcl_defines.h
mcl_rx_storage.o: ../common/mcl_lib_api.h ../common/mcl_types.h
mcl_rx_storage.o: ../alc/mcl_lib_api_alc.h ../common/mcl_version.h
mcl_rx_storage.o: ../common/mcl_addr.h ../common/ssm_for_linux.h
mcl_rx_storage.o: ../common/mcl_socket_lib.h ../common/mcl_debug.h
mcl_rx_storage.o: ../common/mcl_itime.h ../common/mcl_blocking_struct.h
mcl_rx_storage.o: ../common/mcl_periodic_timer.h ../common/mcl_file_io.h
mcl_rx_storage.o: mcl.h mcl_error.h ../../include/rse_fec.h
mcl_rx_storage.o: ../../include/ldpc_fec.h ../../include/ldpc_matrix_sparse.h
mcl_rx_storage.o: ../../include/tools.h ../../include/ldpc_create_pchk.h
mcl_rx_storage.o: mcl_lib.h mcl_data.h mcl_vrmem.h mcl_rx_storage.h
mcl_rx_storage.o: mcl_tx_tab.h mcl_rlc.h mcl_flid_sl.h mcl_socket.h
mcl_rx_storage.o: mcl_periodic_proc.h mcl_lct_hdr.h mcl_alc_hdr.h mcl_stats.h
mcl_rx_storage.o: mcl_fsm.h mcl_proto.h mcl_rx.h mcl_rx_window.h mcl_flute.h
mcl_rx_storage.o: mcl_tx.h mcl_cb.h mcl_fec.h mcl_meta_object.h
mcl_rx_storage.o: mcl_meta_object_layer.h
mcl_layer.o: mcl_includes.h ../common/mcl_types.h ../common/mcl_osdep.h
mcl_layer.o: mcl_profile.h ../common/mcl_defines.h ../common/mcl_lib_api.h
mcl_layer.o: ../common/mcl_types.h ../alc/mcl_lib_api_alc.h
mcl_layer.o: ../common/mcl_version.h ../common/mcl_addr.h
mcl_layer.o: ../common/ssm_for_linux.h ../common/mcl_socket_lib.h
mcl_layer.o: ../common/mcl_debug.h ../common/mcl_itime.h
mcl_layer.o: ../common/mcl_blocking_struct.h ../common/mcl_periodic_timer.h
mcl_layer.o: ../common/mcl_file_io.h mcl.h mcl_error.h
mcl_layer.o: ../../include/rse_fec.h ../../include/ldpc_fec.h
mcl_layer.o: ../../include/ldpc_matrix_sparse.h ../../include/tools.h
mcl_layer.o: ../../include/ldpc_create_pchk.h mcl_lib.h mcl_data.h
mcl_layer.o: mcl_vrmem.h mcl_rx_storage.h mcl_tx_tab.h mcl_rlc.h
mcl_layer.o: mcl_flid_sl.h mcl_socket.h mcl_periodic_proc.h mcl_lct_hdr.h
mcl_layer.o: mcl_alc_hdr.h mcl_stats.h mcl_fsm.h mcl_proto.h mcl_rx.h
mcl_layer.o: mcl_rx_window.h mcl_flute.h mcl_tx.h mcl_cb.h mcl_fec.h
mcl_layer.o: mcl_meta_object.h mcl_meta_object_layer.h
mcl_socket.o: mcl_includes.h ../common/mcl_types.h ../common/mcl_osdep.h
mcl_socket.o: mcl_profile.h ../common/mcl_defines.h ../common/mcl_lib_api.h
mcl_socket.o: ../common/mcl_types.h ../alc/mcl_lib_api_alc.h
mcl_socket.o: ../common/mcl_version.h ../common/mcl_addr.h
mcl_socket.o: ../common/ssm_for_linux.h ../common/mcl_socket_lib.h
mcl_socket.o: ../common/mcl_debug.h ../common/mcl_itime.h
mcl_socket.o: ../common/mcl_blocking_struct.h ../common/mcl_periodic_timer.h
mcl_socket.o: ../common/mcl_file_io.h mcl.h mcl_error.h
mcl_socket.o: ../../include/rse_fec.h ../../include/ldpc_fec.h
mcl_socket.o: ../../include/ldpc_matrix_sparse.h ../../include/tools.h
mcl_socket.o: ../../include/ldpc_create_pchk.h mcl_lib.h mcl_data.h
mcl_socket.o: mcl_vrmem.h mcl_rx_storage.h mcl_tx_tab.h mcl_rlc.h
mcl_socket.o: mcl_flid_sl.h mcl_socket.h mcl_periodic_proc.h mcl_lct_hdr.h
mcl_socket.o: mcl_alc_hdr.h mcl_stats.h mcl_fsm.h mcl_proto.h mcl_rx.h
mcl_socket.o: mcl_rx_window.h mcl_flute.h mcl_tx.h mcl_cb.h mcl_fec.h
mcl_socket.o: mcl_meta_object.h mcl_meta_object_layer.h
mcl_lct_hdr.o: mcl_includes.h ../common/mcl_types.h ../common/mcl_osdep.h
mcl_lct_hdr.o: mcl_profile.h ../common/mcl_defines.h ../common/mcl_lib_api.h
mcl_lct_hdr.o: ../common/mcl_types.h ../alc/mcl_lib_api_alc.h
mcl_lct_hdr.o: ../common/mcl_version.h ../common/mcl_addr.h
mcl_lct_hdr.o: ../common/ssm_for_linux.h ../common/mcl_socket_lib.h
mcl_lct_hdr.o: ../common/mcl_debug.h ../common/mcl_itime.h
mcl_lct_hdr.o: ../common/mcl_blocking_struct.h ../common/mcl_periodic_timer.h
mcl_lct_hdr.o: ../common/mcl_file_io.h mcl.h mcl_error.h
mcl_lct_hdr.o: ../../include/rse_fec.h ../../include/ldpc_fec.h
mcl_lct_hdr.o: ../../include/ldpc_matrix_sparse.h ../../include/tools.h
mcl_lct_hdr.o: ../../include/ldpc_create_pchk.h mcl_lib.h mcl_data.h
mcl_lct_hdr.o: mcl_vrmem.h mcl_rx_storage.h mcl_tx_tab.h mcl_rlc.h
mcl_lct_hdr.o: mcl_flid_sl.h mcl_socket.h mcl_periodic_proc.h mcl_lct_hdr.h
mcl_lct_hdr.o: mcl_alc_hdr.h mcl_stats.h mcl_fsm.h mcl_proto.h mcl_rx.h
mcl_lct_hdr.o: mcl_rx_window.h mcl_flute.h mcl_tx.h mcl_cb.h mcl_fec.h
mcl_lct_hdr.o: mcl_meta_object.h mcl_meta_object_layer.h
mcl_alc_hdr.o: mcl_includes.h ../common/mcl_types.h ../common/mcl_osdep.h
mcl_alc_hdr.o: mcl_profile.h ../common/mcl_defines.h ../common/mcl_lib_api.h
mcl_alc_hdr.o: ../common/mcl_types.h ../alc/mcl_lib_api_alc.h
mcl_alc_hdr.o: ../common/mcl_version.h ../common/mcl_addr.h
mcl_alc_hdr.o: ../common/ssm_for_linux.h ../common/mcl_socket_lib.h
mcl_alc_hdr.o: ../common/mcl_debug.h ../common/mcl_itime.h
mcl_alc_hdr.o: ../common/mcl_blocking_struct.h ../common/mcl_periodic_timer.h
mcl_alc_hdr.o: ../common/mcl_file_io.h mcl.h mcl_error.h
mcl_alc_hdr.o: ../../include/rse_fec.h ../../include/ldpc_fec.h
mcl_alc_hdr.o: ../../include/ldpc_matrix_sparse.h ../../include/tools.h
mcl_alc_hdr.o: ../../include/ldpc_create_pchk.h mcl_lib.h mcl_data.h
mcl_alc_hdr.o: mcl_vrmem.h mcl_rx_storage.h mcl_tx_tab.h mcl_rlc.h
mcl_alc_hdr.o: mcl_flid_sl.h mcl_socket.h mcl_periodic_proc.h mcl_lct_hdr.h
mcl_alc_hdr.o: mcl_alc_hdr.h mcl_stats.h mcl_fsm.h mcl_proto.h mcl_rx.h
mcl_alc_hdr.o: mcl_rx_window.h mcl_flute.h mcl_tx.h mcl_cb.h mcl_fec.h
mcl_alc_hdr.o: mcl_meta_object.h mcl_meta_object_layer.h
mcl_tx_tab.o: mcl_includes.h ../common/mcl_types.h ../common/mcl_osdep.h
mcl_tx_tab.o: mcl_profile.h ../common/mcl_defines.h ../common/mcl_lib_api.h
mcl_tx_tab.o: ../common/mcl_types.h ../alc/mcl_lib_api_alc.h
mcl_tx_tab.o: ../common/mcl_version.h ../common/mcl_addr.h
mcl_tx_tab.o: ../common/ssm_for_linux.h ../common/mcl_socket_lib.h
mcl_tx_tab.o: ../common/mcl_debug.h ../common/mcl_itime.h
mcl_tx_tab.o: ../common/mcl_blocking_struct.h ../common/mcl_periodic_timer.h
mcl_tx_tab.o: ../common/mcl_file_io.h mcl.h mcl_error.h
mcl_tx_tab.o: ../../include/rse_fec.h ../../include/ldpc_fec.h
mcl_tx_tab.o: ../../include/ldpc_matrix_sparse.h ../../include/tools.h
mcl_tx_tab.o: ../../include/ldpc_create_pchk.h mcl_lib.h mcl_data.h
mcl_tx_tab.o: mcl_vrmem.h mcl_rx_storage.h mcl_tx_tab.h mcl_rlc.h
mcl_tx_tab.o: mcl_flid_sl.h mcl_socket.h mcl_periodic_proc.h mcl_lct_hdr.h
mcl_tx_tab.o: mcl_alc_hdr.h mcl_stats.h mcl_fsm.h mcl_proto.h mcl_rx.h
mcl_tx_tab.o: mcl_rx_window.h mcl_flute.h mcl_tx.h mcl_cb.h mcl_fec.h
mcl_tx_tab.o: mcl_meta_object.h mcl_meta_object_layer.h
mcl_rlc.o: mcl_includes.h ../common/mcl_types.h ../common/mcl_osdep.h
mcl_rlc.o: mcl_profile.h ../common/mcl_defines.h ../common/mcl_lib_api.h
mcl_rlc.o: ../common/mcl_types.h ../alc/mcl_lib_api_alc.h
mcl_rlc.o: ../common/mcl_version.h ../common/mcl_addr.h
mcl_rlc.o: ../common/ssm_for_linux.h ../common/mcl_socket_lib.h
mcl_rlc.o: ../common/mcl_debug.h ../common/mcl_itime.h
mcl_rlc.o: ../common/mcl_blocking_struct.h ../common/mcl_periodic_timer.h
mcl_rlc.o: ../common/mcl_file_io.h mcl.h mcl_error.h ../../include/rse_fec.h
mcl_rlc.o: ../../include/ldpc_fec.h ../../include/ldpc_matrix_sparse.h
mcl_rlc.o: ../../include/tools.h ../../include/ldpc_create_pchk.h mcl_lib.h
mcl_rlc.o: mcl_data.h mcl_vrmem.h mcl_rx_storage.h mcl_tx_tab.h mcl_rlc.h
mcl_rlc.o: mcl_flid_sl.h mcl_socket.h mcl_periodic_proc.h mcl_lct_hdr.h
mcl_rlc.o: mcl_alc_hdr.h mcl_stats.h mcl_fsm.h mcl_proto.h mcl_rx.h
mcl_rlc.o: mcl_rx_window.h mcl_flute.h mcl_tx.h mcl_cb.h mcl_fec.h
mcl_rlc.o: mcl_meta_object.h mcl_meta_object_layer.h
mcl_flid_sl.o: mcl_includes.h ../common/mcl_types.h ../common/mcl_osdep.h
mcl_flid_sl.o: mcl_profile.h ../common/mcl_defines.h ../common/mcl_lib_api.h
mcl_flid_sl.o: ../common/mcl_types.h ../alc/mcl_lib_api_alc.h
mcl_flid_sl.o: ../common/mcl_version.h ../common/mcl_addr.h
mcl_flid_sl.o: ../common/ssm_for_linux.h ../common/mcl_socket_lib.h
mcl_flid_sl.o: ../common/mcl_debug.h ../common/mcl_itime.h
mcl_flid_sl.o: ../common/mcl_blocking_struct.h ../common/mcl_periodic_timer.h
mcl_flid_sl.o: ../common/mcl_file_io.h mcl.h mcl_error.h
mcl_flid_sl.o: ../../include/rse_fec.h ../../include/ldpc_fec.h
mcl_flid_sl.o: ../../include/ldpc_matrix_sparse.h ../../include/tools.h
mcl_flid_sl.o: ../../include/ldpc_create_pchk.h mcl_lib.h mcl_data.h
mcl_flid_sl.o: mcl_vrmem.h mcl_rx_storage.h mcl_tx_tab.h mcl_rlc.h
mcl_flid_sl.o: mcl_flid_sl.h mcl_socket.h mcl_periodic_proc.h mcl_lct_hdr.h
mcl_flid_sl.o: mcl_alc_hdr.h mcl_stats.h mcl_fsm.h mcl_proto.h mcl_rx.h
mcl_flid_sl.o: mcl_rx_window.h mcl_flute.h mcl_tx.h mcl_cb.h mcl_fec.h
mcl_flid_sl.o: mcl_meta_object.h mcl_meta_object_layer.h
mcl.o: mcl_includes.h ../common/mcl_types.h ../common/mcl_osdep.h
mcl.o: mcl_profile.h ../common/mcl_defines.h ../common/mcl_lib_api.h
mcl.o: ../common/mcl_types.h ../alc/mcl_lib_api_alc.h ../common/mcl_version.h
mcl.o: ../common/mcl_addr.h ../common/ssm_for_linux.h
mcl.o: ../common/mcl_socket_lib.h ../common/mcl_debug.h ../common/mcl_itime.h
mcl.o: ../common/mcl_blocking_struct.h ../common/mcl_periodic_timer.h
mcl.o: ../common/mcl_file_io.h mcl.h mcl_error.h ../../include/rse_fec.h
mcl.o: ../../include/ldpc_fec.h ../../include/ldpc_matrix_sparse.h
mcl.o: ../../include/tools.h ../../include/ldpc_create_pchk.h mcl_lib.h
mcl.o: mcl_data.h mcl_vrmem.h mcl_rx_storage.h mcl_tx_tab.h mcl_rlc.h
mcl.o: mcl_flid_sl.h mcl_socket.h mcl_periodic_proc.h mcl_lct_hdr.h
mcl.o: mcl_alc_hdr.h mcl_stats.h mcl_fsm.h mcl_proto.h mcl_rx.h
mcl.o: mcl_rx_window.h mcl_flute.h mcl_tx.h mcl_cb.h mcl_fec.h
mcl.o: mcl_meta_object.h mcl_meta_object_layer.h
mcl_lib.o: mcl_includes.h ../common/mcl_types.h ../common/mcl_osdep.h
mcl_lib.o: mcl_profile.h ../common/mcl_defines.h ../common/mcl_lib_api.h
mcl_lib.o: ../common/mcl_types.h ../alc/mcl_lib_api_alc.h
mcl_lib.o: ../common/mcl_version.h ../common/mcl_addr.h
mcl_lib.o: ../common/ssm_for_linux.h ../common/mcl_socket_lib.h
mcl_lib.o: ../common/mcl_debug.h ../common/mcl_itime.h
mcl_lib.o: ../common/mcl_blocking_struct.h ../common/mcl_periodic_timer.h
mcl_lib.o: ../common/mcl_file_io.h mcl.h mcl_error.h ../../include/rse_fec.h
mcl_lib.o: ../../include/ldpc_fec.h ../../include/ldpc_matrix_sparse.h
mcl_lib.o: ../../include/tools.h ../../include/ldpc_create_pchk.h mcl_lib.h
mcl_lib.o: mcl_data.h mcl_vrmem.h mcl_rx_storage.h mcl_tx_tab.h mcl_rlc.h
mcl_lib.o: mcl_flid_sl.h mcl_socket.h mcl_periodic_proc.h mcl_lct_hdr.h
mcl_lib.o: mcl_alc_hdr.h mcl_stats.h mcl_fsm.h mcl_proto.h mcl_rx.h
mcl_lib.o: mcl_rx_window.h mcl_flute.h mcl_tx.h mcl_cb.h mcl_fec.h
mcl_lib.o: mcl_meta_object.h mcl_meta_object_layer.h
mcl_tx_prof.o: mcl_includes.h ../common/mcl_types.h ../common/mcl_osdep.h
mcl_tx_prof.o: mcl_profile.h ../common/mcl_defines.h ../common/mcl_lib_api.h
mcl_tx_prof.o: ../common/mcl_types.h ../alc/mcl_lib_api_alc.h
mcl_tx_prof.o: ../common/mcl_version.h ../common/mcl_addr.h
mcl_tx_prof.o: ../common/ssm_for_linux.h ../common/mcl_socket_lib.h
mcl_tx_prof.o: ../common/mcl_debug.h ../common/mcl_itime.h
mcl_tx_prof.o: ../common/mcl_blocking_struct.h ../common/mcl_periodic_timer.h
mcl_tx_prof.o: ../common/mcl_file_io.h mcl.h mcl_error.h
mcl_tx_prof.o: ../../include/rse_fec.h ../../include/ldpc_fec.h
mcl_tx_prof.o: ../../include/ldpc_matrix_sparse.h ../../include/tools.h
mcl_tx_prof.o: ../../include/ldpc_create_pchk.h mcl_lib.h mcl_data.h
mcl_tx_prof.o: mcl_vrmem.h mcl_rx_storage.h mcl_tx_tab.h mcl_rlc.h
mcl_tx_prof.o: mcl_flid_sl.h mcl_socket.h mcl_periodic_proc.h mcl_lct_hdr.h
mcl_tx_prof.o: mcl_alc_hdr.h mcl_stats.h mcl_fsm.h mcl_proto.h mcl_rx.h
mcl_tx_prof.o: mcl_rx_window.h mcl_flute.h mcl_tx.h mcl_cb.h mcl_fec.h
mcl_tx_prof.o: mcl_meta_object.h mcl_meta_object_layer.h
mcl_sched.o: mcl_includes.h ../common/mcl_types.h ../common/mcl_osdep.h
mcl_sched.o: mcl_profile.h ../common/mcl_defines.h ../common/mcl_lib_api.h
mcl_sched.o: ../common/mcl_types.h ../alc/mcl_lib_api_alc.h
mcl_sched.o: ../common/mcl_version.h ../common/mcl_addr.h
mcl_sched.o: ../common/ssm_for_linux.h ../common/mcl_socket_lib.h
mcl_sched.o: ../common/mcl_debug.h ../common/mcl_itime.h
mcl_sched.o: ../common/mcl_blocking_struct.h ../common/mcl_periodic_timer.h
mcl_sched.o: ../common/mcl_file_io.h mcl.h mcl_error.h
mcl_sched.o: ../../include/rse_fec.h ../../include/ldpc_fec.h
mcl_sched.o: ../../include/ldpc_matrix_sparse.h ../../include/tools.h
mcl_sched.o: ../../include/ldpc_create_pchk.h mcl_lib.h mcl_data.h
mcl_sched.o: mcl_vrmem.h mcl_rx_storage.h mcl_tx_tab.h mcl_rlc.h
mcl_sched.o: mcl_flid_sl.h mcl_socket.h mcl_periodic_proc.h mcl_lct_hdr.h
mcl_sched.o: mcl_alc_hdr.h mcl_stats.h mcl_fsm.h mcl_proto.h mcl_rx.h
mcl_sched.o: mcl_rx_window.h mcl_flute.h mcl_tx.h mcl_cb.h mcl_fec.h
mcl_sched.o: mcl_meta_object.h mcl_meta_object_layer.h
mcl_tx.o: mcl_includes.h ../common/mcl_types.h ../common/mcl_osdep.h
mcl_tx.o: mcl_profile.h ../common/mcl_defines.h ../common/mcl_lib_api.h
mcl_tx.o: ../common/mcl_types.h ../alc/mcl_lib_api_alc.h
mcl_tx.o: ../common/mcl_version.h ../common/mcl_addr.h
mcl_tx.o: ../common/ssm_for_linux.h ../common/mcl_socket_lib.h
mcl_tx.o: ../common/mcl_debug.h ../common/mcl_itime.h
mcl_tx.o: ../common/mcl_blocking_struct.h ../common/mcl_periodic_timer.h
mcl_tx.o: ../common/mcl_file_io.h mcl.h mcl_error.h ../../include/rse_fec.h
mcl_tx.o: ../../include/ldpc_fec.h ../../include/ldpc_matrix_sparse.h
mcl_tx.o: ../../include/tools.h ../../include/ldpc_create_pchk.h mcl_lib.h
mcl_tx.o: mcl_data.h mcl_vrmem.h mcl_rx_storage.h mcl_tx_tab.h mcl_rlc.h
mcl_tx.o: mcl_flid_sl.h mcl_socket.h mcl_periodic_proc.h mcl_lct_hdr.h
mcl_tx.o: mcl_alc_hdr.h mcl_stats.h mcl_fsm.h mcl_proto.h mcl_rx.h
mcl_tx.o: mcl_rx_window.h mcl_flute.h mcl_tx.h mcl_cb.h mcl_fec.h
mcl_tx.o: mcl_meta_object.h mcl_meta_object_layer.h
mcl_rx.o: mcl_includes.h ../common/mcl_types.h ../common/mcl_osdep.h
mcl_rx.o: mcl_profile.h ../common/mcl_defines.h ../common/mcl_lib_api.h
mcl_rx.o: ../common/mcl_types.h ../alc/mcl_lib_api_alc.h
mcl_rx.o: ../common/mcl_version.h ../common/mcl_addr.h
mcl_rx.o: ../common/ssm_for_linux.h ../common/mcl_socket_lib.h
mcl_rx.o: ../common/mcl_debug.h ../common/mcl_itime.h
mcl_rx.o: ../common/mcl_blocking_struct.h ../common/mcl_periodic_timer.h
mcl_rx.o: ../common/mcl_file_io.h mcl.h mcl_error.h ../../include/rse_fec.h
mcl_rx.o: ../../include/ldpc_fec.h ../../include/ldpc_matrix_sparse.h
mcl_rx.o: ../../include/tools.h ../../include/ldpc_create_pchk.h mcl_lib.h
mcl_rx.o: mcl_data.h mcl_vrmem.h mcl_rx_storage.h mcl_tx_tab.h mcl_rlc.h
mcl_rx.o: mcl_flid_sl.h mcl_socket.h mcl_periodic_proc.h mcl_lct_hdr.h
mcl_rx.o: mcl_alc_hdr.h mcl_stats.h mcl_fsm.h mcl_proto.h mcl_rx.h
mcl_rx.o: mcl_rx_window.h mcl_flute.h mcl_tx.h mcl_cb.h mcl_fec.h
mcl_rx.o: mcl_meta_object.h mcl_meta_object_layer.h
mcl_rx_window.o: mcl_includes.h ../common/mcl_types.h ../common/mcl_osdep.h
mcl_rx_window.o: mcl_profile.h ../common/mcl_defines.h
mcl_rx_window.o: ../common/mcl_lib_api.h ../common/mcl_types.h
mcl_rx_window.o: ../alc/mcl_lib_api_alc.h ../common/mcl_version.h
mcl_rx_window.o: ../common/mcl_addr.h ../common/ssm_for_linux.h
mcl_rx_window.o: ../common/mcl_socket_lib.h ../common/mcl_debug.h
mcl_rx_window.o: ../common/mcl_itime.h ../common/mcl_blocking_struct.h
mcl_rx_window.o: ../common/mcl_periodic_timer.h ../common/mcl_file_io.h mcl.h
mcl_rx_window.o: mcl_error.h ../../include/rse_fec.h ../../include/ldpc_fec.h
mcl_rx_window.o: ../../include/ldpc_matrix_sparse.h ../../include/tools.h
mcl_rx_window.o: ../../include/ldpc_create_pchk.h mcl_lib.h mcl_data.h
mcl_rx_window.o: mcl_vrmem.h mcl_rx_storage.h mcl_tx_tab.h mcl_rlc.h
mcl_rx_window.o: mcl_flid_sl.h mcl_socket.h mcl_periodic_proc.h mcl_lct_hdr.h
mcl_rx_window.o: mcl_alc_hdr.h mcl_stats.h mcl_fsm.h mcl_proto.h mcl_rx.h
mcl_rx_window.o: mcl_rx_window.h mcl_flute.h mcl_tx.h mcl_cb.h mcl_fec.h
mcl_rx_window.o: mcl_meta_object.h mcl_meta_object_layer.h
mcl_sig.o: mcl_includes.h ../common/mcl_types.h ../common/mcl_osdep.h
mcl_sig.o: mcl_profile.h ../common/mcl_defines.h ../common/mcl_lib_api.h
mcl_sig.o: ../common/mcl_types.h ../alc/mcl_lib_api_alc.h
mcl_sig.o: ../common/mcl_version.h ../common/mcl_addr.h
mcl_sig.o: ../common/ssm_for_linux.h ../common/mcl_socket_lib.h
mcl_sig.o: ../common/mcl_debug.h ../common/mcl_itime.h
mcl_sig.o: ../common/mcl_blocking_struct.h ../common/mcl_periodic_timer.h
mcl_sig.o: ../common/mcl_file_io.h mcl.h mcl_error.h ../../include/rse_fec.h
mcl_sig.o: ../../include/ldpc_fec.h ../../include/ldpc_matrix_sparse.h
mcl_sig.o: ../../include/tools.h ../../include/ldpc_create_pchk.h mcl_lib.h
mcl_sig.o: mcl_data.h mcl_vrmem.h mcl_rx_storage.h mcl_tx_tab.h mcl_rlc.h
mcl_sig.o: mcl_flid_sl.h mcl_socket.h mcl_periodic_proc.h mcl_lct_hdr.h
mcl_sig.o: mcl_alc_hdr.h mcl_stats.h mcl_fsm.h mcl_proto.h mcl_rx.h
mcl_sig.o: mcl_rx_window.h mcl_flute.h mcl_tx.h mcl_cb.h mcl_fec.h
mcl_sig.o: mcl_meta_object.h mcl_meta_object_layer.h
mcl_network.o: mcl_includes.h ../common/mcl_types.h ../common/mcl_osdep.h
mcl_network.o: mcl_profile.h ../common/mcl_defines.h ../common/mcl_lib_api.h
mcl_network.o: ../common/mcl_types.h ../alc/mcl_lib_api_alc.h
mcl_network.o: ../common/mcl_version.h ../common/mcl_addr.h
mcl_network.o: ../common/ssm_for_linux.h ../common/mcl_socket_lib.h
mcl_network.o: ../common/mcl_debug.h ../common/mcl_itime.h
mcl_network.o: ../common/mcl_blocking_struct.h ../common/mcl_periodic_timer.h
mcl_network.o: ../common/mcl_file_io.h mcl.h mcl_error.h
mcl_network.o: ../../include/rse_fec.h ../../include/ldpc_fec.h
mcl_network.o: ../../include/ldpc_matrix_sparse.h ../../include/tools.h
mcl_network.o: ../../include/ldpc_create_pchk.h mcl_lib.h mcl_data.h
mcl_network.o: mcl_vrmem.h mcl_rx_storage.h mcl_tx_tab.h mcl_rlc.h
mcl_network.o: mcl_flid_sl.h mcl_socket.h mcl_periodic_proc.h mcl_lct_hdr.h
mcl_network.o: mcl_alc_hdr.h mcl_stats.h mcl_fsm.h mcl_proto.h mcl_rx.h
mcl_network.o: mcl_rx_window.h mcl_flute.h mcl_tx.h mcl_cb.h mcl_fec.h
mcl_network.o: mcl_meta_object.h mcl_meta_object_layer.h
mcl_du.o: mcl_includes.h ../common/mcl_types.h ../common/mcl_osdep.h
mcl_du.o: mcl_profile.h ../common/mcl_defines.h ../common/mcl_lib_api.h
mcl_du.o: ../common/mcl_types.h ../alc/mcl_lib_api_alc.h
mcl_du.o: ../common/mcl_version.h ../common/mcl_addr.h
mcl_du.o: ../common/ssm_for_linux.h ../common/mcl_socket_lib.h
mcl_du.o: ../common/mcl_debug.h ../common/mcl_itime.h
mcl_du.o: ../common/mcl_blocking_struct.h ../common/mcl_periodic_timer.h
mcl_du.o: ../common/mcl_file_io.h mcl.h mcl_error.h ../../include/rse_fec.h
mcl_du.o: ../../include/ldpc_fec.h ../../include/ldpc_matrix_sparse.h
mcl_du.o: ../../include/tools.h ../../include/ldpc_create_pchk.h mcl_lib.h
mcl_du.o: mcl_data.h mcl_vrmem.h mcl_rx_storage.h mcl_tx_tab.h mcl_rlc.h
mcl_du.o: mcl_flid_sl.h mcl_socket.h mcl_periodic_proc.h mcl_lct_hdr.h
mcl_du.o: mcl_alc_hdr.h mcl_stats.h mcl_fsm.h mcl_proto.h mcl_rx.h
mcl_du.o: mcl_rx_window.h mcl_flute.h mcl_tx.h mcl_cb.h mcl_fec.h
mcl_du.o: mcl_meta_object.h mcl_meta_object_layer.h
mcl_adu.o: mcl_includes.h ../common/mcl_types.h ../common/mcl_osdep.h
mcl_adu.o: mcl_profile.h ../common/mcl_defines.h ../common/mcl_lib_api.h
mcl_adu.o: ../common/mcl_types.h ../alc/mcl_lib_api_alc.h
mcl_adu.o: ../common/mcl_version.h ../common/mcl_addr.h
mcl_adu.o: ../common/ssm_for_linux.h ../common/mcl_socket_lib.h
mcl_adu.o: ../common/mcl_debug.h ../common/mcl_itime.h
mcl_adu.o: ../common/mcl_blocking_struct.h ../common/mcl_periodic_timer.h
mcl_adu.o: ../common/mcl_file_io.h mcl.h mcl_error.h ../../include/rse_fec.h
mcl_adu.o: ../../include/ldpc_fec.h ../../include/ldpc_matrix_sparse.h
mcl_adu.o: ../../include/tools.h ../../include/ldpc_create_pchk.h mcl_lib.h
mcl_adu.o: mcl_data.h mcl_vrmem.h mcl_rx_storage.h mcl_tx_tab.h mcl_rlc.h
mcl_adu.o: mcl_flid_sl.h mcl_socket.h mcl_periodic_proc.h mcl_lct_hdr.h
mcl_adu.o: mcl_alc_hdr.h mcl_stats.h mcl_fsm.h mcl_proto.h mcl_rx.h
mcl_adu.o: mcl_rx_window.h mcl_flute.h mcl_tx.h mcl_cb.h mcl_fec.h
mcl_adu.o: mcl_meta_object.h mcl_meta_object_layer.h
mcl_stats.o: mcl_includes.h ../common/mcl_types.h ../common/mcl_osdep.h
mcl_stats.o: mcl_profile.h ../common/mcl_defines.h ../common/mcl_lib_api.h
mcl_stats.o: ../common/mcl_types.h ../alc/mcl_lib_api_alc.h
mcl_stats.o: ../common/mcl_version.h ../common/mcl_addr.h
mcl_stats.o: ../common/ssm_for_linux.h ../common/mcl_socket_lib.h
mcl_stats.o: ../common/mcl_debug.h ../common/mcl_itime.h
mcl_stats.o: ../common/mcl_blocking_struct.h ../common/mcl_periodic_timer.h
mcl_stats.o: ../common/mcl_file_io.h mcl.h mcl_error.h
mcl_stats.o: ../../include/rse_fec.h ../../include/ldpc_fec.h
mcl_stats.o: ../../include/ldpc_matrix_sparse.h ../../include/tools.h
mcl_stats.o: ../../include/ldpc_create_pchk.h mcl_lib.h mcl_data.h
mcl_stats.o: mcl_vrmem.h mcl_rx_storage.h mcl_tx_tab.h mcl_rlc.h
mcl_stats.o: mcl_flid_sl.h mcl_socket.h mcl_periodic_proc.h mcl_lct_hdr.h
mcl_stats.o: mcl_alc_hdr.h mcl_stats.h mcl_fsm.h mcl_proto.h mcl_rx.h
mcl_stats.o: mcl_rx_window.h mcl_flute.h mcl_tx.h mcl_cb.h mcl_fec.h
mcl_stats.o: mcl_meta_object.h mcl_meta_object_layer.h
mcl_fec.o: mcl_includes.h ../common/mcl_types.h ../common/mcl_osdep.h
mcl_fec.o: mcl_profile.h ../common/mcl_defines.h ../common/mcl_lib_api.h
mcl_fec.o: ../common/mcl_types.h ../alc/mcl_lib_api_alc.h
mcl_fec.o: ../common/mcl_version.h ../common/mcl_addr.h
mcl_fec.o: ../common/ssm_for_linux.h ../common/mcl_socket_lib.h
mcl_fec.o: ../common/mcl_debug.h ../common/mcl_itime.h
mcl_fec.o: ../common/mcl_blocking_struct.h ../common/mcl_periodic_timer.h
mcl_fec.o: ../common/mcl_file_io.h mcl.h mcl_error.h ../../include/rse_fec.h
mcl_fec.o: ../../include/ldpc_fec.h ../../include/ldpc_matrix_sparse.h
mcl_fec.o: ../../include/tools.h ../../include/ldpc_create_pchk.h mcl_lib.h
mcl_fec.o: mcl_data.h mcl_vrmem.h mcl_rx_storage.h mcl_tx_tab.h mcl_rlc.h
mcl_fec.o: mcl_flid_sl.h mcl_socket.h mcl_periodic_proc.h mcl_lct_hdr.h
mcl_fec.o: mcl_alc_hdr.h mcl_stats.h mcl_fsm.h mcl_proto.h mcl_rx.h
mcl_fec.o: mcl_rx_window.h mcl_flute.h mcl_tx.h mcl_cb.h mcl_fec.h
mcl_fec.o: mcl_meta_object.h mcl_meta_object_layer.h
mcl_fsm.o: mcl_includes.h ../common/mcl_types.h ../common/mcl_osdep.h
mcl_fsm.o: mcl_profile.h ../common/mcl_defines.h ../common/mcl_lib_api.h
mcl_fsm.o: ../common/mcl_types.h ../alc/mcl_lib_api_alc.h
mcl_fsm.o: ../common/mcl_version.h ../common/mcl_addr.h
mcl_fsm.o: ../common/ssm_for_linux.h ../common/mcl_socket_lib.h
mcl_fsm.o: ../common/mcl_debug.h ../common/mcl_itime.h
mcl_fsm.o: ../common/mcl_blocking_struct.h ../common/mcl_periodic_timer.h
mcl_fsm.o: ../common/mcl_file_io.h mcl.h mcl_error.h ../../include/rse_fec.h
mcl_fsm.o: ../../include/ldpc_fec.h ../../include/ldpc_matrix_sparse.h
mcl_fsm.o: ../../include/tools.h ../../include/ldpc_create_pchk.h mcl_lib.h
mcl_fsm.o: mcl_data.h mcl_vrmem.h mcl_rx_storage.h mcl_tx_tab.h mcl_rlc.h
mcl_fsm.o: mcl_flid_sl.h mcl_socket.h mcl_periodic_proc.h mcl_lct_hdr.h
mcl_fsm.o: mcl_alc_hdr.h mcl_stats.h mcl_fsm.h mcl_proto.h mcl_rx.h
mcl_fsm.o: mcl_rx_window.h mcl_flute.h mcl_tx.h mcl_cb.h mcl_fec.h
mcl_fsm.o: mcl_meta_object.h mcl_meta_object_layer.h
mcl_flute.o: mcl_includes.h ../common/mcl_types.h ../common/mcl_osdep.h
mcl_flute.o: mcl_profile.h ../common/mcl_defines.h ../common/mcl_lib_api.h
mcl_flute.o: ../common/mcl_types.h ../alc/mcl_lib_api_alc.h
mcl_flute.o: ../common/mcl_version.h ../common/mcl_addr.h
mcl_flute.o: ../common/ssm_for_linux.h ../common/mcl_socket_lib.h
mcl_flute.o: ../common/mcl_debug.h ../common/mcl_itime.h
mcl_flute.o: ../common/mcl_blocking_struct.h ../common/mcl_periodic_timer.h
mcl_flute.o: ../common/mcl_file_io.h mcl.h mcl_error.h
mcl_flute.o: ../../include/rse_fec.h ../../include/ldpc_fec.h
mcl_flute.o: ../../include/ldpc_matrix_sparse.h ../../include/tools.h
mcl_flute.o: ../../include/ldpc_create_pchk.h mcl_lib.h mcl_data.h
mcl_flute.o: mcl_vrmem.h mcl_rx_storage.h mcl_tx_tab.h mcl_rlc.h
mcl_flute.o: mcl_flid_sl.h mcl_socket.h mcl_periodic_proc.h mcl_lct_hdr.h
mcl_flute.o: mcl_alc_hdr.h mcl_stats.h mcl_fsm.h mcl_proto.h mcl_rx.h
mcl_flute.o: mcl_rx_window.h mcl_flute.h mcl_tx.h mcl_cb.h mcl_fec.h
mcl_flute.o: mcl_meta_object.h mcl_meta_object_layer.h
mcl_meta_object.o: mcl_includes.h ../common/mcl_types.h ../common/mcl_osdep.h
mcl_meta_object.o: mcl_profile.h ../common/mcl_defines.h
mcl_meta_object.o: ../common/mcl_lib_api.h ../common/mcl_types.h
mcl_meta_object.o: ../alc/mcl_lib_api_alc.h ../common/mcl_version.h
mcl_meta_object.o: ../common/mcl_addr.h ../common/ssm_for_linux.h
mcl_meta_object.o: ../common/mcl_socket_lib.h ../common/mcl_debug.h
mcl_meta_object.o: ../common/mcl_itime.h ../common/mcl_blocking_struct.h
mcl_meta_object.o: ../common/mcl_periodic_timer.h ../common/mcl_file_io.h
mcl_meta_object.o: mcl.h mcl_error.h ../../include/rse_fec.h
mcl_meta_object.o: ../../include/ldpc_fec.h
mcl_meta_object.o: ../../include/ldpc_matrix_sparse.h ../../include/tools.h
mcl_meta_object.o: ../../include/ldpc_create_pchk.h mcl_lib.h mcl_data.h
mcl_meta_object.o: mcl_vrmem.h mcl_rx_storage.h mcl_tx_tab.h mcl_rlc.h
mcl_meta_object.o: mcl_flid_sl.h mcl_socket.h mcl_periodic_proc.h
mcl_meta_object.o: mcl_lct_hdr.h mcl_alc_hdr.h mcl_stats.h mcl_fsm.h
mcl_meta_object.o: mcl_proto.h mcl_rx.h mcl_rx_window.h mcl_flute.h mcl_tx.h
mcl_meta_object.o: mcl_cb.h mcl_fec.h mcl_meta_object.h
mcl_meta_object.o: mcl_meta_object_layer.h
mcl_meta_object_layer.o: mcl_includes.h ../common/mcl_types.h
mcl_meta_object_layer.o: ../common/mcl_osdep.h mcl_profile.h
mcl_meta_object_layer.o: ../common/mcl_defines.h ../common/mcl_lib_api.h
mcl_meta_object_layer.o: ../common/mcl_types.h ../alc/mcl_lib_api_alc.h
mcl_meta_object_layer.o: ../common/mcl_version.h ../common/mcl_addr.h
mcl_meta_object_layer.o: ../common/ssm_for_linux.h ../common/mcl_socket_lib.h
mcl_meta_object_layer.o: ../common/mcl_debug.h ../common/mcl_itime.h
mcl_meta_object_layer.o: ../common/mcl_blocking_struct.h
mcl_meta_object_layer.o: ../common/mcl_periodic_timer.h
mcl_meta_object_layer.o: ../common/mcl_file_io.h mcl.h mcl_error.h
mcl_meta_object_layer.o: ../../include/rse_fec.h ../../include/ldpc_fec.h
mcl_meta_object_layer.o: ../../include/ldpc_matrix_sparse.h
mcl_meta_object_layer.o: ../../include/tools.h
mcl_meta_object_layer.o: ../../include/ldpc_create_pchk.h mcl_lib.h
mcl_meta_object_layer.o: mcl_data.h mcl_vrmem.h mcl_rx_storage.h mcl_tx_tab.h
mcl_meta_object_layer.o: mcl_rlc.h mcl_flid_sl.h mcl_socket.h
mcl_meta_object_layer.o: mcl_periodic_proc.h mcl_lct_hdr.h mcl_alc_hdr.h
mcl_meta_object_layer.o: mcl_stats.h mcl_fsm.h mcl_proto.h mcl_rx.h
mcl_meta_object_layer.o: mcl_rx_window.h mcl_flute.h mcl_tx.h mcl_cb.h
mcl_meta_object_layer.o: mcl_fec.h mcl_meta_object.h mcl_meta_object_layer.h
