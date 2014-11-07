/* $Id: mcl_adu.h,v 1.4 2004/02/18 07:56:18 roca Exp $ */
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

#ifndef MCL_ADU_H
#define MCL_ADU_H

#include "mcl_norm_pkt_mgmt.h"		// for mcl_data_hdr_infos_t definition


/** Initial Sequence number to use for the transmission of ADUs. */
#define	MCL_ISS	0

/**
 * Possible ADU status at a sender.
 * The possible transitions are the following:
 * 	NIL -> IN_PROGRESS -> FINISH_TX -> DONE
 */
enum mcl_adu_tx_status {
	ADU_TSTATUS_NIL		= 0,	/* void status */
	ADU_TSTATUS_IN_PROGRESS = 1,	/* DUs are being transmitted */
	ADU_TSTATUS_FINISH_TX	= 2,	/* all DUs from all blocks have been
					   sent once, currently in repair mode*/
	ADU_TSTATUS_DONE	= 3	/* totally and reliably sent, buffers
					   freed */
};

/**
 * Possible block status at a receiver.
 * The possible transitions are the following:
 * 	NIL -> IN_PROGRESS -> COMPLETED -> DECODED -> DELIVERED
 */
enum mcl_adu_rx_status {
	ADU_STATUS_NIL		= 0,	/* void status */
	ADU_STATUS_IN_PROGRESS	= 1,	/* not yet ready */
	ADU_STATUS_COMPLETED	= 2,	/* received all DUs from all blocks */
	ADU_STATUS_DECODED	= 3,	/* COMPLETED and FEC decoding done */
	ADU_STATUS_DELIVERED	= 4	/* sent to the receiving application */
};


/**
 * Application Data Unit (ADU).
 * They are the unit of data submitted by the sending application
 * in an mcl_send[to] call.
 * The ADU boundaries are preserved and communicated to the receivers.
 * Both sender and receivers keep a linked list of ADUs.
 */
class mcl_adu {
  
public:
	/****** Public Members ************************************************/
	/**
	 * Default constructor used by the sender.
	 * Initializes the various adu attributes, including its seq number.
	 * @param mclcb
	 * @param tx_or_rx	is a sender calling this function or a receiver?
	 * @param alen		length of this ADU
	 * @param saddr		NULL at a sender when calling mcl_send,
	 * 			contains dest addr otherwise.
	 * 			Contains sender's addr at a receiver.
	 */
	mcl_adu (class mcl_cb	*const mclcb,
		 //mcl_tx_or_rx	tx_or_rx,
		 INT32		const alen,
		 mcl_addr	const* saddr);

	/**
	 * Default constructor used by the receiver.
	 */
	mcl_adu (class mcl_cb		*const mclcb,
		 mcl_data_hdr_infos_t	const* dhdr_infos,
		 mcl_addr		const* saddr);
	/**
	 * Destructor
	 */
	~mcl_adu ();

	/**
	 * Free all buffers allocated for this ADU.
	 * All the blocks are concerned, and this function also frees all
	 * the data and FEC DUs they contain.
	 * Used by sender and receiver.
	 */
	void		remove_and_free_all_buffers (class mcl_cb *const mclcb);

	/**
	 * Returns the adu seq number.
	 */
	UINT32		get_seq ();

	/**
	 * Sets the ADU length in bytes.
	 * @param len	length
	 */
	void		set_len (INT32 len);

	/**
	 * Returns the ADU length in bytes.
	 * @return	length
	 */
	INT32		get_len () const;

	/**
	 * Calculates and sets the padded length in bytes.
	 * @param len	the real non padded length of this ADU in bytes
	 */
	void		set_padded_len (const INT32 len);

	/**
	 * Returns the padded length in bytes.
	 * @return	padded length in bytes
	 */
	INT32		get_padded_len () const;

