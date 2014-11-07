/* $Id: mcl_lib_api_norm.h,v 1.5 2004/08/03 06:35:56 roca Exp $ */
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

#ifndef MCL_LIB_API_NORM_H
#define MCL_LIB_API_NORM_H

/* 
 * MCL API definition, part II: the reliable multicast specific part of
 * the MCL API.
 *
 * WARNING: This file must not be included by an application, only file:
 *		src/common/mcl_lib_api.h
 * must be included!
 */

enum mcl_tx_profile {
	MCL_TX_PROFILE_LOW_RATE_INTERNET  = 0,	/* modem connection */
	MCL_TX_PROFILE_MID_RATE_INTERNET,	/* eg with VPNs, xDSL */
	MCL_TX_PROFILE_HIGH_SPEED_INTERNET,	/* several Mbps connection */
	MCL_TX_PROFILE_HIGH_SPEED_LAN		/* 100Mbps ethernet LAN */
};


/**
 * FEC schemes available.
 * Use one of them for mcl_ctl(SET_FEC_CODE).
 * A FEC scheme defines both the FEC codec and the way it is used,
 * i.e. the {FEC Encoding ID; FEC Instance ID} tuple.
 */
enum mcl_fec_scheme {
	MCL_FEC_SCHEME_NULL = 0, /* NULL code (i.e. no FEC encoding) */
	MCL_FEC_SCHEME_RSE_129_0
				/* Reed-Solomon erasure FEC code */
	//MCL_FEC_SCHEME_LDGM_STAIRCASE_132_0,
				/* Low Density Generator Matrix-STAIRCASE */
	//MCL_FEC_SCHEME_LDGM_TRIANGLE_132_1
				/* Low Density Generator Matrix-TRIANGLE */
};
#define MCL_FEC_SCHEME_NB	2	/* Number of FEC schemes available */


/* 
 * library options set with the mcl_ctl() function
 */
enum mcl_opt {
	/*
	 * common options used by both sender and receiver
	 */
	MCL_OPT_PORT,		/* port number (uni or multicast), HOST order*/
				/* argument: int (and not ushort!) */
	MCL_OPT_ADDR,		/* address (uni or multicast), HOST order */
				/* argument: int */
	MCL_OPT_SRC_ADDR,	/* (unicast) source address, HOST order, only */
				/* needed at a receiver. */
				/* argument: int */
	MCL_OPT_BIND,		/* specify sockaddr struct, NETWORK byte order*/
				/* argument: struct sockaddr_in */
	MCL_OPT_TTL,		/* specify TTL (time to live) in [0;127] range*/
				/* argument: int */
	MCL_OPT_STATS,		/* collect various stats */
				/* argument: int */
	MCL_OPT_VERBOSITY,	/* verbosity level */
				/* argument: int */
	MCL_OPT_DEBUG,		/* no tx/rx thread to simplify debug */
				/* argument: none */
	MCL_OPT_MOREABOUT,	/* version # and credits */
				/* argument: none */
	MCL_OPT_TMP_DIR,	/* put temporary files in this directory */
				/* argument: a `\0' terminated string, size */					/* given */
	/* two multicast specific socket options - rarely required... */
	// MCL_OPT_NETIF is now DEPRECATED !!! */
	MCL_OPT_SET_NETIF_ADDR,	/* specify the network interface address to */
				/* be used, NETWORK byte order. */
				/* argument: sockaddr_in or sockaddr_in6 */
	MCL_OPT_SET_NETIF_NAME,	/* specify the network interface name to */
				/* be used. */
				/* argument: string (e.g. "eth0")*/
	MCL_OPT_LOOPBACK,	/* will we send mcast packet to local apps */
				/* argument: int passed to setsockopt as is */
	/*
	 * options used by the sender
	 */
	MCL_OPT_TX_PROFILE,	/* select a pre-defined transmission profile */
				/* argument: int (see mcl_tx_profile enum) */
	MCL_OPT_DATAGRAM_SIZE,	/* datagram size */
				/* argument: int */
	MCL_OPT_TX_RATE,	/* tx rate in full-sz packets/s on base layer */
				/* argument: int */
	MCL_OPT_FEC_RATIO,	/* set the FEC packets ratio (n/k) (e.g. 2.0 */
				/* adds 100% of fec pkts, 3.0 adds 200% fec) */
				/* argument: float */
	MCL_OPT_REUSE_APPLI_TX_BUFFER, /* can MCL take control of appli buffer*/
				/* used to avoid an extra copy within MCL */
				/* Warning: buf must be alloc by c/re/malloc */
				/* argument: int (0 (default) or 1) */
	MCL_OPT_RESET_TRANSMISSIONS, /* delete and free all transmission */
				/* tables and the ADU list */
				/* no argument */
	MCL_OPT_SET_FEC_CODE,	/* set the FEC codec to be used. If not */
				/* available, an error is returned */
				/* argument: int */
	MCL_OPT_GET_MAX_BLOCK_SIZE_FOR_CURRENT_FEC, /* retrieve the maximum */
				/* block size in bytes for current FEC code */
				/* argument: int, contains the value on return*/
	/*
	 * options used by the receiver
	 */ 
	MCL_OPT_IMMEDIATE_DELIVERY_TO_APPLI, /* boolean specifying if ADUs */
				/* can be delivered in arrival order (true) */
				/* or in increasing sequence number order */
				/* (false) to the receiving application */
				/* argument: int (0 (default) or 1) */
	MCL_OPT_POSTPONE_FEC_DECODING /* boolean: is FEC decoding postponed? */
				/* argument: int, 0 or 1 (default, postponed) */
};

#endif /* MCL_LIB_API_NORM_H */

