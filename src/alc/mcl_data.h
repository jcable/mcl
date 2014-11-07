/* $Id: mcl_data.h,v 1.16 2005/04/07 15:51:38 moi Exp $ */
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

#ifndef MCL_DATA_H  /* { */
#define MCL_DATA_H

#include "mcl_vrmem.h"

/****** data structures ******/


/*
 * ALC/LCT official terminology		MCLv3 terminology
 * ----------------------------		-----------------
 * transport object			application data unit, or ADU (adu_t)
 * block				block (block_t)
 * data (or source) symbol		data unit, or DU (du_t, is_fec=0)
 * parity (or redundant) symbol		DU (du_t, is_fec=1)
 */


/*
 * Distinguish between normal priority ADUs (e.g. data), and high
 * priority ADUs (e.g. for control information, for FDT with FLUTE)
 * that will be managed separately, on a distinct transmission
 * plannification table.
 */
enum adu_priority_t {
	ADU_NORMAL_PRIO = 0,
	ADU_HIGH_PRIO
};


/**
 * Application Data Unit (ADU), AKA Transport Object.
 * They are the unit of data submitted by the sending application
 * in an mcl_send[to] call.
 * The ADU boundaries are preserved and communicated to the receivers.
 * Both sender and receivers keep a linked list of ADUs.
 */
typedef struct adu {
	struct adu	*prev, *next;
	TOI_t		seq;		/* seq nb of this ADU (AKA TOI) */
	UINT32		len;		/* number of bytes in this ADU */
	UINT32		padded_len;	/* len with optional 0 padding */
	UINT32		FDT_instanceid;	/* associated FDT instance ID if it */
					/* is an FDT (i.e. seq==0) (FLUTE) */
	/* blocking struct */
	UINT32		max_k;		/* req. to calculate blocking struct */
					/* Warning: blocks may be shorter, this
					 * is not the actual max blk length */
	UINT32		max_n;		/* req. to generate the FTI */
	struct mcl_blocking_struct blocking_struct;
					/* blocking structure for this ADU: */
					/* gives nb and length of each block */
	struct block	*block_head;	/* first block of this ADU in list */
	//UINT32	block_nb;	/* number of blocks in this ADU */
	UINT8		fec_encoding_id;/* FEC enc. id used for this ADU */
	UINT8		fec_instance_id;/* FEC inst. id used for this ADU */
	mcl_fec_scheme	fec_scheme;	/* FEC scheme used for this ADU */
	UINT16		symbol_len;	/* full-sized symbol size in bytes */
					/* valid for all DUs of this ADU */
	class mcl_addr	addr;		/* ADU received from or destined to */
	bool		addr_valid;	/* boolean: true if the addr class is */
					/* valid, ie has been initialized */
	/* fields only used by the sender */
	char		*data;		/* ptr to data buffer */
	char		in_txtab;	/* boolean: 1 if there is at least one*/
       					/* du waiting to be sent in txtab */
	adu_priority_t	priority;	/* 1 high importance (e.g. control) */
					/* 0 normal importance (default) */
#if defined(VIRTUAL_TX_MEM) || defined(VIRTUAL_RX_MEM)
	union {
#ifdef VIRTUAL_TX_MEM
		/* field only used by the sender */
		struct vtmem_info	vtm_info_u;	/* adu/vtm correspondance */
#endif
#if 0
//#ifdef VIRTUAL_RX_MEM
		/* field only used by the receiver */
		struct vrmem_info	vrm_info_u;	/* adu/vrm correspondance */
#endif
	} vxm;
#endif /* VIRTUAL_TX_MEM | VIRTUAL_RX_MEM */
	/* fields only used by the receivers */
	char		ia_adu;		/* boolean: 1 if implicitly announced */
	UINT32		recvd_src_data;	/* number of bytes received or decoded*/
					/* Does not include parity symbols. */
	enum mcl_adu_rx_status rx_status; /* rx status: completed/delivered/..*/
} adu_t;


/**
 * Block.
 * They are the result of the segmentation of ADUs into several
 * blocks, each of size appropriate to the FEC codec in use.
 * The algorithm used for this segmentation can be found in
 * common/mcl_blocking_struct.cpp
 */
