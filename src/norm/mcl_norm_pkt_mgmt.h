/* $Id: mcl_norm_pkt_mgmt.h,v 1.2 2004/01/30 16:27:42 roca Exp $ */
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

#ifndef MCL_NORM_PKT_MGMT_H
#define MCL_NORM_PKT_MGMT_H

/**
 * Struct used to store common header information
 */
typedef struct {
	INT32	type;
	UINT16	seq;
	UINT32	source_id;
} mcl_common_hdr_infos_t;


/**
 * Struct used to store data header information
 */
typedef struct mcl_data_hdr_infos {
	/*
	 * data related fields
	 */
	UINT32	flags;
	UINT32	idf_adu;	// ADU identifier
	UINT32	idf_block;	// BLOCK identifier
	UINT32	idf_du;		// DU identifier
	UINT32	is_fec;		// 1 if DU has FEC content
	UINT32	du_len;
	//UINT32 demux_label;	// demux label, also known as TSI
	UINT32	codepoint;	// codepoint
	// fields used by FTI (FEC object tx info) (previously NEWADU)
	UINT32	adu_len;	// ADU length in bytes
	UINT32	block_len;	// full-sized block length in bytes
	//UINT32 block_du_nb;	// full-sized block length in # of DUs (k)

	/*
	 * NORM_CMD specific fields
	 */
	norm_cmd_flavor	cmd_flavor;	// type of command or NORM_CMD_INVALID
	// fields used by NONEWADU
	UINT32	max_idf_adu;	// highest ADU identifier in session
	// fields used by CLOSE
	UINT32	close;		// CLOSE sig: boolean

	/*
	 * NORM_NACK specific flags
	 */
	UINT32	server_id;
	UINT32	grtt_sec;
	UINT32	grtt_usec;
	UINT32	loss_estimate;
	UINT32	grtt_req_seq;
} mcl_data_hdr_infos_t;


/**
 * enumeration of the various signaling types supported.
 * these types are in fact mapped onto the NORM signaling types.
 */
enum mcl_sig_types {
	MCL_SIG_NEWADU = 0,
	MCL_SIG_NONEWADU,
	MCL_SIG_CLOSE
};


/**
 * This class provides all the required functions to create and analyze
 * NORM packets.
 * One instance per session.
 */
class mcl_norm_pkt_mgmt {

public:
	/****** Public Members ************************************************/
	/**
	 * Default constructor.
	 */
	mcl_norm_pkt_mgmt ();

	/**
	 * Default destructor.
	 */
	~mcl_norm_pkt_mgmt ();

	/****** sender specific sending methods *******************************/

	/**
	 * Creates a data packet, allocating and initializing the NORM header.
	 * Used by the sender.
	 * Only the header is initialized, the data buffer is left untouched.
	 * The buffer containing the header is statically assigned, and will
	 * be reused at the next call to this member function, so please use
	 * it immediately...
	 * @param mclcb
	 * @param du	the data unit to send
 	 * @param chdr	pointer to header pointer updated when leaving this
	 * 		function
 	 * @return	the header length or <= 0 in case of error
	 */
	INT32		create_data_hdr (mcl_cb			*const mclcb,
					 mcl_du			*const du,
					 norm_common_hdr_t	**hdr);

	/**
	 * Create a NORM_CMD(FLUSH) header.
	 * Used by the sender.
	 * The buffer containing the header is statically assigned, and will
	 * be reused at the next call to this member function, so please use
	 * it immediately...
	 * @return	the header length or <= 0 in case of error
	 */
	
	INT32		create_cmd_flush_hdr (mcl_cb		*const mclcb,
					      norm_common_hdr_t	**hdr,
					      mcl_du		*du);

	/**
	 * Create a NORM_CMD(NO_NEW_OBJECT) header.
	 * Used by the sender.
	 * The buffer containing the header is statically assigned, and will
	 * be reused at the next call to this member function, so please use
	 * it immediately...
	 * @return	the header length or <= 0 in case of error
	 */
	INT32		create_cmd_no_new_object_hdr (mcl_cb	*const mclcb,
						      norm_common_hdr_t	**hdr);

