# $Id: Makefile,v 1.22 2005/03/23 14:05:04 roca Exp $
#
# Makefile
#

###### OS Dependant Definitions ######

# they are defined in the following file...
# edit Makefile.common as appropriate
include ../../Makefile.common


ifeq (${OS},linux)
	# Linux version
	# A few libs for memory debug:
	# -lefence	for memory debug on Linux (and other plateforms)
	#		Requires: export EF_PROTECT_FREE=1
	#			  export EF_PROTECT_BELOW=1
	#
	#LIBRARIES = -Wl,-Bstatic -lefence -Wl,-Bdynamic -lc -lm -lpthread
	LIBRARIES = -lc -lm -lpthread 
	BINDIR = ../../bin/linux
	LARGEFILE_FLAGS = -D_FILE_OFFSET_BITS=64 -D_LARGEFILE_SOURCE
endif

ifeq (${OS},solaris)
	# Solaris version: requires -lsocket -lnsl and -L/usr/ucblib -lucb
	#
	LIBRARIES = -lc -lm -lsocket -lnsl -L/usr/ucblib -lucb -lpthread
	BINDIR = ../../bin/solaris
	LARGEFILE_FLAGS = -D_FILE_OFFSET_BITS=64 -D_LARGEFILE_SOURCE
endif

ifeq (${OS},freebsd)
	# FreeBSD version
	BINDIR = ../../bin/freebsd
	LDPC_BINDIR = ../../../ldpc/bin/freebsd
	LIBRARIES = -lm -pthread
	# TBC
	LARGEFILE_FLAGS = -D_FILE_OFFSET_BITS=64 -D_LARGEFILE_SOURCE
endif


###### Other Variables ######

# Common source files are here...
COMMON_DIR = ../common/

# List of source, object and include files
#
SRCFILES  = ${COMMON_DIR}/mcl_addr.cpp ${COMMON_DIR}/mcl_debug.cpp ${COMMON_DIR}/mcl_itime.cpp ${COMMON_DIR}/mcl_osdep.cpp ${COMMON_DIR}/rse_fec.cpp ${COMMON_DIR}/mcl_periodic_timer.cpp mcl.cpp mcl_cb.cpp mcl_lib.cpp mcl_adu.cpp mcl_block.cpp mcl_du.cpp mcl_fec.cpp mcl_fsm.cpp mcl_group_mgmt.cpp mcl_node.cpp mcl_timer.cpp mcl_periodic_proc.cpp mcl_ses_channel.cpp mcl_tx.cpp mcl_tx_window.cpp mcl_tx_storage.cpp mcl_tx_ctrl.cpp mcl_rx.cpp mcl_rx_thread.cpp mcl_rx_window.cpp mcl_rx_storage.cpp mcl_rx_ctrl.cpp mcl_stats.cpp mcl_norm_pkt_mgmt.cpp
OFILES = ${SRCFILES:.cpp=.o}


# General purpose local FLAGS.
# WARNING: with gcc/g++, never use -O3 optimisation (compiler bugs)
#
#CFLAGS	= -DNORM ${OSFLAGS} ${LARGEFILE_FLAGS} -DDEBUG -g -fcheck-memory-usage
CFLAGS	= -DNORM ${OSFLAGS} ${LARGEFILE_FLAGS} -DDEBUG -g	# debug version
#CFLAGS	= -DNORM ${OSFLAGS} ${LARGEFILE_FLAGS} -O -g 		# optimized 
#CFLAGS	= -DNORM ${OSFLAGS} ${LARGEFILE_FLAGS} -O -g -pg 	# optimized with profiling

LDFLAGS  =


###### Dependancies ######
# by default, create the MCL lib
all: lib

# Rule for C++ .cpp extension
.cpp.o:
	$(CC) -c $(CFLAGS) -o $*.o $*.cpp


lib: ${OFILES}
	@echo "** Now create the MCL-NORM library..."
	${AR} -rcsv ${BINDIR}/libmclnorm.a ${OFILES}
	${RANLIB} ${BINDIR}/libmclnorm.a
	@echo "done"

tags:
	etags $(SRCS)

clean:
	@for f in core ${OFILES} ; do \
		if [ -f "$${f}" ] ; then \
			rm "$${f}"; \
		fi \
	done

cleanall: clean
	@for f in ${BINDIR}/libmclnorm*.a ; do \
		if [ -f "$${f}" ] ; then \
			rm $${f} ; \
		fi \
	done

depend:
	# Use -Y to prevent including the system header files that won't
	# change. Send all error messages to /dev/null
	${DEPEND} -- ${CFLAGS} -Y -- ${SRCFILES} 2>/dev/null


# Dependencies automatically created by makedepend (see depend target above).
#
# DO NOT DELETE THIS LINE -- make depend depends on it.

