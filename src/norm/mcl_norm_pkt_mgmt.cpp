/* $Id: mcl_norm_pkt_mgmt.cpp,v 1.2 2004/01/30 16:27:42 roca Exp $ */
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

#include "mcl_includes.h"


/**
 * Default constructor.
 */
mcl_norm_pkt_mgmt::mcl_norm_pkt_mgmt ()
{
}


/**
 * Default destructor.
 */
mcl_norm_pkt_mgmt::~mcl_norm_pkt_mgmt ()
{
}


/****** sender specific sending methods ***************************************/


/**
 * Macro to fill in the NORM common header.
 * Assumes the presence of variables:
 *	mcl_cb			*mclcb;
 *	norm_common_hdr_t	*c_hdr;
 */
#define FILL_NORM_COMMON_HDR(t)						\
        c_hdr = (norm_common_hdr_t*)(this->hdr_buffer);			\
        c_hdr->version = NORM_VERSION;					\
        c_hdr->type = t;						\
        c_hdr->seq = htons(mclcb->tx.get_next_norm_pkt_seq());		\
        c_hdr->source_id = htonl(mclcb->node.get_id());


/**
 * Creates a data packet, allocating and initializing the NORM header.
 * Used by the sender.
 * Only the header is initialized, the data buffer is left untouched.
 * The buffer containing the header is statically assigned, and will
 * be reused at the next call to this member function, so please use
 * it immediately...
 * @param mclcb
 * @param du	the data unit to send
 * @param hdr	pointer to header pointer updated when leaving this function
 * @return	the header length or <= 0 in case of error
 */
INT32
mcl_norm_pkt_mgmt::create_data_hdr (mcl_cb		*const mclcb,
				    mcl_du		*const du,
				    norm_common_hdr_t	**hdr)
{
	norm_common_hdr_t	*c_hdr;	// common header part
	norm_data_hdr_t		*d_hdr;	// NORM_DATA header part
	mcl_block		*blk;
	mcl_adu			*adu;

	TRACELVL(5, (mcl_stdout, "-> mcl_norm_pkt_mgmt::create_data_hdr:\n"))
	/*
	 * common part
	 */
	FILL_NORM_COMMON_HDR(NORM_DATA);
	/*
	 * specific part
	 */
	ASSERT(du);
	blk = du->block;
	ASSERT(blk);
	adu = blk->adu;
	ASSERT(adu);

	d_hdr = (norm_data_hdr_t*)(c_hdr + 1);

	d_hdr->flags = 0;
	d_hdr->grtt = 0;
	d_hdr->gsize = 0;
	d_hdr->fec_id = FEC_ENCODING_ID;

	d_hdr->seg_size = htons(du->len);
	d_hdr->obj_size_msb = htons(0);

	d_hdr->obj_size_lsb = htonl(adu->get_len());

#if 0
	d_hdr->reserved = htons(0);
#else
	/* NON-NORM-COMPLIANT: removed */
#endif
	d_hdr->fec_encoding_name = htons(FEC_ENCODING_NAME);

	/* max number of FEC that can be generated for this block */
	d_hdr->fec_num_parity = htons(mclcb->fec.get_n() - mclcb->fec.get_k());
#if 0
	/* actual nb of DUs for this block (may be < k for last blk) */
	d_hdr->fec_block_len = htons((UINT16)(blk->du_nb));
#else
	/* NON-NORM-COMPLIANT: moved to 32 bits and changed meaning */
	d_hdr->fec_block_len = htonl((UINT32)(adu->get_full_size_block_len()));
#endif

	d_hdr->obj_transp_id = htons((UINT16)(adu->get_seq()));
#if 0
	d_hdr->fec_symbol_id = htons((UINT16)(du->seq));
#else
	/* NON-NORM-COMPLIANT: add a FEC flag */
	d_hdr->fec_symbol_id = htons((UINT16)(du->seq | (du->is_fec ? 0x8000 : 0)));
#endif

	d_hdr->fec_block_nb = htonl((UINT32)(blk->seq));

	d_hdr->payload_len = htons(du->len);
	d_hdr->offset_msb = htons(0);

	d_hdr->offset_lsb = htons(0);	// NOT USED...

	*hdr = c_hdr;
	TRACELVL(5, (mcl_stdout, "<- mcl_norm_pkt_mgmt::create_data_hdr:\n"))
	return (sizeof(norm_common_hdr_t) + sizeof(norm_data_hdr_t));
}