	/**
	 * Create a NORM_CMD(CLOSE) header.
	 * Used by the sender.
	 * The buffer containing the header is statically assigned, and will
	 * be reused at the next call to this member function, so please use
	 * it immediately...
 	 * @return	the header length or <= 0 in case of error
	 */
	INT32		create_cmd_close_hdr (mcl_cb		*const mclcb,
					      norm_common_hdr_t	**hdr);

	/****** sender specific parsing methods *******************************/

	/**
	 * Parses the specific header of a NORM-NACK packet, performs some
	 * checks, and initializes the info structure accordingly.
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
	mcl_error_status  parse_nack_hdr (mcl_cb		*const mclcb,
					  class mcl_rx_pkt	*pkt,
					  mcl_data_hdr_infos_t	*infos);

	/**
	 * Return the following norm_nack_content_hdr_t block of packet.
	 * Requires calling parse_nack_hdr() first.
	 * @return	completion status (MCL_OK or MCL_ERROR).
	 */
	norm_nack_content_hdr_t  *get_next_nack_content_block
					(mcl_cb			*const mclcb,
					 class mcl_rx_pkt	*pkt);

	/****** receiver specific parsing methods *****************************/

	/**
	 * Parses the common header of any NORM packet, performs some checks
	 * and initializes the info structure accordingly.
	 * Used by the receiver.
	 * @param mclcb
	 * @param pkt	pointer to the received packet
	 * @param infos	pointer to the info struct that is updated by this
	 * 		function
	 * @return	completion status (MCL_OK or MCL_ERROR).
	 */
	mcl_error_status  parse_common_hdr (mcl_cb		*const mclcb,
					    class mcl_rx_pkt	*pkt,
					    mcl_common_hdr_infos_t *infos);

	/**
	 * Parses the specific header of a NORM-DATA packet, performs some
	 * checks and initializes the info structure accordingly.
	 * Used by the receiver.
	 * @param mclcb
	 * @param pkt	pointer to the received packet
	 * @param infos	pointer to the info struct that is updated by this
	 *		function
	 * @return	completion status (MCL_OK or MCL_ERROR).
	 */
	mcl_error_status  parse_data_hdr (mcl_cb		*const mclcb,
					  mcl_rx_pkt		*pkt,
					  mcl_data_hdr_infos_t *infos);

	/**
	 * Parses the specific header of a NORM_CMD packet, performs some
	 * checks and initializes the info structure accordingly.
	 * Used by the receiver.
	 * @param mclcb
	 * @param pkt	pointer to the received packet
	 * @param infos	pointer to the info struct that is updated by this
	 *		function
	 * @return	completion status (MCL_OK or MCL_ERROR).
	 */
	mcl_error_status  parse_cmd_hdr (mcl_cb		*const mclcb,
					 mcl_rx_pkt		*pkt,
					 mcl_data_hdr_infos_t	*infos);

	/****** receiver specific sending methods *****************************/

	/**
	 * Create a NORM_NACK header.
	 * Used by the receiver.
	 * The buffer containing the header is statically assigned, and will
	 * be reused at the next call to this member function, so please use
	 * it immediately...
	 * @return	the header length or <= 0 in case of error
	 */
	INT32
	create_nack_hdr (mcl_cb		*const mclcb,
					    norm_common_hdr_t	**hdr,
					    UINT32		adu_id,
					    UINT32		blk_id,
					    INT32		missing);


	/****** Public Attributes *********************************************/
 
private:
	/****** Private Members ***********************************************/
	/****** Private Attributes ********************************************/
	/**
	 * Maximum header size.
	 * Equal to the maximum datagram size (e.g. a signaling pkt)
	 */
	static const INT32	MCL_MAX_HDR_SIZE = 1500;

	/** Static buffer used to store the packet header just created. */
	char	hdr_buffer[MCL_MAX_HDR_SIZE];
 
	/**
	 * Pointer used by get_next_nack_content_block().
	 * Used to iterate on the various NACK content blocks of a NACK.
	 * Used by the sender.
	 */
	norm_nack_content_hdr_t	*next_nack_content_ptr;
};


//------------------------------------------------------------------------------
// Inlines for all classes follow
//------------------------------------------------------------------------------


#endif // !MCL_NORM_PKT_MGMT_H
