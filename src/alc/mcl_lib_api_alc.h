/* $Id: mcl_lib_api_alc.h,v 1.38 2005/05/23 14:35:53 roca Exp $ */
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

#ifndef MCL_LIB_API_ALC_H  /* { */
#define MCL_LIB_API_ALC_H

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
	MCL_TX_PROFILE_HIGH_SPEED_LAN,		/* fast/Gbps ethernet LAN */
};


/**
 * List of events available for mcl_wait_event().
 */
#define	MCL_WAIT_EVENT_END_TX	0
#define	MCL_WAIT_EVENT_END_RX	1
#define	MCL_WAIT_EVENT_CLOSED	2


/**
 * FEC schemes available.
 * Use one of them for mcl_ctl(SET_FEC_CODE).
 * A FEC scheme defines both the FEC codec and the way it is used,
 * i.e. the {FEC Encoding ID; FEC Instance ID} tuple.
 */
enum mcl_fec_scheme {
	MCL_FEC_SCHEME_NULL = 0, /* NULL code (i.e. no FEC encoding) */
	MCL_FEC_SCHEME_RSE_129_0,
				/* Reed-Solomon erasure FEC code */
	MCL_FEC_SCHEME_LDGM_STAIRCASE_132_0,
				/* Low Density Generator Matrix-STAIRCASE */
	MCL_FEC_SCHEME_LDGM_TRIANGLE_132_1
				/* Low Density Generator Matrix-TRIANGLE */
};
#define MCL_FEC_SCHEME_NB	4	/* Number of FEC schemes available */


/**
 * Possible congestion controls protocols.
 */
enum mcl_congestion_control_scheme {
	INVALID_CC = 0,		/* initial value, nothing choosen */
	NO_CC,			/* choose not to have any CC */
	RLC_CC,			/* choose to have RLC CC */
	FLID_SL_CC		/* choose to have FLID SL (static layer) CC */
};


/**
 * ADU reception status.
 */
/** possible values of the adu->rx_status field. */
enum mcl_adu_rx_status {
	ADU_STATUS_NIL = 0,	/* void status */
	ADU_STATUS_IN_PROGRESS,	/* not yet ready */
	ADU_STATUS_COMPLETED,	/* received enough DUs for all blocks */
	ADU_STATUS_DECODED,	/* COMPLETED and FEC decoding done */
	ADU_STATUS_DELIVERED	/* sent to the receiving application */
};


/** structure used by the MCL_OPT_GET_ADU_RX_INFO option. */
typedef struct mcl_adu_rx_info {
	UINT64	toi;		/* Transport Objet Identifier of the ADU */
	UINT64	len;		/* ADU length */
	UINT64	recvd_src_data;	/* number of bytes received or decoded. */
				/* Does not include parity symbols. */
	enum mcl_adu_rx_status rx_status; /* rx status: completed,delivered...*/
} mcl_adu_rx_info_t;


/**
 * structure used to communicate FTI information from outside
 * mcl to the lib, used by the MCL_OPT_SET_FTI_INFO option.
 */
typedef struct FTI_infos {
	UINT64	toi;		/* Transport Objet Identifier of the ADU */
	UINT32	fec_encoding_id;/* FEC encoding ID (RFC3452) */
	UINT16	fec_instance_id;/* FEC instance ID (RFC3452) */
	UINT32	fec_key;	/* LDGM/LDPC specific: random seed */
	UINT64 	adu_len;	/* ADU length in bytes */
	UINT32 	max_k;		/* max src blk len in nb of symbols */
	UINT32	max_n;		/* max enc blk len in nb of symbols */
	UINT16 	symbol_len;	/* full-sized symbol length in bytes */
} FTI_infos_t;



/* 
 * library options set with the mcl_ctl() function
 */