/**
 * Create a NORM_CMD(FLUSH) header.
 * The buffer containing the header is statically assigned, and will
 * be reused at the next call to this member function, so please use
 * it immediately...
 * @return	the header length or <= 0 in case of error
 */
INT32
mcl_norm_pkt_mgmt::create_cmd_flush_hdr (mcl_cb			*const mclcb,
					 norm_common_hdr_t	**hdr,
					 mcl_du			*du)
{
	norm_common_hdr_t	*c_hdr;		// common header part
	norm_cmd_flush_hdr_t	*f_hdr;		// NORM_CMD_FLUSH header
	mcl_block		*blk;
	mcl_adu			*adu;

	TRACELVL(5, (mcl_stdout, "-> mcl_norm_pkt_mgmt::create_cmd_flush_hdr:\n"))
	/*
	 * common part
	 */
	FILL_NORM_COMMON_HDR(NORM_CMD);
	/*
	 * specific part
	 */
	ASSERT(du);
	blk = du->block;
	ASSERT(blk);
	adu = blk->adu;
	ASSERT(adu);

	f_hdr = (norm_cmd_flush_hdr_t*)(c_hdr + 1);
	f_hdr->grtt = 0;
	f_hdr->gsize = 0;
	f_hdr->flavor = NORM_CMD_FLUSH;
	f_hdr->flags = NORM_CMD_FLUSH_FLAG_EOT;

	f_hdr->obj_transp_id = htons((UINT16)(adu->get_seq()));
#if 0
	f_hdr->fec_symbol_id = htons((UINT16)(du->seq));
#else
	/* NON-NORM-COMPLIANT: add a FEC flag */
	f_hdr->fec_symbol_id = htons((UINT16)(du->seq | (du->is_fec ? 0x8000 : 0)));
#endif

	f_hdr->fec_block_nb = htonl((UINT32)(blk->seq));

	*hdr = c_hdr;
	TRACELVL(5, (mcl_stdout, "<- mcl_norm_pkt_mgmt::create_cmd_flush_hdr:\n"))
	return (sizeof(norm_common_hdr_t) + sizeof(norm_cmd_flush_hdr_t));
}


/**
 * Create a NORM_CMD(NO_NEW_OBJECT) header.
 * The buffer containing the header is statically assigned, and will
 * be reused at the next call to this member function, so please use
 * it immediately...
 * @return	the header length or <= 0 in case of error
 */
INT32
mcl_norm_pkt_mgmt::create_cmd_no_new_object_hdr (mcl_cb		*const mclcb,
						 norm_common_hdr_t	**hdr)
{
	norm_common_hdr_t	*c_hdr;		// common header part
	norm_cmd_no_new_object_hdr_t *nno_hdr;	// NORM_CMD_NO_NEW_OBJECT header

	TRACELVL(5, (mcl_stdout, "-> mcl_norm_pkt_mgmt::create_cmd_no_new_object_hdr:\n"))
	/*
	 * common part
	 */
	FILL_NORM_COMMON_HDR(NORM_CMD);
	/*
	 * specific part
	 */
	nno_hdr = (norm_cmd_no_new_object_hdr_t*)(c_hdr + 1);
	nno_hdr->grtt = 0;
	nno_hdr->gsize = 0;
	nno_hdr->flavor = NORM_CMD_NO_NEW_OBJECT;
	nno_hdr->reserved = 0;

	nno_hdr->max_object_id = htonl(mclcb->tx.get_highest_submitted_adu_seq());

	*hdr = c_hdr;
	TRACELVL(5, (mcl_stdout, "<- mcl_norm_pkt_mgmt::create_cmd_no_new_object_hdr:\n"))
	return (sizeof(norm_common_hdr_t) + sizeof(norm_cmd_no_new_object_hdr_t));
}


/**
 * Create a NORM_CMD(CLOSE) header.
 * The buffer containing the header is statically assigned, and will
 * be reused at the next call to this member function, so please use
 * it immediately...
 * @return	the header length or <= 0 in case of error
 */