	/**
	 * Set the destination or source address of this ADU.
	 */
	void		set_addr (const struct sockaddr *const saddr,
				  const INT32 saddr_len);
	//struct sockaddr	*get_saddr ();
	//INT32		get_saddr_len ();

	/**
	 * Set the data pointer.
	 * Only used by the sender.
	 * @param data_buf	pointer to data buffer
	 */
	void		set_data_ptr (char *const data_buf);
	char		*get_data_ptr ();

	/**
	 * True if the adu is for tx, false if it is for rx.
	 */
	bool		is_a_tx_adu () const;

	/**
	 * Return the status of a transmitted adu.
	 */
	mcl_adu_tx_status  get_tx_status () const;

	/**
	 * Set the new status of a transmitted adu.
	 */
	void		set_tx_status (mcl_adu_tx_status new_status);

	/**
	 * Return the status of a received adu.
	 */
	mcl_adu_rx_status  get_rx_status () const;

	/**
	 * Set the new status of a received adu.
	 */
	void		set_rx_status (mcl_adu_rx_status new_status);

	/**
	 * Check if this adu is JUST completed and take appropriate measures.
	 * Used by a receiver when a new non-duplicated du is received for
	 * a block.
	 * @param mclcb
	 * @return	true if completed and appropriate measures taken,
	 * 		false otherwise.
	 */
	bool		check_if_completed_and_process (mcl_cb	*const mclcb);

	/**
	 * Check if this adu is JUST completed and update state/stats.
	 * Used by a receiver when a new non-duplicated du is received for
	 * a block.
	 * @param mclcb
	 * @returns	true if the adu has JUST been completely received
	 * 		(decoded or not), false otherwise.
	 */
	bool		check_if_just_completed (mcl_cb *const mclcb);

	/**
	 * Check if this adu is completed.
	 * Used by a receiver.
	 * @param mclcb
	 * @returns	true if the adu has been completely received
	 * 		(decoded or not).
	 */
	bool		check_if_completed (mcl_cb *const mclcb);

	/**
	 * Decode all the block of an ADU.
	 * @param mclcb
	 * @returns	
	 */
	mcl_error_status	decode_all_blocks (mcl_cb *const mclcb);

	/**
	 * Return the head of the block list for this adu.
	 * This info is read-only (initialized at adu creation).
	 */
	class mcl_block	*get_block_head () const;

	/**
	 * Return the number of blocks for this adu.
	 * This info is read-only (initialized at adu creation).
	 */
	INT32		get_block_nb () const;

	/**
	 * Search a given block in this adu.
	 * @param mclcb
	 * @param bseq	block sequence number
	 * @return	returns a pointer to the block if found, NULL otherwise
	 */
	class mcl_block	*find_block (mcl_cb	*const mclcb,
				     UINT32	seq);

	/**
	 * Return the length in bytes of a full-sized block.
	 */
	INT32		get_full_size_block_len () const;

	/**
	 * Segment an adu.
	 * Essentially required by small block FEC codes.
	 * Only used by the sender.
	 * @param mclcb
	 * @return
	 */
	mcl_error_status  segment_for_tx (mcl_cb *const mclcb);

	/**
	 * Get adu before this one in list.
	 * @return Returns the prev pointer value as it is...
	 */
	mcl_adu		*get_prev () const;

	/**
	 * Get adu after this one in list.
	 * @return Returns the next pointer value as it is...
	 */
	mcl_adu		*get_next () const;

	/**
	 * Insert an adu in a list.
	 * Called by mcl_tx::insert_adu and mcl_rx::insert_adu methods with
	 * the appropriate list head parameter.
	 * @param mclcb
	 * @param head	pointer to the list head pointer
	 */
	mcl_error_status  insert_in_list (mcl_cb	*const mclcb,
					  mcl_adu	**head);

	/**
	 * Remove an adu from a list.
	 * Called by mcl_tx::remove_adu and mcl_rx::remove_adu methods with
	 * the appropriate list head parameter.
	 * @param mclcb
	 * @param head	pointer to the list head pointer
	 */
	mcl_error_status  remove_from_list (mcl_cb	*const	mclcb,
					    mcl_adu	**head);