enum mcl_opt {
	/**********************************************************************/
	/*
	 * common options used by both sender and receiver
	 */
	MCL_OPT_LAYER_NB = 1,	/* number of transmission layers at a sender, */
				/* and maximum number of layer at a receiver. */
				/* if more than 1 is specified, the actual */
				/* number of layers at a receiver will be */
				/* determined by the congestion control */
				/* protocol dynamically. */
				/* argument: int */
	MCL_OPT_PORT,		/* port number (uni or multicast), HOST order */
				/* argument: int (and not UINT16!) */
	MCL_OPT_ADDR,		/* IPv4 address (uni or multicast), HOST order*/
				/* DEPRECATED, use BIND */
				/* argument: int */
	MCL_OPT_BIND,		/* specify sockaddr struct, NETWORK byte order*/
				/* argument: sockaddr_in or sockaddr_in6 */
	MCL_OPT_TTL,		/* specify TTL (time to live) in [0;127] range*/
				/* argument: int */
	MCL_OPT_DEMUX_LABEL,	/* specify the LCT demultiplexing label, TSI */
				/* (Transport Session Idf) */
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
#define DEL_MODE_PUSH		0	/* tx once, assumes synchronized rx */
#define DEL_MODE_ON_DEMAND	1	/* tx cyclically, rx start at any time*/
#define DEL_MODE_STREAMING	2	/* streaming mode */
	MCL_OPT_DELIVERY_MODE,	/* specify the delivery mode: push/on-demand */
				/* (cf. i-draft) argument: int */
	/* two multicast specific socket options - rarely required... */
	// MCL_OPT_NETIF is now DEPRECATED !!! */
	MCL_OPT_SET_NETIF_ADDR,	/* specify the network interface address to */
				/* be used, NETWORK byte order. */
				/* argument: sockaddr_in or sockaddr_in6 */
	MCL_OPT_SET_NETIF_NAME,	/* specify the network interface name to */
				/* be used. */
				/* argument: string (e.g. "eth0"). The optlen */
				/* must contain the string len given by strlen*/
	MCL_OPT_LOOPBACK,	/* will we send mcast packet to local apps */
				/* argument: int passed to setsockopt as is */

	/*
	 * FLUTE specific common options
	 */
	MCL_OPT_SET_FLUTE_MODE,	/* Set the FLUTE mode (false by default). */
				/* no argument */
	/*
	 * METAOBJECT specific common options
	 */
	MCL_OPT_SET_META_OBJECT_ONLY_MODE, /* Set the meta_object only mode */
				/* i.e. all objects sent or received are */
				/* metaobjects (default: false). */
				/* no argument */

