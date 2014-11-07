/* $Id: mcl_profile.h,v 1.8 2004/05/26 12:54:56 roca Exp $ */
/*
 *  Copyright (c) 2004 INRIA - All rights reserved
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
 * This file contains various parameters that can be modified to suit
 * specific needs. Modify with care in any case...
 * It is composed of two parts:
 *
 *  - PART 1: those are the NORM specific parameters.
 *
 *  - PART 2: those are the MCL specific parameters. They concern the
 *  	      library itself, and modifying them can help MCL suit a
 *  	      specific application (e.g. increase MCL_MAX_ID if the
 *  	      application opens many simultaneous sessions).
 *  	      This is also the place where packet loss rates can be
 *  	      specified for simulations...
 */

#ifndef MCL_PROFILE_H
#define MCL_PROFILE_H


/****** NORM profile options **************************************************/


/*
 * Do you want IPv6 support?
 */
#define INET6


/**
 * Some control packets are sent MCL_TX_ROBUSTNESS_FACTOR times
 * for improved robustness.
 */
const UINT32	MCL_TX_ROBUSTNESS_FACTOR = 10;
/** 2 should be enough for FLUSH transmissions since they are cumulative. */
const UINT32	MCL_TX_CMD_FLUSH_ROBUSTNESS_FACTOR = 2;


/*
 * Various NORM timers and parameters of the sender (see mcl_tx_ctrl.[h|cpp])
 */

/**
 * Waiting time after a FLUSH transmission.
 * At timeout, a new FLUSH can be sent again or the block free'ed.
 * Timer duration in msec.
 */
const INT32	FLUSH_DONE_TIMEOUT = 1000;

/**
 * Determines how many FLUSH messages must be sent before considering
 * that the block has been reliably delivered.
 * This process is interrupted in case a NACK for this block is received.
 */
const INT32	UNANSWERED_FLUSH_REPEAT_COUNT = 2; // send twice a FLUSH if
						   // there is no answer...

/**
 * Waiting time after a NACK processing (and repair packets have been sent)
 * before issuing a new FLUSH.
 * Timer duration in msec.
 */
const INT32	RETX_DONE_TIMEOUT = 1000;


/*
 * Various NORM timers and parameters of a receiver (see mcl_rx_ctrl.[h|cpp])
 */

/* ... */


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
 * or more files.
 */
const INT32	MAX_NB_MCLCB = 20;


/**
 * Do you want to use the Reed-Solomon FEC codec?
 */
#define RSE_FEC

#if defined(RSE_FEC) || defined(LDPC_FEC)
#define FEC
#endif


#ifdef RSE_FEC
/**
 * Do you want proactive FEC (ie. FEC packets sent along with data packets
 * to immediately recover possible erasures) or not?
 */
#define MCL_USE_PROACTIVE_FEC
#ifdef MCL_USE_PROACTIVE_FEC
/**
 * Defines the number of default proactive FEC packets created per block.
 * The number of FEC symbols created is equal to ratio * du_nb
 */
const float	MCL_PROACTIVE_FEC_RATIO = 0.10;
//const float	MCL_PROACTIVE_FEC_RATIO = 0.25;
#endif
#endif // RSE_FEC


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
//const double	MAX_FEC_RATIO = 4.0;	/* max ratio of fec pkts generated */
//const double	DEFAULT_FEC_RATIO = 1.5;/* dflt ratio of fec pkts generated */

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


/**
 * The size of the timer array.  This is the limit on the number of
 * simultaneous timers that may be active at any time.
 * Used by the RMF (mcl_timer.h/ccp) code.
 */
const INT32	MCL_TIMER_ARRAY_SIZE = 512;


/**
 * default polling period.
 * This is a compromize, not too high to limit resource consumption,
 * not too low to have a good reactivity.
 */
//const INT32	DFLT_POLLING_PERIOD = ((UINT32)5 * MCL_PERIODIC_TIMER_PERIOD);
#define	DFLT_POLLING_PERIOD	((UINT32)5 * MCL_PERIODIC_TIMER_PERIOD)


/**
 * Max and default size of sockets.
 * Must be large enough to absort packet bursts.
 */
const INT32	MCL_MAX_SOCKET_SIZE = (64*1024);
const INT32	MCL_DFLT_SOCKET_SIZE = MCL_MAX_SOCKET_SIZE;


/*
 * mtu		max tx unit as defined by the physical layer (it does not
 * 		consider IP and above headers)
 * payload	data payload of a NORM packet (if any)
 * header	NORM header only
 * datagram	includes payload plus NORM header. This is the unit given to UDP
 * ip_datagram	includes datagram plus UDP/IP headers
 *
 * WARNING: changes of these sizes must be done both at source and receivers...
 */
//const INT32	 DFLT_MTU = 576;
const INT32	DFLT_MTU = 1500;

const INT32	UDP_IPv4_HEADER_SIZE = (8 + 20);	// in case of IPv4

// NORM headers cannot be smaller in any case
const INT32	MIN_NORM_HDR_SIZE = 64;
// NORM headers cannot be larger in case of a DATA NORM packet
const INT32	MAX_NORM_HDR_SIZE_FOR_DATA = 64;

const INT32	MAX_DATAGRAM_SIZE = (DFLT_MTU - UDP_IPv4_HEADER_SIZE);
const INT32	DFLT_DATAGRAM_SIZE = MAX_DATAGRAM_SIZE;

const INT32	MAX_PAYLOAD_SIZE = (MAX_DATAGRAM_SIZE -
				    MAX_NORM_HDR_SIZE_FOR_DATA);
const INT32	DFLT_PAYLOAD_SIZE = MAX_PAYLOAD_SIZE;



/**
 * Create temporary file in this dir (e.g. used by the storage service).
 */
#define MCL_DEFAULT_TMP_DIR_NAME	"/tmp/"


/**
 * Do you want that all traces (out and error) are sent to a dedicated
 * file in the temporary directory defined above or to the stdout and
 * stderr standard output?
 */
//#define STDOUT_TO_FILE


/*
 * Do you want to simulate random bursty packet losses?
 * (i.e. the source will randomly forget to send some bursts of packets)
 * Usually NO...
 */
//#define SIMUL_TX_LOSSES
//#define SIMUL_RX_LOSSES	// not yet supported

#if defined(SIMUL_TX_LOSSES) || defined(SIMUL_RX_LOSSES)
/*
 * Should losses be independant of layer number (ie be constant) or not ?
 */
#define CONSTANT_LOSS_RATIO
/*
 * number of (simulated) losses in % when the previous packet was OK
 */
#define P_LOSS_WHEN_OK	1
//#define P_LOSS_WHEN_OK		5
//#define P_LOSS_WHEN_OK	20
/*
 * number of (simulated) losses in % when the previous packet was LOST 
 */
#define P_LOSS_WHEN_LOSSES	10
//#define P_LOSS_WHEN_LOSSES	50
#endif /* SIMUL_XX_LOSSES */


#endif // MCL_PROFILE_H

