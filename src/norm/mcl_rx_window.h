/* $Id: mcl_rx_window.h,v 1.4 2004/06/29 10:31:35 roca Exp $ */
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


/** Possible delivery modes of ADUs to the receiving application. */
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
				      mcl_adu	*const adu);

	/**
	 * Remove the ADU from the receiving window.
	 * @param mclcb
	 * @param adu
	 * @return	completion status (MCL_OK or MCL_ERROR).
	 */
	mcl_error_status  remove_adu (mcl_cb	*const mclcb,
				      mcl_adu	*const adu);

	/**
	 * Get the first ADU of the receiving window.
	 */
	mcl_adu		*get_first_adu (mcl_cb	*const mclcb);

	/**
	 * Get the last ADU of the receiving window.
	 */
	mcl_adu		*get_last_adu (mcl_cb	*const mclcb);

	/**
	 * Free all the ADUs of the receiving window.
	 * This function is only called at session close/abort.
	 */
	void		free_all_adu (mcl_cb	*const mclcb);

	/**
	 * Find an ADU in the receiving window with its sequence number.
	 * @param mclcb
	 * @param seq	ADU seq number
	 * @return	returns a pointer to the adu if found, NULL otherwise
	 */
	mcl_adu		*find_adu (mcl_cb	*const mclcb,
				   UINT32	idf_adu);

	/**
	 * Process an incoming ADU announcement and insert it in the list.
	 * The ADU must not already be in list.
	 * @param mclcb
	 * @param hddr_infos	structure containing the new ADU infos (plus
	 * 			many other info not used)
	 * @return	completion status (MCL_OK or MCL_ERROR).
	 */
	mcl_error_status  process_adu_announcement
				(mcl_cb			*const mclcb,
				 mcl_addr		* saddr,
				 mcl_data_hdr_infos_t	* dhdr_infos);

	/**
	 * Check if all ADUs have been completed.
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
	 * application.
	 * Warning, this function is read-only and does not change anything
	 * in the rx_window list nor does it mark the ADU as delivered!
	 * @return	next ADU fully received and decoded, complying with the
	 *		reception mode (IMMEDIATE_DELIVERY/ORDERED_DELIVERY) of
	 *		the application, if any, NULL otherwise.
	 */
	mcl_adu		*get_next_ready_adu (mcl_cb	*const mclcb);

	/**
	 * Update nb of ready adus plus the next adu to deliver info
	 * (ORDERED_DELIVERY) and change the ADU status to delivered.
	 */
	void		mark_adu_delivered(mcl_adu *adu);

	/**
	 * Register a new FEC or data DU in the receiving window.
	 * @return	completion status (MCL_OK or MCL_ERROR).
	 */
	mcl_error_status  insert_du (mcl_cb	*const mclcb,
				     mcl_du	*du);

	/**
	 * Does the receiver use an immediate delivery mode.
	 * @return	boolean
	 */
	bool			use_immediate_delivery_to_appli (void);

	/**
	 * Does the receiver use an ordered delivery mode.
	 * @return	boolean
	 */
	bool			use_ordered_delivery_to_appli (void);

	/**
	 * Set the delivery mode.
	 * @param mclcb
	 * @param mode	delivery mode to set
	 * @return	Completion status (MCL_OK or MCL_ERROR).
	 */
	mcl_error_status	set_delivery_mode_to_appli
					(mcl_cb			*const mclcb,
					 mcl_delivery_mode_to_appli	mode);



	/****** Public Attributes *********************************************/
  
	/**
	 * 
	 * @param XXX explanation
	 * @return Completion status (MCL_OK or MCL_ERROR).
	 */
	bool	postpone_fec_decoding;// decode during reception or at the end?
  
private:
	/****** Private Members ***********************************************/
	/**
	 * Insert a DU in the normal priority list.
	 * @param du	data or FEC DU to insert
	 */
	void	insert_in_list (mcl_cb *const mclcb, mcl_du *const du);

	/****** Private Attributes ********************************************/

	/** head of ADU doubly linked list. */
	mcl_adu		*adu_head;

	/**
	 * And here is the corresponding DU lists.
	 */
	/** DU list (du_list). */
	mcl_du		*du_list_head;
	//mcl_du	*du_list_tail;
	/** counter */
	INT32		nb_of_du_in_list;

	/** Number of ADUs been fully decoded and can be sent to the appli */
	INT32		nb_of_ready_adu;

	/* Immediate vs ordered_delivery of ADUs to the receiving appli. */
        mcl_delivery_mode_to_appli      delivery_mode_to_appli;
};


//------------------------------------------------------------------------------
// Inlines for all classes follow
//------------------------------------------------------------------------------

inline mcl_error_status
mcl_rx_window::insert_adu (mcl_cb	*const mclcb,
			   mcl_adu	*const adu)
{
	return adu->insert_in_list(mclcb, &(this->adu_head));
}

inline mcl_error_status
mcl_rx_window::remove_adu (mcl_cb	*const mclcb,
			   mcl_adu	*const adu)
{
	return adu->remove_from_list(mclcb, &(this->adu_head));
}

inline mcl_adu *
mcl_rx_window::find_adu (mcl_cb		*const mclcb,
			 UINT32		idf_adu)
{
	return this->adu_head->find_in_list(mclcb, this->adu_head, idf_adu);
}

inline mcl_adu *
mcl_rx_window::get_first_adu (mcl_cb *const mclcb)
{
	return this->adu_head;
}

inline mcl_adu *
mcl_rx_window::get_last_adu (mcl_cb *const mclcb)
{
	return ((this->adu_head != NULL) ? this->adu_head->get_prev() : NULL);
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
mcl_rx_window::use_immediate_delivery_to_appli ()
{
	return (this->delivery_mode_to_appli == MCL_IMMEDIATE_DELIVERY);
}

inline bool
mcl_rx_window::use_ordered_delivery_to_appli ()
{
	return (this->delivery_mode_to_appli == MCL_ORDERED_DELIVERY);
}

#endif // !MCL_RX_WINDOW_H

