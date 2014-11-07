/* $Id: mcl_block.h,v 1.4 2004/02/18 07:56:18 roca Exp $ */
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


#ifndef MCL_BLOCK_H
#define MCL_BLOCK_H


/**
 * Possible block status at a sender.
 * The possible transitions are the following:
 * 	NIL -> IN_PROGRESS -> FINISH_TX -> DONE
 */
enum mcl_block_tx_status {
	BLK_TSTATUS_NIL		= 0,	/* void status */
	BLK_TSTATUS_IN_PROGRESS	= 1,	/* DUs are being transmitted */
	BLK_TSTATUS_FINISH_TX	= 2,	/* all DUs from this block have been
					   sent once, currently in repair mode*/
	BLK_TSTATUS_DONE	= 3	/* totally and reliably sent, buffer
					   freed */
};

/**
 * Possible block status at a receiver.
 * The possible transitions are the following:
 * 	NIL -> IN_PROGRESS -> COMPLETED -> DECODED -> DELIVERED
 */
enum mcl_block_rx_status {
	BLK_STATUS_NIL		= 0,	/* void status */
	BLK_STATUS_IN_PROGRESS	= 1,	/* not yet ready */
	BLK_STATUS_COMPLETED	= 2,	/* received all DUs for this block */
	BLK_STATUS_DECODED	= 3,	/* COMPLETED and FEC decoding done */
	BLK_STATUS_DELIVERED	= 4	/* sent to the receiving application */
};


/**
 * Source Data Block
 * They are the result of the segmentation of ADUs into several
 * blocks of size appropriate to the FEC code.
 */
class mcl_block {

public:
	/****** Public Members ************************************************/
	/*
	 * There is no default constructor for performance reasons...
	 * The  is assumed to be manually reset to 0 after creation.
	 * see mcl_adu::segment_for_tx()
	 */

	/**
	 * default destructor
	 */
	~mcl_block ();

	/**
	 * Return the status of a transmitted block.
	 */
	mcl_block_tx_status  get_tx_status () const;

	/**
	 * Set the new status of a transmitted block.
	 */
	void		set_tx_status (mcl_block_tx_status new_status);

	/**
	 * Return the status of a received block.
	 */
	mcl_block_rx_status  get_rx_status () const;

	/**
	 * Set the new status of a received block.
	 */
	void		set_rx_status (mcl_block_rx_status new_status);

	/**
	 * Check if this block is completed and take appropriate measures.
	 * Used by a receiver when a new non-duplicated DU is received for
	 * a block.
	 * @param mclcb
	 * @return	true if completed and appropriate measures taken,
	 * 		false otherwise.
	 */
	bool		check_if_completed_and_process (mcl_cb	*const mclcb);

	/**
	 * Check if this block is completed and return the number of missing
	 * DUs if any.
	 * Used by a receiver, in particular during NACK/ACK processing.
	 * @param mclcb
	 * @return	number of missing DUs, or 0 if already completed.
	 */
	INT32		get_number_of_missing_dus (mcl_cb	*const mclcb);

	/**
	 * Set the head of the DU ordered list.
	 */
	void		set_du_head ( mcl_du *const du);

	/**
	 * Get the head of the DU ordered list.
	 */
	 mcl_du	*get_du_head () const;

	/**
	 * Insert a DU in the data DU ordered list.
	 * @param mclcb
	 * @return	MCL_OK if inserted, MCL_ERROR if the DU was already
	 * 		in list or if an error occured
	 */
	mcl_error_status  insert_in_du_list (mcl_cb		*const mclcb,
					      mcl_du	*du);

	/**
	 * Set the head of the FEC DU ordered list.
	 * @param du	new DU head
	 */
	void		set_fec_du_head ( mcl_du *const du);

	/**
	 * Get the head of the FEC DU ordered list.
	 * @return	head
	 */
	 mcl_du	*get_fec_du_head () const;

	/**
	 * Get the tail of the FEC DU ordered list.
	 * @return	tail
	 */
	 mcl_du	*get_fec_du_tail () const;