	/**
	 * Find an adu in a list with its sequence number.
	 * @param mclcb
	 * @param head	pointer to the list head pointer
	 * @param aseq	ADU seq number
	 * @return	returns a pointer to the adu if found, NULL otherwise
	 */
	mcl_adu		*find_in_list (mcl_cb		*const mclcb,
					mcl_adu		*head,
					UINT32		aseq);



	/****** Public Attributes *********************************************/
	mcl_addr	addr;		// ADU recvd from or destinated to
  
private:
	/****** Private Members ***********************************************/
	/****** Private Attributes ********************************************/
	mcl_adu		*prev, *next;
	mcl_tx_or_rx	type;		// differentiate between TX and RX ADU
	UINT32		seq;		// seq nb of this ADU
	INT32		len;		// number of bytes in this ADU
	INT32		padded_len;	// len with optional 0 padding
	INT32		full_size_block_len; // lenght in bytes of a full size
					// block if any, else size of 1st blk
	class mcl_block	*block_head;	// first block of this ADU in list
	INT32		block_nb;	// number of blocks in this ADU
	/* fields only used by the sender */
	char		*data;		// ptr to data buffer */
	mcl_adu_tx_status tx_status;	// ADU tx status
	/* fields only used by the receivers */
	mcl_adu_rx_status rx_status;	// ADU rx status
  
};


//------------------------------------------------------------------------------
// Inlines for all classes follow
//------------------------------------------------------------------------------

inline UINT32
mcl_adu::get_seq ()
{
	return this->seq;
}

inline void
mcl_adu::set_len (INT32	len)
{
	this->len = len;
}

inline INT32
mcl_adu::get_len () const
{
	return this->len;
}


inline INT32
mcl_adu::get_padded_len () const
{
	return this->padded_len;
}


inline void
mcl_adu::set_addr (const struct sockaddr *const saddr, const INT32 saddr_len)
{
	this->addr.set_addr_struct((struct sockaddr_in*)saddr);
}


#if 0
inline struct sockaddr *
mcl_adu::get_saddr ()
{
	return &(this->saddr);
}

inline INT32
mcl_adu::get_saddr_len ()
{
	return this->saddr_len;
}
#endif

inline void
mcl_adu::set_data_ptr (char *const data_buf)
{
	ASSERT(this->type == MCL_TX);
	this->data =  data_buf;
}

inline char *
mcl_adu::get_data_ptr ()
{
	return this->data;
}

inline bool
mcl_adu::is_a_tx_adu () const
{
	return (this->type == MCL_TX ? true : false);
}

inline mcl_adu_tx_status
mcl_adu::get_tx_status () const
{
	return this->tx_status;
}

inline void
mcl_adu::set_tx_status (mcl_adu_tx_status new_status)
{
	this->tx_status = new_status;
}

inline mcl_adu_rx_status
mcl_adu::get_rx_status () const
{
	return this->rx_status;
}

inline void
mcl_adu::set_rx_status (mcl_adu_rx_status new_status)
{
	this->rx_status = new_status;
}

inline bool
mcl_adu::check_if_completed (mcl_cb *const mclcb)
{
	return ((this->rx_status >= ADU_STATUS_COMPLETED) ? true : false);
}

inline mcl_block *
mcl_adu::get_block_head () const
{
	return this->block_head;
}

inline INT32	
mcl_adu::get_block_nb () const
{
	return this->block_nb;
}

inline INT32
mcl_adu::get_full_size_block_len () const
{
	return this->full_size_block_len;
}

inline mcl_adu *
mcl_adu::get_prev () const
{
	return this->prev;
}

inline mcl_adu *
mcl_adu::get_next () const
{
	return this->next;
}

#endif // !MCL_ADU_H