	/**********************************************************************/
	/*
	 * options used by the sender
	 */
	MCL_OPT_SET_NEXT_TOI,	/* set the Transport Object Id (AKA ADU id) */
				/* to use for the next object submitted */
				/* argument: UINT32 */
	MCL_OPT_SET_NEXT_ADU_HIGH_IMPORTANCE, 	/* next ADU is of high */
				/* priority (e.g. control data, FDT...) */
				/* Must be called before each high prio obj. */
				/* no argument */
	MCL_OPT_FLUSH_HIGH_IMPORTANCE_OBJECTS, 	/* send all high prio objects */
				/* immediatly. */
				/* The high prio objects will continue to be */
				/* sent after the flush, but at normal high */
				/* priority rate */
				/* no argument */
	MCL_OPT_TX_PROFILE,	/* select a pre-defined transmission profile */
				/* argument: int (see mcl_tx_profile enum) */
	MCL_OPT_PRINT_TX_PROFILE,/* print the transmission profile */
				/* argument: none */
	MCL_OPT_DATAGRAM_SIZE,	/* datagram size */
				/* argument: int */
	MCL_OPT_TX_RATE,	/* tx rate in full-size packets/s on base */
				/* layer */
				/* argument: double */
	MCL_OPT_SET_CC_SCHEME,	/* specify the congestion control mode  */
				/* argument: mcl_congestion_control_scheme */
	MCL_OPT_FEC_RATIO,	/* set the FEC packets ratio (n/k) (e.g. 2.0 */
				/* adds 100% of fec pkts, 3.0 adds 200% fec) */
				/* argument: float (must be >= 1.0) */
	MCL_OPT_NB_OF_TX,	/* send DUs that number of times on base layer*/
				/* argument: int (default 1 or infinity if */
       				/* in DEL_MODE_ON_DEMAND mode) */
	MCL_OPT_ADD_NB_OF_TX,	/* extends the number of times a DU is sent */
				/* on base layer to this number of cycles. */
				/* argument: int */				
	MCL_OPT_REUSE_APPLI_TX_BUFFER, /* can MCL take control of appli buffer*/
				/* used to avoid an extra copy within MCL */
				/* Warning: buf must be alloc by c/re/malloc */
				/* argument: int (0 (default) or 1) */
	MCL_OPT_VIRTUAL_TX_MEMORY, /* do you need the virtual tx mem service? */
				/* argmument: int (0 or 1 (default)) */
	MCL_OPT_VIRTUAL_RX_MEMORY, /* do you need the virtual rx mem service? */
				/* argmument: int (0 or 1 (default)) */
	MCL_OPT_KEEP_DATA,	/* step1: wait before doing ADU scheduling */
				/* no argument */
	MCL_OPT_PUSH_DATA,	/* step2: now perform ADU scheduling and send */
				/* no argument */
	MCL_OPT_KEEP_META_OBJECT,/* step1: start of a new meta object */
				/* no argument */
	MCL_OPT_PUSH_META_OBJECT,/* step2: end of current meta object*/
				/* no argument */
	MCL_OPT_RESET_TRANSMISSIONS, /* delete and free all transmission */
				/* tables and the associated ADUs */
				/* WARNING: this control deletes ALL ADUs */
				/* that are in the transmission table, so take*/
				/* care if you use anticipated_tx_for_push */
				/* since it will also delete anticipated ADUs.*/
				/* no argument */
	MCL_OPT_STOP_TRANSMITTING_ADU, /* removes the specified ADU from the */
				/* transmission tables, and frees the ADU */
				/* argument: UINT64, TOI of the ADU */
#ifdef SVSOA_RECV
	MCL_OPT_SET_RX_LEVEL, 	/* set the reception level at session start*/
				/* argument: int (number of layers)*/
	MCL_OPT_GET_RX_LEVEL, 	/* return reception level*/
				/* argument: int, contains the value on return*/
	MCL_USED_FOR_BASE_VIDEO_LAYER,/* this session is used for base video */
				/* layer. argument: none */
#endif
				
#define MCL_SCHED_LCT1		0	/* see mcl_profile.h for descr. */
#define MCL_SCHED_NB		1	/* nb of schedulers defined */
	MCL_OPT_SCHED,		/* choose a scheduler from above possibilities*/
				/* argument: int */
#define MCL_SCHED_SEQUENTIAL_OBJ_ORDER	0 /* tx ADUs in sequence */
#define MCL_SCHED_RANDOM_OBJ_ORDER	1 /* tx ADUs in random order in each */
					  /* layer */
#define MCL_SCHED_PARTIALLY_MIXED_ORDER	2 /* mix partially all DUs of all ADUs*/
#define MCL_SCHED_MIXED_ORDER		3 /* mix all DUs of all ADUs */
	MCL_OPT_OBJ_SCHED,	/* choose an object scheduler */
				/* argument: int */
	MCL_OPT_SET_FEC_CODE,	/* set the FEC codec to be used. If not */
				/* available, an error is returned */
				/* argument: int */
	MCL_OPT_GET_MAX_BLOCK_SIZE_FOR_CURRENT_FEC, /* retrieve the maximum */
				/* block size in bytes for current FEC code */
				/* argument: int, contains the value on return*/