	/**
	 * Insert a DU in the FEC DU ordered list.
	 * @param mclcb
	 * @return	MCL_OK if inserted, MCL_ERROR if the DU was already
	 * 		in list or if an error occured
	 */
	mcl_error_status  insert_in_fec_du_list (mcl_cb		*const mclcb,
						  mcl_du	*du);

	/**
	 * Remove and free all data DUs of the list.
	 * Used only by receivers.
	 * @param mclcb
	 */
	void		remove_and_free_all_data_dus (mcl_cb	*const mclcb);

	/**
	 * Remove and free all FEC DUs of the list.
	 * Can be called both by a sender or a receiver.
	 * @param mclcb
	 */
	void		remove_and_free_all_fec_dus (mcl_cb	*const mclcb);

	/**
	 * Return the number of data DUs in the data list.
	 */
	INT32		get_du_nb_in_list ();

	/**
	 * Return the number of FEC DUs in the FEC list.
	 */
	INT32		get_fec_du_nb_in_list ();

	/****** Public Attributes *********************************************/
  	 mcl_adu	*adu;		// ADU to which this block belongs
	UINT32		seq;		// block sequence number
	INT32		len;		// number of bytes in this block

	INT32		du_nb;		// nb of data DUs that compose this blk
					// (different from avail DUs in lists)
	/* sender specific fields */
	INT32		pending_fec_creation_req; //nb pending FEC pkt creations

private:
	/****** Private Attributes ********************************************/
	/*
	 * DU lists
	 */
 	 mcl_du	*du_head;	// first DU of this block in list/tab
	 mcl_du	*fec_du_head;	// first FEC DU of this block in list
	INT32		du_in_list;	// nb of non-FEC DUs avail (!= du_nb)
	INT32		fec_du_in_list;	// nb of FEC DUs available in list

	mcl_block_tx_status tx_status;	// tx status
	mcl_block_rx_status rx_status;	// rx status (completed/decoded/..)

};


//------------------------------------------------------------------------------
// Inlines for all es follow
//------------------------------------------------------------------------------

inline void
mcl_block::set_du_head ( mcl_du	*const du)
{
	this->du_head = du;
}

inline  mcl_du *
mcl_block::get_du_head () const
{
	return this->du_head;
}

inline void
mcl_block::set_fec_du_head ( mcl_du	*const du)
{
	ASSERT(du);
	this->fec_du_head = du;
}

inline  mcl_du *
mcl_block::get_fec_du_head () const
{
	return this->fec_du_head;
}

inline  mcl_du *
mcl_block::get_fec_du_tail () const
{
	if (this->fec_du_head)
		return this->fec_du_head->get_prev();
	else
		return NULL;
}

inline mcl_block_tx_status
mcl_block::get_tx_status () const
{
	return this->tx_status;
}

inline void
mcl_block::set_tx_status (mcl_block_tx_status	new_status)
{
	this->tx_status = new_status;
}


inline mcl_block_rx_status
mcl_block::get_rx_status () const
{
	return this->rx_status;
}

inline void
mcl_block::set_rx_status (mcl_block_rx_status	new_status)
{
	this->rx_status = new_status;
}

inline mcl_error_status
mcl_block::insert_in_du_list (mcl_cb	*const mclcb,
			       mcl_du	*du)
{
	if (du->insert_in_list(mclcb, &(this->du_head)) == MCL_OK) {
		this->du_in_list++;
		return MCL_OK;
	} else
		return MCL_ERROR;
}

inline mcl_error_status
mcl_block::insert_in_fec_du_list (mcl_cb	*const mclcb,
				   mcl_du	*du)
{
	if (du->insert_in_list(mclcb, &(this->fec_du_head)) == MCL_OK) {
		this->fec_du_in_list++;
		return MCL_OK;
	} else
		return MCL_ERROR;
}

inline INT32
mcl_block::get_du_nb_in_list ()
{
	return this->du_in_list;
}

inline INT32
mcl_block::get_fec_du_nb_in_list ()
{
	return this->fec_du_in_list;
}

#endif // !MCL_BLOCK_H

