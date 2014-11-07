/* $Id: mcl_profile.h,v 1.44 2005/05/23 14:35:54 roca Exp $ */
/*
 *  Copyright (c) 1999-2004 INRIA - All rights reserved
 *  (main author: Vincent Roca - vincent.roca@inrialpes.fr)
 *
 *  This program is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU General Public License
 *  as published by the Free Software Foundation; either version 2
 *  of the License, or (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307,
 *  USA.
 */

/*
 * This file contains all the parameters/constants/preprocessing flags
 * controlling the profile of the MCL library. Edit as appropriate...
 * Most of the values given here can be further modified using mcl_ctl().
 *
 * Of course this file must be the first one included!!!
 */

#ifndef MCL_PROFILE_H  /* { */
#define MCL_PROFILE_H


/****** ALC profile options ***************************************************/

/*
 * Do you want IPv6 support?
 */
#define INET6


/*
 * Do you want SSM (Source Specific Multicast) support?
 */
#define SSM


/*
 * Do you want physical object aggregation support?
 */
#define METAOBJECT_USED


#if 0	// not yet
/*
 * Do you want to enable the use of Virtual Transmission Memory service?
 * Really really recommanded if you deal with very large files.
 */
/* NB: mode set by default in each new session if you define VIRTUAL_TX_MEM */
//#define VIRTUAL_TX_MEM
#ifdef VIRTUAL_TX_MEM
/*
 * what is the maximum size of the physical memory area before storing data
 * on disk? Adjust according to your desires/available physical memory...
 */
const INT32	VIRTUAL_TX_MEM_MAX_BUFFERING_SIZE =	30*1024*1024;
//const INT32	VIRTUAL_TX_MEM_MAX_BUFFERING_SIZE =	100*1024*1024;
#endif
#endif // 0


/*
 * Do you want to enable the use of Virtual Reception Memory service?
 * Really really recommanded if you deal with very large files.
 */
/* NB: mode set by default in each new session if you define VIRTUAL_RX_MEM */
#define VIRTUAL_RX_MEM

#ifdef VIRTUAL_RX_MEM
/*
 * what is the maximum size of the physical memory area before storing data
 * on disk? Adjust according to your desires/available physical memory...
 */
const INT32	VIRTUAL_RX_MEM_MAX_BUFFERING_SIZE =	10*1024*1024;
//const INT32	VIRTUAL_RX_MEM_MAX_BUFFERING_SIZE =	100*1024*1024;

/**
 * Do you want to enable several receivers to use the VRMEM on the
 * same host simultaneously or not?
 * If enabled, then each VRM temporary file will be given a different
 * file name, otherwise the same name will be used.
 * By default we use the same name, since in case of crash, this temporary
 * file will be overwritten automatically (instead of having several old
 * versions, that take more andmore place).
 * The downside is that there can be at most a single receiver per host,
 * otherwise crashes will result (several receivers will access the same
 * file simultaneously...).
 */
#define	VIRTUAL_RX_MEM_SINGLE_RECEIVER_PER_HOST_MODE
#endif


/*
 * Do you want to use congestion control (derived from RLC,
 * Receiver Driver Layered Congestion Control Scheme) or not ? 
 */
#define	RLC


/*
 * Do you want to use FLID Static Layer congestion control module or not ? 
 */
#define	FLIDS


/*
 * Do you want to use Reed-Solomon Forward Error Correction (FEC) ?
 */
#define RSE_FEC


/*
 * Do you want to use LDPC large block Forward Error Correction (FEC) ?
 * Highly recommended when sending large objects...
 */
#define LDPC_FEC

#if defined(RSE_FEC) || defined(LDPC_FEC)
#define FEC
#endif


/*
 * by default, do you want to postpone FEC decoding at a receiver or not?
 * it is recommended to postpone with very high speed reception rates and/or
 * slow CPUs.
 *
 * WARNING: do not use if you have many independant tx to do as the receiver
 * will be blocked, waiting for the end of all the tx.
 */
/*#define POSTPONE_FEC_DECODING*/


/*
 * an optimization for transmissions in PUSH mode.
 * highly recommended!
 */
#define ANTICIPATED_TX_FOR_PUSH
#ifdef ANTICIPATED_TX_FOR_PUSH
/*
 * select an aggressiveness.
 * depends on the receivers capabilities, and the desire to favor some
 * of them (low-end receivers versus high-end receivers)
 */
//#define ANTICIPATED_TX_FOR_PUSH_AGGRESSIVENESS 3	/* favors low-end rx */
#define ANTICIPATED_TX_FOR_PUSH_AGGRESSIVENESS 4	/* default */
//#define ANTICIPATED_TX_FOR_PUSH_AGGRESSIVENESS 5	/* favors high-end rx */
#endif /* ANTICIPATED_TX_FOR_PUSH */


/*
 * What scheduling algorithm(s) do you need (there can be more than 1) ?
 */