	/**********************************************************************/
	/*
	 * options used by the receiver
	 */ 
	MCL_OPT_SET_SSM,	/* set the SSM (Source Specific Multicast) */
				/* mode at the receiver. In that case, the */
				/* MCL_OPT_SRC_ADDR is mandatory */
				/* argument: int (0 (default) or 1) */
	MCL_OPT_SRC_ADDR,	/* (unicast) source address, HOST order, only */
				/* needed at a receiver. */
				/* argument: sockaddr_in or sockaddr_in6 */
	MCL_OPT_IMMEDIATE_DELIVERY_TO_APPLI, /* boolean specifying if ADU can */
				/* be delivered in arrival order or not */
				/* (i.e. in sequence). */
				/* Compatible with FLUTE delivery) */
				/* argument: int (0 (default) or 1) */
	MCL_OPT_POSTPONE_FEC_DECODING, /* boolean: is FEC decoding postponed? */
				/* only valid with RSE, doesn't apply to LDPC */
				/* argument: int, 0 or 1 (default, postponed) */
	MCL_OPT_NEVER_LEAVE_BASE_LAYER, /* mcl_wait(MCL_WAIT_EVENT_CLOSED) */
				/* needs it; argument: int (0 (default) or 1) */
	MCL_OPT_GET_ADU_RX_INFO, /* Get reception information for this ADU. */
				/* argument: pointer to mcl_adu_rx_info_t */
	MCL_OPT_SET_FTI_INFO, 	/* Set the FTI information */
				/* argument: pointer to FTI_infos_t */

	/*
	 * FLUTE specific options at a receiver
	 */
	MCL_OPT_SET_FLUTE_STORE_ALL_ADUS_BY_DEFAULT,
				/* If set, all received ADUs will be stored */
				/* in MCL, no matter whether or not they have */
				/* been requested by FLUTE. When set, this */
				/* mode improves delivery efficiency, since */
				/* it is not required to wait for a */
				/* DELIVER_THIS_ADU_TO_APPLI to start ADU rx. */
				/* WARNING: if set and if an FDT Entry has */
				/* been missed the ADU will remain in the MCL */
				/* lib till the session closes! */
				/* Requires that SET_FLUTE_MODE has been */
				/* issued previously. */
				/* argument: int (0 (default) or 1) */
	MCL_OPT_FLUTE_DELIVER_THIS_ADU_TO_APPLI,
				/* Specifies a TOI (AKA. ADU Id) to deliver. */
				/* TOIs (!=0) that have not been enabled with */
				/* a FLUTE_DELIVER_THIS_ADU_TO_APPLI won't be */
				/* delivered to FLUTE. */
				/* Requires that SET_FLUTE_MODE has been */
				/* issued previously. */
				/* argument: int (TOI of the desired ADU) */

	/**********************************************************************/
	/*
	 * options used by rlc_ctl() for RLC congestion control init
	 * (rarely required... use default values)
	 */
	RLC_OPT_SP_CYCLE,	/* interval between 2 SPs at layer 0 */
				/* (in microseconds) */
				/* argument: int */
	RLC_OPT_PKT_TIMEOUT,	/* Time To Wait for a late packet to arrive */
				/* before assuming it's lost */
				/* argument: int */
	RLC_OPT_DEAF_PERIOD,	/* Deaf period after dropping a layer */
				/* argument: int */
	RLC_OPT_LATE_ACCEPTED,	/* number of late packets accepted between */
				/* 2 SPs at highest layer */
				/* argument: int */
	RLC_OPT_LOSS_ACCEPTED,	/* number of lost packets accepted between */
				/* 2 SPs at highest layer */
				/* argument: int */
	RLC_OPT_LOSS_LIMIT,	/* loss limit before layer drop */
				/* argument: int */
	RLC_OPT_LOSS_TIMEOUT,	/* timeout for loss limit */
				/* argument: int */
	RLC_OPT_AGGRESSIVE_CC,	/* boolean: 1 for an aggressive RLC profile, */
				/* more tolerant to packet losses than default*/
				/* argument: int (0 or 1) */

	/**********************************************************************/
	/*
	 * options used by FLIDs_ctl() for FLID-SL congestion control init
	 * (rarely required... use default values)
	 */
	FLIDS_OPT_TSD,		/* TimeSlot Duration (in microseconds) */
				/* argument: int */

	FLIDS_OPT_DEAF_PERIOD	/* Duration of Deaf period in multiple of TSD */
				/* argument: int */
};

#endif /* } MCL_LIB_API_ALC */