../common//mcl_addr.o: ../norm/mcl_includes.h ../common/mcl_types.h
../common//mcl_addr.o: ../common/mcl_osdep.h mcl_profile.h
../common//mcl_addr.o: ../common/mcl_defines.h ../common/mcl_lib_api.h
../common//mcl_addr.o: ../common/mcl_types.h ../norm/mcl_lib_api_norm.h
../common//mcl_addr.o: ../common/mcl_version.h ../common/mcl_addr.h
../common//mcl_addr.o: ../common/mcl_debug.h ../common/mcl_itime.h
../common//mcl_addr.o: ../common/mcl_list.h ../common/rse_fec.h mcl.h
../common//mcl_addr.o: mcl_norm_hdr.h mcl_adu.h mcl_norm_pkt_mgmt.h mcl_du.h
../common//mcl_addr.o: mcl_block.h mcl_cb.h mcl_node.h mcl_fsm.h mcl_timer.h
../common//mcl_addr.o: mcl_timer_handler.h ../common/mcl_periodic_timer.h
../common//mcl_addr.o: mcl_periodic_proc.h mcl_fec.h mcl_stats.h
../common//mcl_addr.o: mcl_ses_channel.h mcl_tx.h mcl_lib_api_norm.h
../common//mcl_addr.o: mcl_tx_window.h mcl_tx_ctrl.h mcl_tx_storage.h
../common//mcl_addr.o: mcl_group_mgmt.h mcl_rx.h mcl_rx_window.h
../common//mcl_addr.o: mcl_rx_thread.h mcl_rx_ctrl.h mcl_rx_storage.h
../common//mcl_debug.o: ../norm/mcl_includes.h ../common/mcl_types.h
../common//mcl_debug.o: ../common/mcl_osdep.h mcl_profile.h
../common//mcl_debug.o: ../common/mcl_defines.h ../common/mcl_lib_api.h
../common//mcl_debug.o: ../common/mcl_types.h ../norm/mcl_lib_api_norm.h
../common//mcl_debug.o: ../common/mcl_version.h ../common/mcl_addr.h
../common//mcl_debug.o: ../common/mcl_debug.h ../common/mcl_itime.h
../common//mcl_debug.o: ../common/mcl_list.h ../common/rse_fec.h mcl.h
../common//mcl_debug.o: mcl_norm_hdr.h mcl_adu.h mcl_norm_pkt_mgmt.h mcl_du.h
../common//mcl_debug.o: mcl_block.h mcl_cb.h mcl_node.h mcl_fsm.h mcl_timer.h
../common//mcl_debug.o: mcl_timer_handler.h ../common/mcl_periodic_timer.h
../common//mcl_debug.o: mcl_periodic_proc.h mcl_fec.h mcl_stats.h
../common//mcl_debug.o: mcl_ses_channel.h mcl_tx.h mcl_lib_api_norm.h
../common//mcl_debug.o: mcl_tx_window.h mcl_tx_ctrl.h mcl_tx_storage.h
../common//mcl_debug.o: mcl_group_mgmt.h mcl_rx.h mcl_rx_window.h
../common//mcl_debug.o: mcl_rx_thread.h mcl_rx_ctrl.h mcl_rx_storage.h
../common//mcl_itime.o: ../norm/mcl_includes.h ../common/mcl_types.h
../common//mcl_itime.o: ../common/mcl_osdep.h mcl_profile.h
../common//mcl_itime.o: ../common/mcl_defines.h ../common/mcl_lib_api.h
../common//mcl_itime.o: ../common/mcl_types.h ../norm/mcl_lib_api_norm.h
../common//mcl_itime.o: ../common/mcl_version.h ../common/mcl_addr.h
../common//mcl_itime.o: ../common/mcl_debug.h ../common/mcl_itime.h
../common//mcl_itime.o: ../common/mcl_list.h ../common/rse_fec.h mcl.h
../common//mcl_itime.o: mcl_norm_hdr.h mcl_adu.h mcl_norm_pkt_mgmt.h mcl_du.h
../common//mcl_itime.o: mcl_block.h mcl_cb.h mcl_node.h mcl_fsm.h mcl_timer.h
../common//mcl_itime.o: mcl_timer_handler.h ../common/mcl_periodic_timer.h
../common//mcl_itime.o: mcl_periodic_proc.h mcl_fec.h mcl_stats.h
../common//mcl_itime.o: mcl_ses_channel.h mcl_tx.h mcl_lib_api_norm.h
../common//mcl_itime.o: mcl_tx_window.h mcl_tx_ctrl.h mcl_tx_storage.h
../common//mcl_itime.o: mcl_group_mgmt.h mcl_rx.h mcl_rx_window.h
../common//mcl_itime.o: mcl_rx_thread.h mcl_rx_ctrl.h mcl_rx_storage.h
../common//mcl_osdep.o: ../norm/mcl_includes.h ../common/mcl_types.h
../common//mcl_osdep.o: ../common/mcl_osdep.h mcl_profile.h
../common//mcl_osdep.o: ../common/mcl_defines.h ../common/mcl_lib_api.h
../common//mcl_osdep.o: ../common/mcl_types.h ../norm/mcl_lib_api_norm.h
../common//mcl_osdep.o: ../common/mcl_version.h ../common/mcl_addr.h
../common//mcl_osdep.o: ../common/mcl_debug.h ../common/mcl_itime.h
../common//mcl_osdep.o: ../common/mcl_list.h ../common/rse_fec.h mcl.h
../common//mcl_osdep.o: mcl_norm_hdr.h mcl_adu.h mcl_norm_pkt_mgmt.h mcl_du.h
../common//mcl_osdep.o: mcl_block.h mcl_cb.h mcl_node.h mcl_fsm.h mcl_timer.h
../common//mcl_osdep.o: mcl_timer_handler.h ../common/mcl_periodic_timer.h
../common//mcl_osdep.o: mcl_periodic_proc.h mcl_fec.h mcl_stats.h
../common//mcl_osdep.o: mcl_ses_channel.h mcl_tx.h mcl_lib_api_norm.h
../common//mcl_osdep.o: mcl_tx_window.h mcl_tx_ctrl.h mcl_tx_storage.h
../common//mcl_osdep.o: mcl_group_mgmt.h mcl_rx.h mcl_rx_window.h
../common//mcl_osdep.o: mcl_rx_thread.h mcl_rx_ctrl.h mcl_rx_storage.h
../common//rse_fec.o: ../common/mcl_debug.h
../common//mcl_periodic_timer.o: ../norm/mcl_includes.h ../common/mcl_types.h
../common//mcl_periodic_timer.o: ../common/mcl_osdep.h mcl_profile.h
../common//mcl_periodic_timer.o: ../common/mcl_defines.h
../common//mcl_periodic_timer.o: ../common/mcl_lib_api.h
../common//mcl_periodic_timer.o: ../common/mcl_types.h
../common//mcl_periodic_timer.o: ../norm/mcl_lib_api_norm.h
../common//mcl_periodic_timer.o: ../common/mcl_version.h ../common/mcl_addr.h
../common//mcl_periodic_timer.o: ../common/mcl_debug.h ../common/mcl_itime.h
../common//mcl_periodic_timer.o: ../common/mcl_list.h ../common/rse_fec.h
../common//mcl_periodic_timer.o: mcl.h mcl_norm_hdr.h mcl_adu.h
../common//mcl_periodic_timer.o: mcl_norm_pkt_mgmt.h mcl_du.h mcl_block.h
../common//mcl_periodic_timer.o: mcl_cb.h mcl_node.h mcl_fsm.h mcl_timer.h
../common//mcl_periodic_timer.o: mcl_timer_handler.h
../common//mcl_periodic_timer.o: ../common/mcl_periodic_timer.h
../common//mcl_periodic_timer.o: mcl_periodic_proc.h mcl_fec.h mcl_stats.h
../common//mcl_periodic_timer.o: mcl_ses_channel.h mcl_tx.h
../common//mcl_periodic_timer.o: mcl_lib_api_norm.h mcl_tx_window.h
../common//mcl_periodic_timer.o: mcl_tx_ctrl.h mcl_tx_storage.h
../common//mcl_periodic_timer.o: mcl_group_mgmt.h mcl_rx.h mcl_rx_window.h
../common//mcl_periodic_timer.o: mcl_rx_thread.h mcl_rx_ctrl.h
../common//mcl_periodic_timer.o: mcl_rx_storage.h
mcl.o: mcl_includes.h ../common/mcl_types.h ../common/mcl_osdep.h
mcl.o: mcl_profile.h ../common/mcl_defines.h ../common/mcl_lib_api.h
mcl.o: ../common/mcl_types.h ../norm/mcl_lib_api_norm.h
mcl.o: ../common/mcl_version.h ../common/mcl_addr.h ../common/mcl_debug.h
mcl.o: ../common/mcl_itime.h ../common/mcl_list.h ../common/rse_fec.h mcl.h
mcl.o: mcl_norm_hdr.h mcl_adu.h mcl_norm_pkt_mgmt.h mcl_du.h mcl_block.h
mcl.o: mcl_cb.h mcl_node.h mcl_fsm.h mcl_timer.h mcl_timer_handler.h
mcl.o: ../common/mcl_periodic_timer.h mcl_periodic_proc.h mcl_fec.h
mcl.o: mcl_stats.h mcl_ses_channel.h mcl_tx.h mcl_lib_api_norm.h
mcl.o: mcl_tx_window.h mcl_tx_ctrl.h mcl_tx_storage.h mcl_group_mgmt.h
mcl.o: mcl_rx.h mcl_rx_window.h mcl_rx_thread.h mcl_rx_ctrl.h
mcl.o: mcl_rx_storage.h
mcl_cb.o: mcl_includes.h ../common/mcl_types.h ../common/mcl_osdep.h
mcl_cb.o: mcl_profile.h ../common/mcl_defines.h ../common/mcl_lib_api.h
mcl_cb.o: ../common/mcl_types.h ../norm/mcl_lib_api_norm.h
mcl_cb.o: ../common/mcl_version.h ../common/mcl_addr.h ../common/mcl_debug.h
mcl_cb.o: ../common/mcl_itime.h ../common/mcl_list.h ../common/rse_fec.h
mcl_cb.o: mcl.h mcl_norm_hdr.h mcl_adu.h mcl_norm_pkt_mgmt.h mcl_du.h
mcl_cb.o: mcl_block.h mcl_cb.h mcl_node.h mcl_fsm.h mcl_timer.h
mcl_cb.o: mcl_timer_handler.h ../common/mcl_periodic_timer.h
mcl_cb.o: mcl_periodic_proc.h mcl_fec.h mcl_stats.h mcl_ses_channel.h
mcl_cb.o: mcl_tx.h mcl_lib_api_norm.h mcl_tx_window.h mcl_tx_ctrl.h
mcl_cb.o: mcl_tx_storage.h mcl_group_mgmt.h mcl_rx.h mcl_rx_window.h
mcl_cb.o: mcl_rx_thread.h mcl_rx_ctrl.h mcl_rx_storage.h
mcl_lib.o: mcl_includes.h ../common/mcl_types.h ../common/mcl_osdep.h
mcl_lib.o: mcl_profile.h ../common/mcl_defines.h ../common/mcl_lib_api.h
mcl_lib.o: ../common/mcl_types.h ../norm/mcl_lib_api_norm.h
mcl_lib.o: ../common/mcl_version.h ../common/mcl_addr.h ../common/mcl_debug.h
mcl_lib.o: ../common/mcl_itime.h ../common/mcl_list.h ../common/rse_fec.h
mcl_lib.o: mcl.h mcl_norm_hdr.h mcl_adu.h mcl_norm_pkt_mgmt.h mcl_du.h
mcl_lib.o: mcl_block.h mcl_cb.h mcl_node.h mcl_fsm.h mcl_timer.h
mcl_lib.o: mcl_timer_handler.h ../common/mcl_periodic_timer.h
mcl_lib.o: mcl_periodic_proc.h mcl_fec.h mcl_stats.h mcl_ses_channel.h
mcl_lib.o: mcl_tx.h mcl_lib_api_norm.h mcl_tx_window.h mcl_tx_ctrl.h
mcl_lib.o: mcl_tx_storage.h mcl_group_mgmt.h mcl_rx.h mcl_rx_window.h
mcl_lib.o: mcl_rx_thread.h mcl_rx_ctrl.h mcl_rx_storage.h
mcl_adu.o: mcl_includes.h ../common/mcl_types.h ../common/mcl_osdep.h
mcl_adu.o: mcl_profile.h ../common/mcl_defines.h ../common/mcl_lib_api.h
mcl_adu.o: ../common/mcl_types.h ../norm/mcl_lib_api_norm.h
mcl_adu.o: ../common/mcl_version.h ../common/mcl_addr.h ../common/mcl_debug.h
mcl_adu.o: ../common/mcl_itime.h ../common/mcl_list.h ../common/rse_fec.h
mcl_adu.o: mcl.h mcl_norm_hdr.h mcl_adu.h mcl_norm_pkt_mgmt.h mcl_du.h
mcl_adu.o: mcl_block.h mcl_cb.h mcl_node.h mcl_fsm.h mcl_timer.h
mcl_adu.o: mcl_timer_handler.h ../common/mcl_periodic_timer.h
mcl_adu.o: mcl_periodic_proc.h mcl_fec.h mcl_stats.h mcl_ses_channel.h
mcl_adu.o: mcl_tx.h mcl_lib_api_norm.h mcl_tx_window.h mcl_tx_ctrl.h
mcl_adu.o: mcl_tx_storage.h mcl_group_mgmt.h mcl_rx.h mcl_rx_window.h
mcl_adu.o: mcl_rx_thread.h mcl_rx_ctrl.h mcl_rx_storage.h
mcl_block.o: mcl_includes.h ../common/mcl_types.h ../common/mcl_osdep.h
mcl_block.o: mcl_profile.h ../common/mcl_defines.h ../common/mcl_lib_api.h
mcl_block.o: ../common/mcl_types.h ../norm/mcl_lib_api_norm.h
mcl_block.o: ../common/mcl_version.h ../common/mcl_addr.h
mcl_block.o: ../common/mcl_debug.h ../common/mcl_itime.h ../common/mcl_list.h
mcl_block.o: ../common/rse_fec.h mcl.h mcl_norm_hdr.h mcl_adu.h
mcl_block.o: mcl_norm_pkt_mgmt.h mcl_du.h mcl_block.h mcl_cb.h mcl_node.h
mcl_block.o: mcl_fsm.h mcl_timer.h mcl_timer_handler.h
mcl_block.o: ../common/mcl_periodic_timer.h mcl_periodic_proc.h mcl_fec.h
mcl_block.o: mcl_stats.h mcl_ses_channel.h mcl_tx.h mcl_lib_api_norm.h
mcl_block.o: mcl_tx_window.h mcl_tx_ctrl.h mcl_tx_storage.h mcl_group_mgmt.h
mcl_block.o: mcl_rx.h mcl_rx_window.h mcl_rx_thread.h mcl_rx_ctrl.h
mcl_block.o: mcl_rx_storage.h
mcl_du.o: mcl_includes.h ../common/mcl_types.h ../common/mcl_osdep.h
mcl_du.o: mcl_profile.h ../common/mcl_defines.h ../common/mcl_lib_api.h
mcl_du.o: ../common/mcl_types.h ../norm/mcl_lib_api_norm.h
mcl_du.o: ../common/mcl_version.h ../common/mcl_addr.h ../common/mcl_debug.h
mcl_du.o: ../common/mcl_itime.h ../common/mcl_list.h ../common/rse_fec.h
mcl_du.o: mcl.h mcl_norm_hdr.h mcl_adu.h mcl_norm_pkt_mgmt.h mcl_du.h
mcl_du.o: mcl_block.h mcl_cb.h mcl_node.h mcl_fsm.h mcl_timer.h
mcl_du.o: mcl_timer_handler.h ../common/mcl_periodic_timer.h
mcl_du.o: mcl_periodic_proc.h mcl_fec.h mcl_stats.h mcl_ses_channel.h
mcl_du.o: mcl_tx.h mcl_lib_api_norm.h mcl_tx_window.h mcl_tx_ctrl.h
mcl_du.o: mcl_tx_storage.h mcl_group_mgmt.h mcl_rx.h mcl_rx_window.h
mcl_du.o: mcl_rx_thread.h mcl_rx_ctrl.h mcl_rx_storage.h
mcl_fec.o: mcl_includes.h ../common/mcl_types.h ../common/mcl_osdep.h
mcl_fec.o: mcl_profile.h ../common/mcl_defines.h ../common/mcl_lib_api.h
mcl_fec.o: ../common/mcl_types.h ../norm/mcl_lib_api_norm.h
mcl_fec.o: ../common/mcl_version.h ../common/mcl_addr.h ../common/mcl_debug.h
mcl_fec.o: ../common/mcl_itime.h ../common/mcl_list.h ../common/rse_fec.h
mcl_fec.o: mcl.h mcl_norm_hdr.h mcl_adu.h mcl_norm_pkt_mgmt.h mcl_du.h
mcl_fec.o: mcl_block.h mcl_cb.h mcl_node.h mcl_fsm.h mcl_timer.h
mcl_fec.o: mcl_timer_handler.h ../common/mcl_periodic_timer.h
mcl_fec.o: mcl_periodic_proc.h mcl_fec.h mcl_stats.h mcl_ses_channel.h
mcl_fec.o: mcl_tx.h mcl_lib_api_norm.h mcl_tx_window.h mcl_tx_ctrl.h
mcl_fec.o: mcl_tx_storage.h mcl_group_mgmt.h mcl_rx.h mcl_rx_window.h
mcl_fec.o: mcl_rx_thread.h mcl_rx_ctrl.h mcl_rx_storage.h
mcl_fsm.o: mcl_includes.h ../common/mcl_types.h ../common/mcl_osdep.h
mcl_fsm.o: mcl_profile.h ../common/mcl_defines.h ../common/mcl_lib_api.h
mcl_fsm.o: ../common/mcl_types.h ../norm/mcl_lib_api_norm.h
mcl_fsm.o: ../common/mcl_version.h ../common/mcl_addr.h ../common/mcl_debug.h
mcl_fsm.o: ../common/mcl_itime.h ../common/mcl_list.h ../common/rse_fec.h
mcl_fsm.o: mcl.h mcl_norm_hdr.h mcl_adu.h mcl_norm_pkt_mgmt.h mcl_du.h
mcl_fsm.o: mcl_block.h mcl_cb.h mcl_node.h mcl_fsm.h mcl_timer.h
mcl_fsm.o: mcl_timer_handler.h ../common/mcl_periodic_timer.h
mcl_fsm.o: mcl_periodic_proc.h mcl_fec.h mcl_stats.h mcl_ses_channel.h
mcl_fsm.o: mcl_tx.h mcl_lib_api_norm.h mcl_tx_window.h mcl_tx_ctrl.h
mcl_fsm.o: mcl_tx_storage.h mcl_group_mgmt.h mcl_rx.h mcl_rx_window.h
mcl_fsm.o: mcl_rx_thread.h mcl_rx_ctrl.h mcl_rx_storage.h
mcl_group_mgmt.o: mcl_includes.h ../common/mcl_types.h ../common/mcl_osdep.h
mcl_group_mgmt.o: mcl_profile.h ../common/mcl_defines.h
mcl_group_mgmt.o: ../common/mcl_lib_api.h ../common/mcl_types.h
mcl_group_mgmt.o: ../norm/mcl_lib_api_norm.h ../common/mcl_version.h
mcl_group_mgmt.o: ../common/mcl_addr.h ../common/mcl_debug.h
mcl_group_mgmt.o: ../common/mcl_itime.h ../common/mcl_list.h
mcl_group_mgmt.o: ../common/rse_fec.h mcl.h mcl_norm_hdr.h mcl_adu.h
mcl_group_mgmt.o: mcl_norm_pkt_mgmt.h mcl_du.h mcl_block.h mcl_cb.h
mcl_group_mgmt.o: mcl_node.h mcl_fsm.h mcl_timer.h mcl_timer_handler.h
mcl_group_mgmt.o: ../common/mcl_periodic_timer.h mcl_periodic_proc.h
mcl_group_mgmt.o: mcl_fec.h mcl_stats.h mcl_ses_channel.h mcl_tx.h
mcl_group_mgmt.o: mcl_lib_api_norm.h mcl_tx_window.h mcl_tx_ctrl.h
mcl_group_mgmt.o: mcl_tx_storage.h mcl_group_mgmt.h mcl_rx.h mcl_rx_window.h
mcl_group_mgmt.o: mcl_rx_thread.h mcl_rx_ctrl.h mcl_rx_storage.h
mcl_node.o: mcl_includes.h ../common/mcl_types.h ../common/mcl_osdep.h
mcl_node.o: mcl_profile.h ../common/mcl_defines.h ../common/mcl_lib_api.h
mcl_node.o: ../common/mcl_types.h ../norm/mcl_lib_api_norm.h
mcl_node.o: ../common/mcl_version.h ../common/mcl_addr.h
mcl_node.o: ../common/mcl_debug.h ../common/mcl_itime.h ../common/mcl_list.h
mcl_node.o: ../common/rse_fec.h mcl.h mcl_norm_hdr.h mcl_adu.h
mcl_node.o: mcl_norm_pkt_mgmt.h mcl_du.h mcl_block.h mcl_cb.h mcl_node.h
mcl_node.o: mcl_fsm.h mcl_timer.h mcl_timer_handler.h
mcl_node.o: ../common/mcl_periodic_timer.h mcl_periodic_proc.h mcl_fec.h
mcl_node.o: mcl_stats.h mcl_ses_channel.h mcl_tx.h mcl_lib_api_norm.h
mcl_node.o: mcl_tx_window.h mcl_tx_ctrl.h mcl_tx_storage.h mcl_group_mgmt.h
mcl_node.o: mcl_rx.h mcl_rx_window.h mcl_rx_thread.h mcl_rx_ctrl.h
mcl_node.o: mcl_rx_storage.h
mcl_timer.o: mcl_includes.h ../common/mcl_types.h ../common/mcl_osdep.h
mcl_timer.o: mcl_profile.h ../common/mcl_defines.h ../common/mcl_lib_api.h
mcl_timer.o: ../common/mcl_types.h ../norm/mcl_lib_api_norm.h
mcl_timer.o: ../common/mcl_version.h ../common/mcl_addr.h
mcl_timer.o: ../common/mcl_debug.h ../common/mcl_itime.h ../common/mcl_list.h
mcl_timer.o: ../common/rse_fec.h mcl.h mcl_norm_hdr.h mcl_adu.h
mcl_timer.o: mcl_norm_pkt_mgmt.h mcl_du.h mcl_block.h mcl_cb.h mcl_node.h
mcl_timer.o: mcl_fsm.h mcl_timer.h mcl_timer_handler.h
mcl_timer.o: ../common/mcl_periodic_timer.h mcl_periodic_proc.h mcl_fec.h
mcl_timer.o: mcl_stats.h mcl_ses_channel.h mcl_tx.h mcl_lib_api_norm.h
mcl_timer.o: mcl_tx_window.h mcl_tx_ctrl.h mcl_tx_storage.h mcl_group_mgmt.h
mcl_timer.o: mcl_rx.h mcl_rx_window.h mcl_rx_thread.h mcl_rx_ctrl.h
mcl_timer.o: mcl_rx_storage.h
mcl_periodic_proc.o: mcl_includes.h ../common/mcl_types.h
mcl_periodic_proc.o: ../common/mcl_osdep.h mcl_profile.h
mcl_periodic_proc.o: ../common/mcl_defines.h ../common/mcl_lib_api.h
mcl_periodic_proc.o: ../common/mcl_types.h ../norm/mcl_lib_api_norm.h
mcl_periodic_proc.o: ../common/mcl_version.h ../common/mcl_addr.h
mcl_periodic_proc.o: ../common/mcl_debug.h ../common/mcl_itime.h
mcl_periodic_proc.o: ../common/mcl_list.h ../common/rse_fec.h mcl.h
mcl_periodic_proc.o: mcl_norm_hdr.h mcl_adu.h mcl_norm_pkt_mgmt.h mcl_du.h
mcl_periodic_proc.o: mcl_block.h mcl_cb.h mcl_node.h mcl_fsm.h mcl_timer.h
mcl_periodic_proc.o: mcl_timer_handler.h ../common/mcl_periodic_timer.h
mcl_periodic_proc.o: mcl_periodic_proc.h mcl_fec.h mcl_stats.h
mcl_periodic_proc.o: mcl_ses_channel.h mcl_tx.h mcl_lib_api_norm.h
mcl_periodic_proc.o: mcl_tx_window.h mcl_tx_ctrl.h mcl_tx_storage.h
mcl_periodic_proc.o: mcl_group_mgmt.h mcl_rx.h mcl_rx_window.h
mcl_periodic_proc.o: mcl_rx_thread.h mcl_rx_ctrl.h mcl_rx_storage.h
mcl_ses_channel.o: mcl_includes.h ../common/mcl_types.h ../common/mcl_osdep.h
mcl_ses_channel.o: mcl_profile.h ../common/mcl_defines.h
mcl_ses_channel.o: ../common/mcl_lib_api.h ../common/mcl_types.h
mcl_ses_channel.o: ../norm/mcl_lib_api_norm.h ../common/mcl_version.h
mcl_ses_channel.o: ../common/mcl_addr.h ../common/mcl_debug.h
mcl_ses_channel.o: ../common/mcl_itime.h ../common/mcl_list.h
mcl_ses_channel.o: ../common/rse_fec.h mcl.h mcl_norm_hdr.h mcl_adu.h
mcl_ses_channel.o: mcl_norm_pkt_mgmt.h mcl_du.h mcl_block.h mcl_cb.h
mcl_ses_channel.o: mcl_node.h mcl_fsm.h mcl_timer.h mcl_timer_handler.h
mcl_ses_channel.o: ../common/mcl_periodic_timer.h mcl_periodic_proc.h
mcl_ses_channel.o: mcl_fec.h mcl_stats.h mcl_ses_channel.h mcl_tx.h
mcl_ses_channel.o: mcl_lib_api_norm.h mcl_tx_window.h mcl_tx_ctrl.h
mcl_ses_channel.o: mcl_tx_storage.h mcl_group_mgmt.h mcl_rx.h mcl_rx_window.h
mcl_ses_channel.o: mcl_rx_thread.h mcl_rx_ctrl.h mcl_rx_storage.h
mcl_tx.o: mcl_includes.h ../common/mcl_types.h ../common/mcl_osdep.h
mcl_tx.o: mcl_profile.h ../common/mcl_defines.h ../common/mcl_lib_api.h
mcl_tx.o: ../common/mcl_types.h ../norm/mcl_lib_api_norm.h
mcl_tx.o: ../common/mcl_version.h ../common/mcl_addr.h ../common/mcl_debug.h
mcl_tx.o: ../common/mcl_itime.h ../common/mcl_list.h ../common/rse_fec.h
mcl_tx.o: mcl.h mcl_norm_hdr.h mcl_adu.h mcl_norm_pkt_mgmt.h mcl_du.h
mcl_tx.o: mcl_block.h mcl_cb.h mcl_node.h mcl_fsm.h mcl_timer.h
mcl_tx.o: mcl_timer_handler.h ../common/mcl_periodic_timer.h
mcl_tx.o: mcl_periodic_proc.h mcl_fec.h mcl_stats.h mcl_ses_channel.h
mcl_tx.o: mcl_tx.h mcl_lib_api_norm.h mcl_tx_window.h mcl_tx_ctrl.h
mcl_tx.o: mcl_tx_storage.h mcl_group_mgmt.h mcl_rx.h mcl_rx_window.h
mcl_tx.o: mcl_rx_thread.h mcl_rx_ctrl.h mcl_rx_storage.h
mcl_tx_window.o: mcl_includes.h ../common/mcl_types.h ../common/mcl_osdep.h
mcl_tx_window.o: mcl_profile.h ../common/mcl_defines.h
mcl_tx_window.o: ../common/mcl_lib_api.h ../common/mcl_types.h
mcl_tx_window.o: ../norm/mcl_lib_api_norm.h ../common/mcl_version.h
mcl_tx_window.o: ../common/mcl_addr.h ../common/mcl_debug.h
mcl_tx_window.o: ../common/mcl_itime.h ../common/mcl_list.h
mcl_tx_window.o: ../common/rse_fec.h mcl.h mcl_norm_hdr.h mcl_adu.h
mcl_tx_window.o: mcl_norm_pkt_mgmt.h mcl_du.h mcl_block.h mcl_cb.h mcl_node.h
mcl_tx_window.o: mcl_fsm.h mcl_timer.h mcl_timer_handler.h
mcl_tx_window.o: ../common/mcl_periodic_timer.h mcl_periodic_proc.h mcl_fec.h
mcl_tx_window.o: mcl_stats.h mcl_ses_channel.h mcl_tx.h mcl_lib_api_norm.h
mcl_tx_window.o: mcl_tx_window.h mcl_tx_ctrl.h mcl_tx_storage.h
mcl_tx_window.o: mcl_group_mgmt.h mcl_rx.h mcl_rx_window.h mcl_rx_thread.h
mcl_tx_window.o: mcl_rx_ctrl.h mcl_rx_storage.h
mcl_tx_storage.o: mcl_includes.h ../common/mcl_types.h ../common/mcl_osdep.h
mcl_tx_storage.o: mcl_profile.h ../common/mcl_defines.h
mcl_tx_storage.o: ../common/mcl_lib_api.h ../common/mcl_types.h
mcl_tx_storage.o: ../norm/mcl_lib_api_norm.h ../common/mcl_version.h
mcl_tx_storage.o: ../common/mcl_addr.h ../common/mcl_debug.h
mcl_tx_storage.o: ../common/mcl_itime.h ../common/mcl_list.h
mcl_tx_storage.o: ../common/rse_fec.h mcl.h mcl_norm_hdr.h mcl_adu.h
mcl_tx_storage.o: mcl_norm_pkt_mgmt.h mcl_du.h mcl_block.h mcl_cb.h
mcl_tx_storage.o: mcl_node.h mcl_fsm.h mcl_timer.h mcl_timer_handler.h
mcl_tx_storage.o: ../common/mcl_periodic_timer.h mcl_periodic_proc.h
mcl_tx_storage.o: mcl_fec.h mcl_stats.h mcl_ses_channel.h mcl_tx.h
mcl_tx_storage.o: mcl_lib_api_norm.h mcl_tx_window.h mcl_tx_ctrl.h
mcl_tx_storage.o: mcl_tx_storage.h mcl_group_mgmt.h mcl_rx.h mcl_rx_window.h
mcl_tx_storage.o: mcl_rx_thread.h mcl_rx_ctrl.h mcl_rx_storage.h
mcl_tx_ctrl.o: mcl_includes.h ../common/mcl_types.h ../common/mcl_osdep.h
mcl_tx_ctrl.o: mcl_profile.h ../common/mcl_defines.h ../common/mcl_lib_api.h
mcl_tx_ctrl.o: ../common/mcl_types.h ../norm/mcl_lib_api_norm.h
mcl_tx_ctrl.o: ../common/mcl_version.h ../common/mcl_addr.h
mcl_tx_ctrl.o: ../common/mcl_debug.h ../common/mcl_itime.h
mcl_tx_ctrl.o: ../common/mcl_list.h ../common/rse_fec.h mcl.h mcl_norm_hdr.h
mcl_tx_ctrl.o: mcl_adu.h mcl_norm_pkt_mgmt.h mcl_du.h mcl_block.h mcl_cb.h
mcl_tx_ctrl.o: mcl_node.h mcl_fsm.h mcl_timer.h mcl_timer_handler.h
mcl_tx_ctrl.o: ../common/mcl_periodic_timer.h mcl_periodic_proc.h mcl_fec.h
mcl_tx_ctrl.o: mcl_stats.h mcl_ses_channel.h mcl_tx.h mcl_lib_api_norm.h
mcl_tx_ctrl.o: mcl_tx_window.h mcl_tx_ctrl.h mcl_tx_storage.h
mcl_tx_ctrl.o: mcl_group_mgmt.h mcl_rx.h mcl_rx_window.h mcl_rx_thread.h
mcl_tx_ctrl.o: mcl_rx_ctrl.h mcl_rx_storage.h
mcl_rx.o: mcl_includes.h ../common/mcl_types.h ../common/mcl_osdep.h
mcl_rx.o: mcl_profile.h ../common/mcl_defines.h ../common/mcl_lib_api.h
mcl_rx.o: ../common/mcl_types.h ../norm/mcl_lib_api_norm.h
mcl_rx.o: ../common/mcl_version.h ../common/mcl_addr.h ../common/mcl_debug.h
mcl_rx.o: ../common/mcl_itime.h ../common/mcl_list.h ../common/rse_fec.h
mcl_rx.o: mcl.h mcl_norm_hdr.h mcl_adu.h mcl_norm_pkt_mgmt.h mcl_du.h
mcl_rx.o: mcl_block.h mcl_cb.h mcl_node.h mcl_fsm.h mcl_timer.h
mcl_rx.o: mcl_timer_handler.h ../common/mcl_periodic_timer.h
mcl_rx.o: mcl_periodic_proc.h mcl_fec.h mcl_stats.h mcl_ses_channel.h
mcl_rx.o: mcl_tx.h mcl_lib_api_norm.h mcl_tx_window.h mcl_tx_ctrl.h
mcl_rx.o: mcl_tx_storage.h mcl_group_mgmt.h mcl_rx.h mcl_rx_window.h
mcl_rx.o: mcl_rx_thread.h mcl_rx_ctrl.h mcl_rx_storage.h
mcl_rx_thread.o: mcl_includes.h ../common/mcl_types.h ../common/mcl_osdep.h
mcl_rx_thread.o: mcl_profile.h ../common/mcl_defines.h
mcl_rx_thread.o: ../common/mcl_lib_api.h ../common/mcl_types.h
mcl_rx_thread.o: ../norm/mcl_lib_api_norm.h ../common/mcl_version.h
mcl_rx_thread.o: ../common/mcl_addr.h ../common/mcl_debug.h
mcl_rx_thread.o: ../common/mcl_itime.h ../common/mcl_list.h
mcl_rx_thread.o: ../common/rse_fec.h mcl.h mcl_norm_hdr.h mcl_adu.h
mcl_rx_thread.o: mcl_norm_pkt_mgmt.h mcl_du.h mcl_block.h mcl_cb.h mcl_node.h
mcl_rx_thread.o: mcl_fsm.h mcl_timer.h mcl_timer_handler.h
mcl_rx_thread.o: ../common/mcl_periodic_timer.h mcl_periodic_proc.h mcl_fec.h
mcl_rx_thread.o: mcl_stats.h mcl_ses_channel.h mcl_tx.h mcl_lib_api_norm.h
mcl_rx_thread.o: mcl_tx_window.h mcl_tx_ctrl.h mcl_tx_storage.h
mcl_rx_thread.o: mcl_group_mgmt.h mcl_rx.h mcl_rx_window.h mcl_rx_thread.h
mcl_rx_thread.o: mcl_rx_ctrl.h mcl_rx_storage.h
mcl_rx_window.o: mcl_includes.h ../common/mcl_types.h ../common/mcl_osdep.h
mcl_rx_window.o: mcl_profile.h ../common/mcl_defines.h
mcl_rx_window.o: ../common/mcl_lib_api.h ../common/mcl_types.h
mcl_rx_window.o: ../norm/mcl_lib_api_norm.h ../common/mcl_version.h
mcl_rx_window.o: ../common/mcl_addr.h ../common/mcl_debug.h
mcl_rx_window.o: ../common/mcl_itime.h ../common/mcl_list.h
mcl_rx_window.o: ../common/rse_fec.h mcl.h mcl_norm_hdr.h mcl_adu.h
mcl_rx_window.o: mcl_norm_pkt_mgmt.h mcl_du.h mcl_block.h mcl_cb.h mcl_node.h
mcl_rx_window.o: mcl_fsm.h mcl_timer.h mcl_timer_handler.h
mcl_rx_window.o: ../common/mcl_periodic_timer.h mcl_periodic_proc.h mcl_fec.h
mcl_rx_window.o: mcl_stats.h mcl_ses_channel.h mcl_tx.h mcl_lib_api_norm.h
mcl_rx_window.o: mcl_tx_window.h mcl_tx_ctrl.h mcl_tx_storage.h
mcl_rx_window.o: mcl_group_mgmt.h mcl_rx.h mcl_rx_window.h mcl_rx_thread.h
mcl_rx_window.o: mcl_rx_ctrl.h mcl_rx_storage.h
mcl_rx_storage.o: mcl_includes.h ../common/mcl_types.h ../common/mcl_osdep.h
mcl_rx_storage.o: mcl_profile.h ../common/mcl_defines.h
mcl_rx_storage.o: ../common/mcl_lib_api.h ../common/mcl_types.h
mcl_rx_storage.o: ../norm/mcl_lib_api_norm.h ../common/mcl_version.h
mcl_rx_storage.o: ../common/mcl_addr.h ../common/mcl_debug.h
mcl_rx_storage.o: ../common/mcl_itime.h ../common/mcl_list.h
mcl_rx_storage.o: ../common/rse_fec.h mcl.h mcl_norm_hdr.h mcl_adu.h
mcl_rx_storage.o: mcl_norm_pkt_mgmt.h mcl_du.h mcl_block.h mcl_cb.h
mcl_rx_storage.o: mcl_node.h mcl_fsm.h mcl_timer.h mcl_timer_handler.h
mcl_rx_storage.o: ../common/mcl_periodic_timer.h mcl_periodic_proc.h
mcl_rx_storage.o: mcl_fec.h mcl_stats.h mcl_ses_channel.h mcl_tx.h
mcl_rx_storage.o: mcl_lib_api_norm.h mcl_tx_window.h mcl_tx_ctrl.h
mcl_rx_storage.o: mcl_tx_storage.h mcl_group_mgmt.h mcl_rx.h mcl_rx_window.h
mcl_rx_storage.o: mcl_rx_thread.h mcl_rx_ctrl.h mcl_rx_storage.h
mcl_rx_ctrl.o: mcl_includes.h ../common/mcl_types.h ../common/mcl_osdep.h
mcl_rx_ctrl.o: mcl_profile.h ../common/mcl_defines.h ../common/mcl_lib_api.h
mcl_rx_ctrl.o: ../common/mcl_types.h ../norm/mcl_lib_api_norm.h
mcl_rx_ctrl.o: ../common/mcl_version.h ../common/mcl_addr.h
mcl_rx_ctrl.o: ../common/mcl_debug.h ../common/mcl_itime.h
mcl_rx_ctrl.o: ../common/mcl_list.h ../common/rse_fec.h mcl.h mcl_norm_hdr.h
mcl_rx_ctrl.o: mcl_adu.h mcl_norm_pkt_mgmt.h mcl_du.h mcl_block.h mcl_cb.h
mcl_rx_ctrl.o: mcl_node.h mcl_fsm.h mcl_timer.h mcl_timer_handler.h
mcl_rx_ctrl.o: ../common/mcl_periodic_timer.h mcl_periodic_proc.h mcl_fec.h
mcl_rx_ctrl.o: mcl_stats.h mcl_ses_channel.h mcl_tx.h mcl_lib_api_norm.h
mcl_rx_ctrl.o: mcl_tx_window.h mcl_tx_ctrl.h mcl_tx_storage.h
mcl_rx_ctrl.o: mcl_group_mgmt.h mcl_rx.h mcl_rx_window.h mcl_rx_thread.h
mcl_rx_ctrl.o: mcl_rx_ctrl.h mcl_rx_storage.h
mcl_stats.o: mcl_includes.h ../common/mcl_types.h ../common/mcl_osdep.h
mcl_stats.o: mcl_profile.h ../common/mcl_defines.h ../common/mcl_lib_api.h
mcl_stats.o: ../common/mcl_types.h ../norm/mcl_lib_api_norm.h
mcl_stats.o: ../common/mcl_version.h ../common/mcl_addr.h
mcl_stats.o: ../common/mcl_debug.h ../common/mcl_itime.h ../common/mcl_list.h
mcl_stats.o: ../common/rse_fec.h mcl.h mcl_norm_hdr.h mcl_adu.h
mcl_stats.o: mcl_norm_pkt_mgmt.h mcl_du.h mcl_block.h mcl_cb.h mcl_node.h
mcl_stats.o: mcl_fsm.h mcl_timer.h mcl_timer_handler.h
mcl_stats.o: ../common/mcl_periodic_timer.h mcl_periodic_proc.h mcl_fec.h
mcl_stats.o: mcl_stats.h mcl_ses_channel.h mcl_tx.h mcl_lib_api_norm.h
mcl_stats.o: mcl_tx_window.h mcl_tx_ctrl.h mcl_tx_storage.h mcl_group_mgmt.h
mcl_stats.o: mcl_rx.h mcl_rx_window.h mcl_rx_thread.h mcl_rx_ctrl.h
mcl_stats.o: mcl_rx_storage.h
mcl_norm_pkt_mgmt.o: mcl_includes.h ../common/mcl_types.h
mcl_norm_pkt_mgmt.o: ../common/mcl_osdep.h mcl_profile.h
mcl_norm_pkt_mgmt.o: ../common/mcl_defines.h ../common/mcl_lib_api.h
mcl_norm_pkt_mgmt.o: ../common/mcl_types.h ../norm/mcl_lib_api_norm.h
mcl_norm_pkt_mgmt.o: ../common/mcl_version.h ../common/mcl_addr.h
mcl_norm_pkt_mgmt.o: ../common/mcl_debug.h ../common/mcl_itime.h
mcl_norm_pkt_mgmt.o: ../common/mcl_list.h ../common/rse_fec.h mcl.h
mcl_norm_pkt_mgmt.o: mcl_norm_hdr.h mcl_adu.h mcl_norm_pkt_mgmt.h mcl_du.h
mcl_norm_pkt_mgmt.o: mcl_block.h mcl_cb.h mcl_node.h mcl_fsm.h mcl_timer.h
mcl_norm_pkt_mgmt.o: mcl_timer_handler.h ../common/mcl_periodic_timer.h
mcl_norm_pkt_mgmt.o: mcl_periodic_proc.h mcl_fec.h mcl_stats.h
mcl_norm_pkt_mgmt.o: mcl_ses_channel.h mcl_tx.h mcl_lib_api_norm.h
mcl_norm_pkt_mgmt.o: mcl_tx_window.h mcl_tx_ctrl.h mcl_tx_storage.h
mcl_norm_pkt_mgmt.o: mcl_group_mgmt.h mcl_rx.h mcl_rx_window.h
mcl_norm_pkt_mgmt.o: mcl_rx_thread.h mcl_rx_ctrl.h mcl_rx_storage.h
