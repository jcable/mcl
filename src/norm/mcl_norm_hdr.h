/* $Id: mcl_norm_hdr.h,v 1.2 2004/01/30 16:27:42 roca Exp $ */
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

#ifndef MCL_NORM_HDR_H
#define MCL_NORM_HDR_H

#define FEC_ENCODING_ID		129	/* FEC encoding identifier */
					/* 128: small/large/expandable codes */
					/* 129: small block systematic codes */
#define FEC_ENCODING_NAME	0	/* for Reed-Solomon (TBC) */

/* define to check packet integrity */
#define MAGIC_VERSION	
#ifdef MAGIC_VERSION
const UINT8	NORM_VERSION = 0xBB;
#else
const UINT8	NORM_VERSION = 0;
#endif


/**
 * The various NORM message types
 */
enum norm_msg_type {
	NORM_INVALID	= 0,
	NORM_INFO	= 1,
	NORM_DATA	= 2,
	NORM_CMD	= 3,
	NORM_NACK	= 4,
	NORM_ACK	= 5,
	NORM_REPORT	= 6
};


/**
 * Common part of all NORM messages
 */
typedef struct {
	UINT8	version;
	UINT8	type;
	UINT16	seq;
	UINT32	source_id;
} norm_common_hdr_t;


/**
 * NORM_DATA header
 */
typedef	struct {
	UINT8	flags;
	UINT8	grtt;
	UINT8	gsize;
	UINT8	fec_id;

	UINT16	seg_size;		// max payload length for this block
	UINT16	obj_size_msb;

	UINT32	obj_size_lsb;

#if 0
	UINT16	reserved;		// NON-NORM-COMPLIANT: removed
#endif
	UINT16	fec_encoding_name;

	UINT16	fec_num_parity;
#if 0
	UINT16	fec_block_len;		// # of data symbols in current block
#else
	UINT32	fec_block_len;		// NON-NORM-COMPLIANT: moved to 32 bits
					// and changed meaning. Now contains the
					// size in bytes of a full length block
#endif

	UINT16	obj_transp_id;		// truncated object idf
	UINT16	fec_symbol_id;		// symbol idf (with small block FEC)
					// NB: permutted for simplicity

	UINT32	fec_block_nb;		// block idf
					// NB: permutted for simplicity

	UINT16	payload_len;		// actual length of this symbol
	UINT16	offset_msb;

	UINT32	offset_lsb;
} norm_data_hdr_t;


/**
 * NORM_INFO header
 */
typedef struct {
	UINT8	flags;
	UINT8	grtt;
	UINT8	gsize;
	UINT8	fec_id;

	UINT16	seg_size;
	UINT16	obj_size_msb;

	UINT32	obj_size_lsb;

	UINT16	fec_encoding_name;
	UINT16	fec_num_parity;

	UINT16	fec_block_len;
	UINT16	obj_transp_id;
} norm_info_hdr_t;


/**
 * NORM_CMD flavor definitions
 */
enum norm_cmd_flavor {
	NORM_CMD_INVALID	= 0,
	NORM_CMD_FLUSH		= 1,
	NORM_CMD_SQUELCH	= 2,
	NORM_CMD_ACK_REQ	= 3,
	NORM_CMD_REPAIR_ADV	= 4,
	NORM_CMD_CC		= 5,
	NORM_CMD_APPLICATION	= 6,
	NORM_CMD_NO_NEW_OBJECT	= 7,	// NON-NORM-COMPLIANT command
	NORM_CMD_CLOSE		= 8	// NON-NORM-COMPLIANT command
};


/**
 * NORM_CMD common header
 */
typedef struct {
	UINT8	grtt;
	UINT8	gsize;
	UINT8	flavor;
	UINT8	reserved;
} norm_cmd_common_hdr_t;


/**
 * NORM_CMD(FLUSH) header
 */
const UINT8	NORM_CMD_FLUSH_FLAG_EOT = 0x01;

typedef struct {
	UINT8	grtt;
	UINT8	gsize;
	UINT8	flavor;
	UINT8	flags;

	UINT16	obj_transp_id;
	UINT16	fec_symbol_id;		// NB: permutted for simplicity

	UINT32	fec_block_nb;		// NB: permutted for simplicity
} norm_cmd_flush_hdr_t;


