/* $Id: mcl_du.h,v 1.2 2003/12/17 15:56:01 roca Exp $ */
/*
 *  Copyright (c) 2003 INRIA - All rights reserved
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

#ifndef MCL_DU_H
#define MCL_DU_H


/**
 * Data Unit (DU)
 * They are the result of the segmentation of a block (i.e. ADU chunck)
 * and identify a sequence of bytes.
 * They can either contain plain data or redundant FEC data.
 *
 * Four kinds of DUs exist:
 * SENDER
 * 	- data DUs: their data field points to an internal copy of the
 * 	  ADU's data buffer. All the DUs of an ADU are allocated at
 * 	  the same time, in a table of DU objects. There is no default
 * 	  DU constructor in order to reset this whole DU table at once
 * 	  (rather than individually, which is much slower!)
 * 	- FEC DUs: their data field points to a malloc'd buffer.
 * 	  These DUs are allocated individually.
 * RECEIVER
 * 	- data and FEC DUs received from network: their pkt field points
 * 	  to the rx_packet object which owns the data buffer.
 * 	- reconstructed data DUs: no pkt field, the data field points to
 * 	  a malloc'd buffer. These DUs are allocated individually.
 */

class mcl_du {
 
public:
	/****** Public Members ************************************************/
	/*
	 * There is no default constructor for performance reasons...
	 * The class is assumed to be manually reset to 0 after creation.
	 * see mcl_adu::segment_for_tx()
	 */

	/**
	 * default destructor
	 */
	~mcl_du ();

	/**
	 * overloaded destructor.
	 * Called when the du->block->adu chain is not fully initialized.
	 * @param type	differentiate between TX and RX ADU
	 */
	//mcl_du::~mcl_du (mcl_tx_or_rx	type);

	/**
	 * Manipulate the main list pointers.
	 */
	void		set_prev (mcl_du *const p);
	mcl_du		*get_prev () const;
	void		set_next (mcl_du *const n);
	mcl_du		*get_next () const;

	/**
	 * Same as above with the second (sender specific) list.
	 */
	void		set_tx_prev (mcl_du *const p);
	mcl_du		*get_tx_prev () const;
	void		set_tx_next (mcl_du *const n);
	mcl_du		*get_tx_next () const;

	/**
	 * Insert a du in an ordered list according to the sequence number.
	 * Called with the appropriate list head parameter.
	 * @param mclcb
	 * @param head	pointer to the list head pointer
	 * @return	completion status: MCL_OK if inserted, or MCL_ERROR
	 * 		if already present in list.
	 */
	mcl_error_status  insert_in_list (mcl_cb	*const mclcb,
					  mcl_du	**head);

#if 0
	/**
	 * Remove a du from a list.
	 * Called with the appropriate list head parameter.
	 * @param mclcb
	 * @param head	pointer to the list head pointer
	 */
	mcl_error_status  remove_from_list (mcl_cb	*const	mclcb,
					    mcl_du	**head);

	/**
	 * Find a du in a list with its sequence number.
	 * @param mclcb
	 * @param head	pointer to the list head pointer
	 * @param seq	ADU seq number
	 * @return	returns a pointer to the adu if found, NULL otherwise
	 */
	mcl_adu		*find_in_list (mcl_cb		*const mclcb,
					mcl_du		*head,
					UINT32		seq);
#endif // 0

	/****** Public Attributes *********************************************/
	mcl_block	*block;		/* block to which this DU belongs */
	char		*data;		/* ptr to data in buffer */
	UINT32		seq;		/* sequence number of this DU */
	UINT16		len;		/* number of bytes in this DU */
	bool		is_fec;		/* is it a FEC or a data DU? */
	/* rx specific field */
	class mcl_rx_pkt *pkt;		// object containing the packet recvd
 
private:
	/****** Private Members ***********************************************/
	/****** Private Attributes ********************************************/
	/**
	 * Pointers to DU and FEC DU lists of a block.
	 * These are the only lists can should use these pointers!
	 */
	mcl_du		*prev, *next;
 
	/* tx specific field */
	/**
	 * Pointer to second DU and FEC DU list.
	 * Used by the tx_window service!
	 */
	mcl_du		*tx_prev, *tx_next;
};


//------------------------------------------------------------------------------
// Inlines for all classes follow
//------------------------------------------------------------------------------

/**
 * Manipulate the main list pointers.
 */

inline void
mcl_du::set_prev (mcl_du *const p)
{
	this->prev = p;
}

inline mcl_du *
mcl_du::get_prev () const
{
	return this->prev;
}

inline void
mcl_du::set_next (mcl_du *const n)
{
	this->next = n;
}

inline mcl_du *
mcl_du::get_next () const
{
	return this->next;
}


/**
 * Same as above with the second (sender specific) list.
 */

inline void
mcl_du::set_tx_prev (mcl_du *const p)
{
	this->tx_prev = p;
}

inline mcl_du *
mcl_du::get_tx_prev () const
{
	return this->tx_prev;
}

inline void
mcl_du::set_tx_next (mcl_du *const n)
{
	this->tx_next = n;
}

inline mcl_du *
mcl_du::get_tx_next () const
{
	return this->tx_next;
}

#endif // !MCL_DU_H