/*
 * LCT_SCHED1
 * transmit all the original+FEC DUs on each layer in a random order
 * NB: now required since it is the only scheduling left!
 */
#define	LCT_SCHED1



/****** MCL lib profile options ***********************************************/


/**
 * Max number of sessions per instance of the MCL library.
 * Large numbers are possible if you have enough memory, disk space and CPU
 * power ;-)
 * Note that some systems (e.g. solaris) are more strict on the default
 * maximum number of sockets in use.
 * Note also that setting MCLCB_MAX_ID to a certain value does not mean
 * that you can open up to this value, because of the way IDs are allocated
 * (see mcl_cb::mcl_cb). To get a unique ID in the file descriptor space,
 * we duplicate stdin and use the returned value. Therefore, since file
 * descriptors 0, 1, and 2 are already allocated, MCLCB_MAX_ID should be
 * at least equal to 3, or more if the upper application has opened one
 * or more file descriptors.
 */
const INT32	MAX_NB_MCLCB = 20;


#ifdef FEC
/*
 * parameters associated to the use of FEC:
 * - maximum value of k for (k,n) FEC codecs; this is also the number
 *   of source symbols (i.e. of DUs) per block
 * - maximum value of n for (k, n) FEC codecs; n-k is also the maximum number
 *   of FEC symbols that can be generated per block
 * - FEC redundancy, or n/k ratio; e.g. 1.5 for 50% of FEC, 2.0 for 100%
 *   which is a by product of setting the k and n parameters
 */
#ifdef RSE_FEC
#define		RSE_MAX_K   GF_SIZE	/* at most max_k DUs per source block */
#define		RSE_MAX_N   GF_SIZE	/* at most max_n DUs after encoding */
const INT32	RSE_DEFAULT_K = 64;	/* default */
const INT32	RSE_DEFAULT_N = 128;	/* default */
#endif
#ifdef LDPC_FEC
const INT32	LDPC_MAX_K = 200*1000;	/* at most 200*10^3 packets per block */
const INT32	LDPC_MAX_N = 200*1000;	/* at most 200*10^3 packets after enc */
const INT32	LDPC_DEFAULT_K = 20*1000; /* default: 20*10^3 packets */
const INT32	LDPC_DEFAULT_N = 40*1000; /* default: 40*10^3 packets */
#endif
#endif /* FEC */


/*
 * default transmission profile
 * choose one (can be overided with mcl_ctl anyway)
 *
 * WARNING1: using a DFLT_TX_PROFILE overrides the actual max_tx_level,
 * 	datagram_size, and tx_rate... (see mcl_tx_prof.c)
 * WARNING2: a change of the default datagram size must be done both at
 * 	source and receivers.
 */
/*#define DFLT_TX_PROFILE		MCL_TX_PROFILE_LOW_RATE_INTERNET*/
#define DFLT_TX_PROFILE			MCL_TX_PROFILE_MID_RATE_INTERNET
/*#define DFLT_TX_PROFILE		MCL_TX_PROFILE_HIGH_SPEED_LAN*/


/*
 * max number of transmission layers
 * max number of multicast groups
 */
#define MAX_NB_TX_LAYERS	20	/* max nb of layers */
#define MAX_MC_GROUP		MAX_NB_TX_LAYERS


/*
 * mtu          max tx unit as defined by the physical layer (it does not
 *              consider IP and above headers). Be careful, if you know
 *		that tunneling is used, reduce this size!
 * payload      data payload of an ALC packet (if any)
 * header       ALC/LCT header only
 * datagram     includes payload plus ALC/LCT header.
 *		This is the Protocol Data Unit given to UDP
 * ip_datagram  includes datagram plus UDP/IP headers
 *
 * WARNING: changes of these sizes must be done both at source and receivers...
 */
//const INT32	DFLT_MTU = 576;
const INT32	DFLT_MTU = 1400;
const INT32	MAX_ETHERNET_MTU = 1500;

const INT32	UDP_IPv4_HEADER_SIZE = (8 + 20);        // in case of IPv4
const INT32	UDP_IPv6_HEADER_SIZE = (8 + 40);        // in case of IPv6

// ALC/LCT headers cannot be smaller in any case
const INT32	MIN_ALC_HEADER_SIZE = 8;
// ALC/LCT headers cannot be larger
// 4(fixed hdr) + 4(CCI) + 4(TSI) + 4(TOI) + 24(our FTI) + 8(No NEW ADU)
// + 8(FPI) + 4 (FDT/MODT)
// This maximum header size is really MCL specific, since it takes into
// account features not supported by MCL (e.g. CCI of more than 32bits),
// and additional fields required by MCL (FTI for FEC Enc ID 140, No NEW ADU).
const INT32	MAX_ALC_HEADER_SIZE = 60;