INT32
mcl_norm_pkt_mgmt::create_cmd_close_hdr (mcl_cb			*const mclcb,
					 norm_common_hdr_t	**hdr)
{
	norm_common_hdr_t	*c_hdr;		// common header part
	norm_cmd_close_hdr_t	*close_hdr;	// NORM_CMD_CLOSE header part

	TRACELVL(5, (mcl_stdout, "-> mcl_norm_pkt_mgmt::create_cmd_close_hdr:\n"))
	/*
	 * common part
	 */
	FILL_NORM_COMMON_HDR(NORM_CMD);
	/*
	 * specific part
	 */
	close_hdr = (norm_cmd_close_hdr_t*)(c_hdr + 1);
	close_hdr->grtt = 0;
	close_hdr->gsize = 0;
	close_hdr->flavor = NORM_CMD_CLOSE;
	close_hdr->reserved = 0;

	*hdr = c_hdr;
	TRACELVL(5, (mcl_stdout, "<- mcl_norm_pkt_mgmt::create_cmd_close_hdr:\n"))
	return (sizeof(norm_common_hdr_t) + sizeof(norm_cmd_close_hdr_t));
}


/****** sender specific parsing methods ***************************************/


/**
 * Parses the specific header of a NORM-NACK packet, performs some checks,
 * and initializes the info structure accordingly.
 * Used by the sender.
 * After calling this function, call get_next_nack_content_block() to
 * get all the NACK content blocks of the packet.
 * @param mclcb
 * @param pkt	pointer to the received packet
 * @param infos	pointer to the info struct that is updated by this
 *		function
 * @param	
 * @return	completion status (MCL_OK or MCL_ERROR).
 */
mcl_error_status
mcl_norm_pkt_mgmt::parse_nack_hdr (mcl_cb		*const mclcb,
				   mcl_rx_pkt		*pkt,
				   mcl_data_hdr_infos_t	*infos)
{
	norm_nack_hdr_t		*n_hdr;

	TRACELVL(5, (mcl_stdout, "-> mcl_norm_pkt_mgmt::parse_nack_hdr:\n"))
	/*
	 * quick check on packet length; must at least contain the
	 * norm_common + data headers
	 */
	if (pkt->pkt_len < (INT32)sizeof(norm_common_hdr_t) +
			   (INT32)sizeof(norm_nack_hdr_t) +
			   (INT32)sizeof(norm_nack_content_hdr_t)) {
		PRINT_ERR((mcl_stderr,
			"mcl_norm_pkt_mgmt::parse_nack_hdr: ERROR, too short (at least %d expected, got %d)\n",
			(INT32)sizeof(norm_common_hdr_t) +
			(INT32)sizeof(norm_data_hdr_t) +
			(INT32)sizeof(norm_nack_content_hdr_t), pkt->pkt_len))
		return MCL_ERROR;
	}
	/*
	 * content processing
	 */
	n_hdr = (norm_nack_hdr_t*)(pkt->get_buf() + sizeof(norm_common_hdr_t));
	infos->server_id = ntohl(n_hdr->server_id);
	infos->grtt_sec = ntohl(n_hdr->grtt_response_sec);
	infos->grtt_usec = ntohl(n_hdr->grtt_response_usec);
	infos->loss_estimate = ntohs(n_hdr->loss_estimate);
	infos->grtt_req_seq = ntohs(n_hdr->grtt_req_seq);
	/*
	 * remember position of the first norm_nack_content_hdr_t block
	 */
	this->next_nack_content_ptr = (norm_nack_content_hdr_t*)(n_hdr + 1);
	TRACELVL(5, (mcl_stdout, "<- mcl_norm_pkt_mgmt::parse_nack_hdr: ok\n"))
	return MCL_OK;
}


/**
 * Return the following norm_nack_content_hdr_t block of packet.
 * Requires calling parse_nack_hdr() first.
 * @return	completion status (MCL_OK or MCL_ERROR).
 */
norm_nack_content_hdr_t  *
mcl_norm_pkt_mgmt::get_next_nack_content_block (mcl_cb		*const mclcb,
						class mcl_rx_pkt *pkt)
{
	norm_nack_content_hdr_t	*nc;

