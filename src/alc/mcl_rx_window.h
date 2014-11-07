/* $Id: mcl_rx_window.h,v 1.9 2005/01/11 13:12:33 roca Exp $ */
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

#ifndef MCL_RX_WINDOW_H
#define MCL_RX_WINDOW_H


/**
 * Possible ADU delivery to the application modes.
 * IMMEDIATE_DELIVERY: return an ADU to the application in arrival order
 * ORDERED_DELIVERY:	return an ADU to the application only if all ADUs
 *			with a lower TOI have already been delivered
 */
enum mcl_delivery_mode_to_appli {
	MCL_IMMEDIATE_DELIVERY,
	MCL_ORDERED_DELIVERY
};


/**
 * This class provides all the required services to manage the reception
 * window at a receiver.
 */
class mcl_rx_window {
 
public:
	/****** Public Members ************************************************/
	/**
	 * default constructor
	 */
	mcl_rx_window ();

	/**
	 * default destructor
	 */
	~mcl_rx_window ();

	/**
	 * Register a new ADU in the receiving window.
	 * @param mclcb
	 * @param adu
	 * @return	completion status (MCL_OK or MCL_ERROR).
	 */
	mcl_error_status  insert_adu (mcl_cb	*const mclcb,
				      adu_t	*const adu);

	/**
	 * Remove the ADU from the receiving window.
	 * @param mclcb
	 * @param adu
	 * @return	completion status (MCL_OK or MCL_ERROR).
	 */
	mcl_error_status  remove_adu (mcl_cb	*const mclcb,
				      adu_t	*const adu);

	/**
	 * Get the first ADU of the receiving window.
	 * @param mclcb
	 */
	adu_t		*get_first_adu (mcl_cb	*const mclcb);

	/**
	 * Get the last ADU of the receiving window.
	 * @param mclcb
	 */
	adu_t		*get_last_adu (mcl_cb	*const mclcb);

	/**
	 * Free all the ADUs of the receiving window.
	 * This function is only called at session close/abort.
	 * @param mclcb
	 */
	void		free_all_adu (mcl_cb	*const mclcb);

	/**
	 * Find an ADU in the receiving window with its sequence number.
	 * @param mclcb
	 * @param toi	Transport Object Idf, AKA ADU seq ID
	 * @param FDT_instanceid associated FDT instance ID if it
			is an FDT (i.e. toi == 0) (FLUTE specific)
	 * @return	returns a pointer to the adu if found, NULL otherwise
	 */
	adu_t		*find_adu (mcl_cb	*const mclcb,
				   UINT32	toi,
				   INT32	FDT_instanceid);

	/**
	 * Check if all ADUs have been completed.
	 * @param mclcb
	 * @return	boolean
	 */
	bool		check_if_all_adu_completed (mcl_cb	*const mclcb);
	
	/**
	 * Note that a new ADU is ready.
	 */
	void		mark_ready_adu ();

	/**
	 * Is there an ADU ready to be returned to the appli?
	 */
	bool		is_an_adu_ready ();

	/**
	 * Find the following ADU that can be returned to the receiving
	 * application, while complying with the delivery mode
	 * (IMMEDIATE_DELIVERY/ORDERED_DELIVERY) and FLUTE mode if applicable.
	 * Warning, this function is read-only and does not change anything
	 * in the rx_window list nor does it mark the ADU as delivered!
	 * @param mclcb
	 * @return	next ADU fully received and decoded, complying with the
	 *		delivery mode, NULL otherwise.
	 */
	adu_t		*get_next_ready_adu (mcl_cb	*const mclcb);

	/**
	 * Try to return an ADU to the appli.
	 * Takes into account the various delivery modes to the application,
	 * the FLUTE mode if applicable, and the application desires.
	 * This function updates everything.
	 * @param mclcb
	 * @param msg
	 * @param flags
	 * @param toi	pointer to variable which will contain the TOI upon
	 * 		return (FLUTE and Meta-object specific).
	 * @return	the amount (> 0) of data copied to the buffers/files
	 * 		specified by msg (unless
	 * 		MCL_MSG_CHECK_IF_DATA_IS_AVAILABLE is set in flags in
	 * 		which case nothing is copied), or < 0 if no data is
	 * 		available.
	 */
	INT32		return_adu_to_appli (mcl_cb		*const mclcb,
					     struct mcl_msghdr	*msg,
					     enum mcl_msgflag	flags,
					     UINT32		*toi);

	/**
	 * Update the nb of ready adus (plus the next adu to deliver info
	 * in ORDERED_DELIVERY mode), and change the ADU status to delivered.
	 */
	void		mark_adu_delivered (adu_t	*adu);

	/**
	 * Does the receiver use an immediate delivery mode.
	 * @return	boolean
	 */
	bool		uses_immediate_delivery_to_appli (void);

	/**
	 * Does the receiver use an ordered delivery mode.
	 * @return	boolean
	 */
	bool		uses_ordered_delivery_to_appli (void);

	/**
	 * Set the ADU delivery mode to the application.
	 * @param mclcb
	 * @param mode	delivery mode to set
	 * @return	Completion status (MCL_OK or MCL_ERROR).
	 */
	mcl_error_status	set_delivery_mode_to_appli
					(mcl_cb			*const mclcb,
					 mcl_delivery_mode_to_appli	mode);