typedef struct block {
	struct adu	*adu;		/* ADU to which this block belongs */
	TOI_t		seq;		/* block sequence number */
	UINT32		len;		/* number of bytes in this block */
	struct du	*du_head;	/* first DU of this block in list */
	UINT32		k;		/* nb of DUs for this block (k param) */
	UINT32		n;		/* n parameter for this block */
#ifdef FEC
	struct du	*fec_du_head;	/* first FEC DU of this block in list */
	UINT32		fec_du_nb_in_list; /* nb of FEC DUs in this block */
					/* at a source, it's the n-k param. */
#ifdef LDPC_FEC
	UINT16		fec_key;	/* LDGM/LDPC specific: random seed */
#endif /* LDPC_FEC */
#endif
	/* fields only used by the receivers */
	UINT32		du_nb_in_list;	/* nb of non-FEC DU recvd (!= k) */
					/* or decoded (in case of LDPC) */
	char		rx_status;	/* rx status (completed/decoded/..) */
#ifdef LDPC_FEC
	class LDPCFecSession *ldpc_ses;	/* LDPC context, used by decoder */
	void		**pkt_canvas;	/* ptrs to pkt table, used by decoder */
	struct ldpc_callback_context*	callback_ctxt;
					/* carries context info required by */
					/* the LDPC callback function */
#endif /* LDPC_FEC */
} block_t;

/* possible values of the du->rx_status field */
#define BLK_STATUS_NIL		0	/* void status */
#define BLK_STATUS_IN_PROGRESS	1	/* not yet ready */
#define BLK_STATUS_COMPLETED	2	/* received all DUs from all blocks */
#define BLK_STATUS_DECODED	3	/* COMPLETED and FEC decoding done */


/**
 * Data Unit (DU).
 * They are the result of the FEC encoding of a block (i.e. ADU chunck)
 * and identify a sequence of bytes.
 * They can either contain plain data or redundant FEC data.
 * They are the data payload of the packets sent.
 * Used both by the sending and receiving sides.
 */
typedef struct du {
	struct du	*prev, *next;	/* links used only by the receiver! */
	struct block	*block;		/* block to which this DU belongs */
	char		*data;		/* ptr to data in buffer */
	UINT32		seq;		/* sequence number of this DU */
	UINT16		len;		/* number of bytes in this DU */
#ifdef FEC
	bool		is_fec;		/* true if this is a FEC DU */
#endif
	/* rx specific field */
	class mcl_rx_pkt *pkt;		/* object containing the packet recvd */
#if 0
#if defined(VIRTUAL_TX_MEM) || defined(VIRTUAL_RX_MEM)
	struct vv {
#ifdef VIRTUAL_TX_MEM
		/* field only used by the sender */
		struct vtmem_info	vtm_info_u;/* adu/vtm correspondance */
#endif
#ifdef VIRTUAL_RX_MEM
		/* field only used by the receiver */
		struct vrmem_info	vrm_info_u;/* adu/vrm correspondance */
#endif
	} vxm;
#endif /* VIRTUAL_TX_MEM | VIRTUAL_RX_MEM */
#else
#ifdef VIRTUAL_RX_MEM
	/* field only used by the receiver */
	struct vrmem_info	vrm_info;/* adu/vrm correspondance */
#endif
#endif
} du_t;


#if 0
#if defined(VIRTUAL_TX_MEM) || defined(VIRTUAL_RX_MEM)
/* easy access for the adu_t and du_t union members */
#define	vtm_info	vxm.vtm_info_u
#define	vrm_info	vxm.vrm_info_u
#endif /* VIRTUAL_TX_MEM | VIRTUAL_RX_MEM */
#endif


/* 
 * Signaling list used by the sender to prepare ALC extension headers
 */
typedef struct sig_tab {
	struct sig_tab 	*next;
	int		eh_type;	/* EXT_CCI, ... SIG_NONEWADU, ... */
	char		*buf;		/* complementary info, or full ALC EH */
	int		len;		/* future ALC EH has this len (bytes) */
	int		target_lvl;	/* to send on this layer, any if -1 */
	int		rem2send;	/* nb of times it can still be sent */
	/*struct sockaddr *saddr;*/	/* destination, otherwise default lvl */
	class mcl_addr	*saddr;		/* destination, otherwise default lvl */
	int 		saddr_len;	/* and associated len */
} sig_tab_t;

#endif /* }  MCL_DATA_H */