	if (this->next_nack_content_ptr == NULL)
		return NULL;
	/*
	 * process the following NACK content block, i.e. move all
	 * values into host format.
	 */
	// TODO: perform block length check
	nc = next_nack_content_ptr;
	nc->length = ntohs(nc->length);
	nc->obj_transp_id = ntohs(nc->obj_transp_id);
	nc->fec_symbol_id_or_erasure_count = ntohs(nc->fec_symbol_id_or_erasure_count);
	nc->fec_block_nb = ntohl(nc->fec_block_nb);

	/* NB: assume there is a single content block! */
	// TODO: support of more than a single block...
	this->next_nack_content_ptr = NULL;
	return nc;
}



/****** receiver specific parsing methods *************************************/


/**
 * Parses the common header of a NORM packet, performs some checks,
 * and initializes the info structure accordingly.
 * Used by the receiver.
 * @param mclcb
 * @param pkt	pointer to the received packet
 * @param infos	pointer to the info struct that is updated by this
 * 		function
 * @return	completion status (MCL_OK or MCL_ERROR).
 */
mcl_error_status
mcl_norm_pkt_mgmt::parse_common_hdr (mcl_cb			*const mclcb,
				     mcl_rx_pkt			*pkt,
				     mcl_common_hdr_infos_t	*infos)
{
	norm_common_hdr_t	*c_hdr;

	TRACELVL(5, (mcl_stdout, "-> mcl_norm_pkt_mgmt::parse_common_hdr:\n"))
	/*
	 * quick check on packet length; must at least contain the
	 * common header
	 */
	if (pkt->pkt_len < (INT32)(sizeof(norm_common_hdr_t))) {
		PRINT_ERR((mcl_stderr,
			"mcl_rx::parse_common_hdr: ERROR, too short (at least %d expected, got %d)\n",
			(INT32)sizeof(norm_common_hdr_t), pkt->pkt_len))
		return MCL_ERROR;
	}
	c_hdr = (norm_common_hdr_t*)(pkt->get_buf());
	if ((c_hdr->version) != NORM_VERSION) {
		PRINT_ERR((mcl_stderr,
			"mcl_rx::parse_common_hdr: ERROR, bad version (%d expected, got %d)\n",
			NORM_VERSION, c_hdr->version))
		return MCL_ERROR;
	}
	infos->type = c_hdr->type;
	infos->seq = ntohs(c_hdr->seq);
	infos->source_id = ntohl(c_hdr->source_id);
	TRACELVL(5, (mcl_stdout, "<- mcl_norm_pkt_mgmt::parse_common_hdr:\n"))
	return MCL_OK;
}


/**
 * Parses the specific header of a NORM-DATA packet, performs some checks,
 * and initializes the info structure accordingly.
 * Used by the receiver.
 * @param mclcb
 * @param pkt	pointer to the received packet
 * @param infos	pointer to the info struct that is updated by this
 *		function
 * @return	completion status (MCL_OK or MCL_ERROR).
 */