#ifdef INET6
// if compiled with IPv6 support, then be conservative: reserve room
// for IPv6 headers...
const INT32	MAX_DATAGRAM_SIZE = (DFLT_MTU - UDP_IPv6_HEADER_SIZE);
#else
const INT32	MAX_DATAGRAM_SIZE = (DFLT_MTU - UDP_IPv4_HEADER_SIZE);
#endif
const INT32	DFLT_DATAGRAM_SIZE = MAX_DATAGRAM_SIZE;

const INT32	MAX_PAYLOAD_SIZE = (MAX_DATAGRAM_SIZE -
                                    MAX_ALC_HEADER_SIZE);
const INT32	DFLT_PAYLOAD_SIZE = MAX_PAYLOAD_SIZE;


/*
 * period of the main timer; used to trigger all the periodic processings
 * (congestion control timers, data transmissions, etc.)
 */
/*
 * this value (in microseconds) determines the internal MCL clock granularity.
 * It is the sleep duration between two consecutive virtual time ticks.
 * All other timers (see below) are multiples of this period!
 * (eg. MCL_TX_PERIOD can be set to 2*MCL_TIMER_PERIOD for half the frequency)
 *
 * The MCL_PERIODIC_TIMER_PERIOD is set in common/mcl_periodic_timer.h
 * and can be changed there.
 */
#define MCL_TIMER_PERIOD	MCL_PERIODIC_TIMER_PERIOD

/*
 * period in microseconds between two calls to the RLC timer func (receiver).
 * NB: must be a multiple of MCL_TIMER_PERIOD!
 */
#define MCL_RLC_RX_PERIOD	(10 * MCL_TIMER_PERIOD)


/*
 * default polling period...
 * This is a compromize, not too high to limit resource consumption,
 * not too low to have a good reactivity.
 * (using MCL_TIMER_PERIOD is nice as it defines the max rate at which new
 * packets are sent and thus events can take place)
 */
#define DFLT_POLLING_PERIOD	((unsigned long)4 * MCL_TIMER_PERIOD)


#ifdef METAOBJECT_USED
/*
 * polling period in the case Metaobjects are used
 * it is a multiple of DFLT_POLLING_PERIOD since
 * searching decoded obkects in a metaobject uses
 * quite a lot CPU 
 */
#define MODT_POLLING_PERIOD	((unsigned long)25 * DFLT_POLLING_PERIOD)
#endif

/*
 * send important data every MCL_IMPORTANT_DATA_FREQUENCY (microseconds)
 * Take care: amount of important data may become not important enough if 
 * we have high transmission rates and/or only one tx layer.
 */
#define IMPORTANT_DATA_FREQUENCY 250000.0   //1000000.0 = 1 second

/*
 * Do you want to simulate random bursty packet losses?
 * (i.e. the source will randomly forget to send some bursts of packets)
 * Usually NO...
 */
/* #define	SIMUL_LOSSES */

#ifdef SIMUL_LOSSES
/*
 * Should losses be independant of layer number (ie be constant) or not ?
 */
//#define CONSTANT_LOSS_RATIO
/*
 * number of (simulated) losses in % when the previous packet was OK
 */
#define P_LOSS_WHEN_OK	0.1
//#define P_LOSS_WHEN_OK	0.5
//#define P_LOSS_WHEN_OK		1
/*
 * number of (simulated) losses in % when the previous packet was LOST 
 */
#define P_LOSS_WHEN_LOSSES	10
//#define P_LOSS_WHEN_LOSSES	75
#endif /* SIMUL_LOSSES */


#ifdef SOLARIS
/*
 * Do you want to collect system information?
 * IMPORTANT:	Requires to edit mcl_stats.c!!! Change the login name and
 * 		tool name (in the grep)
 *		Ok, that's a bit ugly but it is so simple...
 * NB: Only works on solaris today!
 */
/*#define GET_SYSINFO*/

#ifdef GET_SYSINFO
#define MCL_SYSINFO_PERIOD	(3 * MCL_TIMER_PERIOD)
#endif /* GET_SYSINFO */
#endif /* SOLARIS */


/*
 * default ttl used for multicast tx
 */
#define TTL		1


/*
 * create temporary file in this dir
 * (e.g. used by the vtmem service)
 */
#ifdef WIN32
#define MCL_DEFAULT_TMP_DIR_NAME	""
#else  /* UNIX */
#define MCL_DEFAULT_TMP_DIR_NAME	"/tmp/"
#endif /* UNIX */


/*
 * do you want that all traces (out and error) are sent to a dedicated
 * file in the temporary directory defined above or to the stdout and
 * stderr standard output?
 */
/*#define STDOUT_TO_FILE*/

/* ODT parameters */
#define ODT_VERSION 1
#define MOID_LEN 2
#define NB_OF_OBJECTS_LEN 2
#define OBJECTS_LEN_LEN 4

#endif /* }  MCL_PROFILE_H */