	/**
	 * Get the ADU delivery mode to the application.
	 * @param mclcb
	 * @return	delivery mode
	 */
	mcl_delivery_mode_to_appli	get_delivery_mode_to_appli
							(mcl_cb	*const mclcb);


	/****** Public Attributes *********************************************/
  
	/**
	 * Is RSE decoding performed progressively or at the very end?
	 */
	bool	postpone_fec_decoding;


private:
	/****** Private Members ***********************************************/

	/**
	 * Initialize the copy from DUs to the buffers/files specified by
	 * the msghdr. Creates a context. Be careful, this function is not
	 * re-entrant.
	 * @param mclcb
	 * @param msg	Message control struct used by mcl_sndmsg/mcl_recvmsg.
	 * @param flags	Flags used by mcl_sndmsg/mcl_recvmsg.
	 * @return	Completion status (MCL_OK or MCL_ERROR).
	 */
	mcl_error_status	init_adu_copy_to_iovec
					(mcl_cb			*const mclcb,
					 struct mcl_msghdr	*msg,
					 enum mcl_msgflag	flags);

	/**
	 * Copy an individual DU to the buffers/files specified by the msghdr.
	 * @param mclcb
	 * @param du
	 * @return	true if there is some more room in the destination
	 *		buffers/files, false otherwise.
	 */
	bool	copy_next_du_to_iovec (mcl_cb	*const mclcb,
				       du_t	*du);

	/**
	 * Return the total amount of data copied to buffers/files since
	 * the previous init_adu_copy_to_iovec.
	 * @param mclcb
	 * @return	total number of data copied.
	 */
	INT64	get_amount_of_data_copied_to_iovec (mcl_cb	*const mclcb);


	/****** Private Attributes ********************************************/
	/** Head of ADU doubly linked list. */
	adu_t		*adu_head;

	/** Number of ADUs been fully decoded and can be sent to the appli */
	UINT32		nb_of_ready_adu;

	/** Next ADU to return in case of an ORDERED_DELIVERY. */
	adu_t		*next_ordered_adu_to_return;
	/** TOI of next ADU to return in case of an ORDERED_DELIVERY. */
	UINT32		next_ordered_adu_to_return_toi;

	/* Immediate vs ordered_delivery of ADUs to the receiving appli. */
        mcl_delivery_mode_to_appli      delivery_mode_to_appli;


	/*
	 * Copy to iovec service.
	 */
	/** Amount of data already copied */
	INT64		cp2iov_copied;	

	/** True if dest is a buffer, false if dest is a file. */
	bool		cp2iov_is_buffer;

	/** Destination ptr in buffer. Does not necessarily point to start.
	 * Only valid if cp2iov_is_buffer is true */
	char		*cp2iov_dst;

	/** Destination file descriptor. */
	INT32		cp2iov_fd;	

#if 0
	/** Offset in destination file. */
	INT64		cp2iov_offset;	
#endif

	/** Remaining size in destination area for the current iovec entry. */
	INT64		cp2iov_rem;

	/** Source msghdr struct. */
	struct mcl_msghdr	*cp2iov_msg;

	/** Current iovec in the iovec vector. */
	struct mcl_iovec	*cp2iov_cur_iov;

	/** Current iovec index in the iovec vector. */
	INT32			cp2iov_cur_iov_idx;
};


//------------------------------------------------------------------------------
// Inlines for all classes follow
//------------------------------------------------------------------------------

inline mcl_error_status
mcl_rx_window::insert_adu (mcl_cb	*const mclcb,
			   adu_t	*const adu)
{
	mcl_insert_adu(mclcb, adu, &(this->adu_head));
	return MCL_OK;
	//return adu->insert_in_list(mclcb, &(this->adu_head));
}

inline mcl_error_status
mcl_rx_window::remove_adu (mcl_cb	*const mclcb,
			   adu_t	*const adu)
{
	mcl_remove_adu(mclcb, adu, &(this->adu_head));
	return MCL_OK;
	//return adu->remove_from_list(mclcb, &(this->adu_head));
}

inline adu_t *
mcl_rx_window::find_adu (mcl_cb		*const mclcb,
			 UINT32		toi,
			 INT32		FDT_instanceid)
{
	return (mcl_find_adu(mclcb, toi, FDT_instanceid, this->adu_head));
	//return this->adu_head->find_in_list(mclcb, this->adu_head, toi);
}

inline adu_t *
mcl_rx_window::get_first_adu (mcl_cb *const mclcb)
{
	return this->adu_head;
}

inline adu_t *
mcl_rx_window::get_last_adu (mcl_cb *const mclcb)
{
	return ((this->adu_head != NULL) ? this->adu_head->prev : NULL);
}

inline void
mcl_rx_window::mark_ready_adu()
{
	nb_of_ready_adu++;
}

inline	bool
mcl_rx_window::is_an_adu_ready ()
{
	return (nb_of_ready_adu > 0);
}

inline bool
mcl_rx_window::uses_immediate_delivery_to_appli ()
{
	return (this->delivery_mode_to_appli == MCL_IMMEDIATE_DELIVERY);
}

inline bool
mcl_rx_window::uses_ordered_delivery_to_appli ()
{
	return (this->delivery_mode_to_appli == MCL_ORDERED_DELIVERY);
}

inline mcl_delivery_mode_to_appli
mcl_rx_window::get_delivery_mode_to_appli (mcl_cb	*const mclcb)
{
	return this->delivery_mode_to_appli;
}

#endif // MCL_RX_WINDOW_H