mcl_error_status
mcl_norm_pkt_mgmt::parse_data_hdr (mcl_cb		*const mclcb,
				   mcl_rx_pkt		*pkt,
				   mcl_data_hdr_infos_t	*infos)
{
	norm_data_hdr_t		*d_hdr;

	TRACELVL(5, (mcl_stdout, "-> mcl_norm_pkt_mgmt::parse_data_hdr:\n"))
	/*
	 * quick check on packet length; must at least contain the
	 * norm_common + data headers
	 */
	if (pkt->pkt_len < (INT32)sizeof(norm_common_hdr_t) + (INT32)sizeof(norm_data_hdr_t)) {
		PRINT_ERR((mcl_stderr,
			"mcl_norm_pkt_mgmt::parse_data_hdr: ERROR, too short (at least %d expected, got %d)\n",
			(INT32)sizeof(norm_common_hdr_t) +
			(INT32)sizeof(norm_data_hdr_t), pkt->pkt_len))
		return MCL_ERROR;
	}
	d_hdr = (norm_data_hdr_t*)(pkt->get_buf() + sizeof(norm_common_hdr_t));
	infos->flags = d_hdr->flags;
	infos->idf_adu = ntohs(d_hdr->obj_transp_id);
	infos->idf_block = ntohl(d_hdr->fec_block_nb);
	infos->idf_du = ntohs(d_hdr->fec_symbol_id);
	infos->du_len = ntohs(d_hdr->payload_len);
	infos->adu_len = ntohl(d_hdr->obj_size_lsb);
	/* NON-NORM-COMPLIANT: block_len is full-size block length in bytes */
	//infos->block_len = ntohs(d_hdr->fec_block_len);
	infos->block_len = ntohl(d_hdr->fec_block_len);
#if 0
	infos->is_fec = (infos->idf_du < infos->block_len) ? false : true;
#else
	/* NON-NORM-COMPLIANT: use a FEC flag in the symbol idf instead */
	infos->is_fec = (infos->idf_du & 0x8000 ? true : false);
	infos->idf_du = (infos->idf_du & 0x7FFF);	/* FEC flag removed! */
#endif
#if 0
	TRACE(("mcl_norm_pkt_mgmt::parse_data_hdr: is_fec=%d, idf_du=%d, block_len=%d\n",
		infos->is_fec, infos->idf_du, infos->block_len))
#endif
	/*
	 * perform some field checking now...
	 */
	if (infos->adu_len == 0) {
		PRINT_ERR((mcl_stderr,
			"mcl_norm_pkt_mgmt::parse_data_hdr: ERROR, bad adu_len field (got %d)\n",
			infos->adu_len))
		return MCL_ERROR;
	}
#if 0
	if (infos->block_len == 0 || (INT32)infos->block_len >= mclcb->fec.get_max_k()) {
		PRINT_ERR((mcl_stderr,
			"mcl_norm_pkt_mgmt::parse_data_hdr: ERROR, bad block_len field (got %d, should be [0;%d[)\n",
			infos->block_len, mclcb->fec.get_max_k()))
		return MCL_ERROR;
	}
#else
	if (infos->block_len == 0 ||
	    (INT32)infos->block_len >= mclcb->fec.get_max_k() * mclcb->get_payload_size()) {
		PRINT_ERR((mcl_stderr,
			"mcl_norm_pkt_mgmt::parse_data_hdr: ERROR, bad block_len field (got %d, should be [0;%d[)\n",
			infos->block_len,
			mclcb->fec.get_max_k() * mclcb->get_payload_size()))
		return MCL_ERROR;
	}
#endif
	TRACELVL(5, (mcl_stdout, "<- mcl_norm_pkt_mgmt::parse_data_hdr: ok\n"))
	return MCL_OK;
}


/**
 * Parses the specific header of a NORM_CMD packet, performs some checks,
 * and initializes the info structure accordingly.
 * Used by the receiver.
 * @param mclcb
 * @param pkt	pointer to the received packet
 * @param infos	pointer to the info struct that is updated by this
 *		function
 * @return	completion status (MCL_OK or MCL_ERROR).
 */