/**
 * NORM_CMD(SQUELCH) header
 */
typedef struct {
} norm_cmd_squelch_hdr_t;


/**
 * NORM_CMD(ACK_REQ) header
 */
typedef struct {
} norm_cmd_ack_req_hdr_t;


/**
 * NORM_CMD(ACK_REQ(WATERMARK)) header
 */
typedef struct {
} norm_cmd_watermark_hdr_t;


/**
 * NORM_CMD(ACK_REQ(RTT)) header
 */
typedef struct {
} norm_cmd_ack_req_rtt_hdr_t;


/**
 * NORM_CMD(REPAIR_ADV) header
 */
typedef struct {
	UINT8	grtt;
	UINT8	gsize;
	UINT8	flavor;
	UINT8	flags;
} norm_cmd_repair_adv_hdr_t;


/**
 * NORM_CMD(CC) header
 */
typedef struct {
} norm_cmd_cc_hdr_t;


/**
 * NORM_CMD(APPLICATION) header
 */
typedef struct {
} norm_cmd_application_hdr_t;


/* start of NON-NORM-COMPLIANT extensions { */

/**
 * NORM_CMD(NO_NEW_OBJECT) header
 * Informs receivers of all the objects that are being sent in this session
 * by this sender.
 * NON-NORM-COMPLIANT command
 */
typedef struct {
	UINT8	grtt;
	UINT8	gsize;
	UINT8	flavor;			// NORM_CMD_NO_NEW_OBJECT
	UINT8	reserved;		// must be 0'ed

	UINT32	max_object_id;
} norm_cmd_no_new_object_hdr_t;

/**
 * NORM_CMD(CLOSE) header
 * Informs receivers that this sender leaves immediately the NORM session.
 * This is a non-gracefull close.
 * NON-NORM-COMPLIANT command
 */
typedef struct {
	UINT8	grtt;
	UINT8	gsize;
	UINT8	flavor;			// NORM_CMD_CLOSE
	UINT8	reserved;		// must be 0'ed
} norm_cmd_close_hdr_t;


/* } end of NON-NORM-COMPLIANT extensions */


/****** NACK stuff ******/


/**
 * NORM_NACK header
 */
typedef struct {
	UINT32	server_id;

	UINT32	grtt_response_sec;

	UINT32	grtt_response_usec;

	UINT16	loss_estimate;
	UINT16	grtt_req_seq;
} norm_nack_hdr_t;


/**
 * NORM_NACK content can have one of the following forms
 */
enum norm_nack_form {
	NORM_NACK_ITEMS = 1,
	NORM_NACK_RANGES,
	NORM_NACK_ERASURES
};

/**
 * NORM_NACK flags
 */
enum norm_nack_flags {
	NORM_NACK_SEGMENT = 0x01,
	NORM_NACK_BLOCK = 0x02,
	NORM_NACK_INFO = 0x04,
	NORM_NACK_OBJECT = 0x08
};


/**
 * NORM_NACK content header
 */
typedef struct {
	UINT8	form;
	UINT8	flags;
	UINT16	length;

	UINT16	obj_transp_id;
	UINT16	fec_symbol_id_or_erasure_count;	// NB: permutted for simplicity

	UINT32	fec_block_nb;		// NB: permutted for simplicity
} norm_nack_content_hdr_t;


/****** end of NACK stuff ******/


/**
 * NORM_ACK common header
 */
typedef struct {
	UINT32	server_id;

	UINT32	grtt_response_sec;

	UINT32	grtt_response_usec;

	UINT16	loss_estimate;
	UINT16	grtt_req_seq;

	UINT8	ack_flavor;
	UINT8	reserved1;
	UINT16	reserved2;
} norm_ack_hdr_t;


/**
 * NORM_ACK(WATERMARK) ACK content header
 */
typedef struct {
	UINT16	obj_transp_id;
	UINT16	fec_symbol_id;		// NB: permutted for simplicity

	UINT32	fec_block_nb;		// NB: permutted for simplicity
} norm_ack_watermark_hdr_t;


#endif // MCL_NORM_HDR_H