mcl_error_status
mcl_norm_pkt_mgmt::parse_cmd_hdr (mcl_cb		*const mclcb,
				  mcl_rx_pkt		*pkt,
				  mcl_data_hdr_infos_t	*infos)
{
	norm_cmd_common_hdr_t	*cmd_hdr;

	TRACELVL(5, (mcl_stdout, "-> mcl_norm_pkt_mgmt::parse_cmd_hdr:\n"))
	/*
	 * quick check on packet length; must at least contain the
	 * norm_common + cmd_common headers
	 */
	if (pkt->pkt_len < (INT32)sizeof(norm_common_hdr_t) + (INT32)sizeof(norm_cmd_common_hdr_t)) {
		PRINT_ERR((mcl_stderr,
			"mcl_norm_pkt_mgmt::parse_cmd_hdr: ERROR, too short (at least %d expected, got %d)\n",
			(INT32)sizeof(norm_common_hdr_t) +
			(INT32)sizeof(norm_cmd_common_hdr_t), pkt->pkt_len))
		return MCL_ERROR;
	}
	cmd_hdr = (norm_cmd_common_hdr_t*)(pkt->get_buf() + sizeof(norm_common_hdr_t));
	infos->cmd_flavor = (norm_cmd_flavor)cmd_hdr->flavor;

	switch (infos->cmd_flavor) {
	case NORM_CMD_FLUSH:
		norm_cmd_flush_hdr_t	*f_hdr;

		f_hdr = (norm_cmd_flush_hdr_t*)cmd_hdr;
		infos->idf_adu = ntohs(f_hdr->obj_transp_id);
		infos->idf_block = ntohl(f_hdr->fec_block_nb);
		infos->idf_du = 0;	// not used even if it's available
		infos->is_fec = false;	// not used even if it's available
		TRACELVL(5, (mcl_stdout,
		"   mcl_norm_pkt_mgmt::parse_cmd_hdr: NORM_CMD_FLUSH rcvd\n"))
		break;

	case NORM_CMD_NO_NEW_OBJECT:
		infos->max_idf_adu = ntohl(((norm_cmd_no_new_object_hdr_t*)cmd_hdr)->max_object_id);
		TRACELVL(5, (mcl_stdout,
		"   mcl_norm_pkt_mgmt::parse_cmd_hdr: NORM_CMD_NO_NEW_OBJECT rcvd, adu_idf range is [0; %d]\n",
			infos->max_idf_adu))
		break;
	case NORM_CMD_CLOSE:
		/* nothing to do for the present */
		TRACELVL(5, (mcl_stdout,
		"   mcl_norm_pkt_mgmt::parse_cmd_hdr: NORM_CMD_CLOSE rcvd\n"))
		break;
	default:
		PRINT_ERR((mcl_stderr,
		"mcl_norm_pkt_mgmt::parse_cmd_hdr: ERROR, NORM_CMD flavor %d not supported\n",
			infos->cmd_flavor))
		return MCL_ERROR;
	}
	TRACELVL(5, (mcl_stdout, "<- mcl_norm_pkt_mgmt::parse_cmd_hdr: ok\n"))
	return MCL_OK;
}


/****** receiver specific sending methods *************************************/

/**
 * Create a NORM_NACK header.
 * The buffer containing the header is statically assigned, and will
 * be reused at the next call to this member function, so please use
 * it immediately...
 * @return	the header length or <= 0 in case of error
 */
INT32
mcl_norm_pkt_mgmt::create_nack_hdr (mcl_cb		*const mclcb,
				    norm_common_hdr_t	**hdr,
				    UINT32		adu_id,
				    UINT32		blk_id,
				    INT32		missing)
{
	norm_common_hdr_t	*c_hdr;		// common header part
	norm_nack_hdr_t		*n_hdr;		// NORM_NACK header
	norm_nack_content_hdr_t	*nc_hdr;	// NORM_NACK content header

	TRACELVL(5, (mcl_stdout, "-> mcl_norm_pkt_mgmt::create_nack_hdr:\n"))
	/*
	 * common part
	 */
	FILL_NORM_COMMON_HDR(NORM_NACK);
	/*
	 * specific part
	 */
	ASSERT(missing > 0);
	n_hdr = (norm_nack_hdr_t*)(c_hdr + 1);
	n_hdr->server_id = 0;		// TODO
	n_hdr->grtt_response_sec = 0;	// TODO
	n_hdr->grtt_response_sec = 0;	// TODO
	n_hdr->loss_estimate = 0;	// TODO
	n_hdr->grtt_req_seq = 0;	// TODO
	/*
	 * fill in the nack content now
	 * only supports the "ask for additional FEC for this block" version
	 */
	nc_hdr = (norm_nack_content_hdr_t*)(n_hdr + 1);
	nc_hdr->form = NORM_NACK_ERASURES;
	nc_hdr->flags = NORM_NACK_BLOCK;
	nc_hdr->obj_transp_id = htons((UINT16)adu_id);
	nc_hdr->fec_symbol_id_or_erasure_count = htons(missing);
	nc_hdr->fec_block_nb = htonl((UINT32)blk_id);

	*hdr = c_hdr;
	TRACELVL(5, (mcl_stdout, "<- mcl_norm_pkt_mgmt::create_nack_hdr:\n"))
	return (sizeof(norm_common_hdr_t) + sizeof(norm_nack_hdr_t) +
		sizeof(norm_nack_content_hdr_t));
}


