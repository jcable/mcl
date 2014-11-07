/* $Id: mcl_tx.h,v 1.4 2004/05/26 07:36:04 roca Exp $ */
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

#ifndef MCL_TX_H
#define MCL_TX_H

#include "mcl_lib_api_norm.h"

/**
 * This Class controls many aspects related to transmissions.
 */
class mcl_tx {
 
public:
	/****** Public Members ************************************************/
	mcl_tx ();
	~mcl_tx ();

	/**
	 * Return the adu sequence number to use for a newly submitted adu
	 * AND INCREMENT the counter.
	 * @return	returns the adu sequence number to use
	 */
	UINT32		get_seq_for_new_adu ();

	/**
	 * Return the highest adu seq number ever submitted by the sending app.
	 * @return	highest adu seq number submitted
	 */
	UINT32		get_highest_submitted_adu_seq () const;

	/**
	 * Informs there won't be any more ADU.
	 * This function is called as soon as the sending application
	 * issues a mcl_close().
	 */
	void		set_no_new_adu (mcl_cb	*const mclcb);

	/**
	 * Insert the adu in the tx adu list.
	 * @param mclcb
	 * @param adu
	 * @return	completion status (MCL_OK or MCL_ERROR).
	 */
	mcl_error_status	insert_adu (mcl_cb *const mclcb,
					    mcl_adu *const adu);
	/**
	 * Remove the adu from the tx adu list.
	 * @param mclcb
	 * @param adu
	 * @return	completion status (MCL_OK or MCL_ERROR).
	 */
	mcl_error_status	remove_adu (mcl_cb *const mclcb,
					    mcl_adu *const adu);

	/**
	 * Get the first adu of the tx adu list.
	 */
	mcl_adu		*get_first_adu (mcl_cb *const mclcb);

	/**
	 * Get the last adu of the tx adu list.
	 */
	mcl_adu		*get_last_adu (mcl_cb *const mclcb);

	/**
	 * Free all the adus of the tx adu list.
	 * This function is only called at session close/abort.
	 */
	void		free_all_adu (mcl_cb *const mclcb);

	/**
	 * Find an ADU in the receiving window with its sequence number.
	 * @param mclcb
	 * @param seq	ADU seq number
	 * @return	returns a pointer to the adu if found, NULL otherwise
	 */
	mcl_adu		*find_adu (mcl_cb	*const mclcb,
				   UINT32	idf_adu);

	/**
	 * Determine if application buffer can be reused by MCL or not.
	 * @param val	boolean
	 */
	void		set_reuse_appli_buf_bool (bool val);

	/**
	 * True if application buffer can be reused by MCL.
	 * @return	boolean
	 */
	bool		can_reuse_appli_buf () const;

	/**
	 * Get the transmission rate in bits per second (bps).
	 * @return	current transmission rate
	 */
	float		get_bps_tx_rate () const;

	/**
	 * Get the transmission rate in packets per second (pps).
	 * @return	current transmission rate
	 */
	float		get_pkt_per_sec_tx_rate () const;

	/**
	 * Get the transmission rate in packets per tick (ppt).
	 * @return	current transmission rate
	 */
	float		get_pkt_per_tick_tx_rate () const;

	/**
	 * Set the transmission rate in bits per second (bps).
	 * The desired rate can be adjusted so that the is an integral
	 * number of packets per tick.
	 * @param desired_rate desired rate in bits per second
	 * @return	rate actually set
	 */
	float		set_bps_tx_rate (mcl_cb		*const mclcb,
					 const float	desired_rate);

	/**
	 * Set a pre-defined transmission profile.
	 * @return	completion status (MCL_OK or MCL_ERROR).
	 */
	mcl_error_status  set_tx_profile (mcl_cb		*const mclcb,
					  mcl_tx_profile	prof);

	/**
	 * Try to send as much data as possible in a tx tick for this session.
	 * This is the general sending function.
	 * @param mclcb
	 */
	void		try_to_send (mcl_cb *mclcb);

	/**
	 * Return the NORM sequence number of the following packet to send.
	 * The internal counter is automatically incremented by this call.
	 * The internal counter (a UINT16) cycles regularly, this is normal!
	 */
	UINT16		get_next_norm_pkt_seq ();


	/****** Public Attributes *********************************************/

 

private:
	/****** Private Members ***********************************************/
	/**
	 * Send this number of DUs for this session.
	 * @param mclcb
	 * @param du_nb		Number of DUs to send
	 */
	void		send_pkt (mcl_cb	*const mclcb,
				  INT32		du_nb);

	/****** Private Attributes ********************************************/
	UINT32		highest_adu_seq;// highest seq nb of ADUs submit. so far
	bool		reuse_appli_tx_buffer;
	mcl_adu		*adu_head;	// head of ADU doubly linked list

	/** Transmission rate in bits per second (bps) */
	float		bps_tx_rate;
	/** Transmission rate in packets per second (pps) */
	float		pps_tx_rate;
	/** Transmission rate in packets per tick of the periodic timer (ppt).
	 * It must be an integer */
	float		ppt_tx_rate;

	/**
	 * Counter incremented each time a NORM packet is sent, no matter
	 * its type.
	 * Used to discover packet losses in a continuous flow.
	 */
	UINT16		next_norm_pkt_seq;

};


//------------------------------------------------------------------------------
// Inlines for all classes follow
//------------------------------------------------------------------------------


inline UINT32
mcl_tx::get_seq_for_new_adu ()
{
	this->highest_adu_seq++;
	return this->highest_adu_seq;
}

inline UINT32
mcl_tx::get_highest_submitted_adu_seq () const
{
	return this->highest_adu_seq;
}

inline void
mcl_tx::set_no_new_adu (mcl_cb	*const mclcb)
{
	//if (mclcb->fsm.no_new_adu(mclcb) == false)
	//	exit(-1);
	//ASSERT(mclcb->fsm.no_new_adu(mclcb));
}

inline mcl_error_status
mcl_tx::insert_adu (mcl_cb	*const mclcb,
		    mcl_adu	*const adu)
{
	//ASSERT(mclcb->fsm.no_new_adu(mclcb) == false);
	adu->insert_in_list(mclcb, &(this->adu_head));
	return MCL_OK;
}

inline mcl_error_status
mcl_tx::remove_adu (mcl_cb	*const mclcb,
		    mcl_adu	*const adu)
{
	adu->remove_from_list(mclcb, &(this->adu_head));
	return MCL_OK;
}


inline mcl_adu *
mcl_tx::get_first_adu (mcl_cb *const mclcb)
{
	return this->adu_head;
}

inline mcl_adu *
mcl_tx::get_last_adu (mcl_cb *const mclcb)
{
	return ((this->adu_head != NULL) ? this->adu_head->get_prev() : NULL);
}

inline mcl_adu *
mcl_tx::find_adu (mcl_cb	*const mclcb,
		  UINT32	idf_adu)
{
	return this->adu_head->find_in_list(mclcb, this->adu_head, idf_adu);
}

inline void
mcl_tx::set_reuse_appli_buf_bool (bool val)
{
	this->reuse_appli_tx_buffer = val;
}

inline bool
mcl_tx::can_reuse_appli_buf () const
{
	return this->reuse_appli_tx_buffer;
}

inline float
mcl_tx::get_bps_tx_rate () const
{
	return this->bps_tx_rate;
}

inline float
mcl_tx::get_pkt_per_sec_tx_rate () const
{
	return this->pps_tx_rate;
}

inline float
mcl_tx::get_pkt_per_tick_tx_rate () const
{
	return this->ppt_tx_rate;
}

inline UINT16
mcl_tx::get_next_norm_pkt_seq ()
{
	UINT16 s = next_norm_pkt_seq;
	next_norm_pkt_seq++;
	return s;
}


#endif // !MCL_TX_H

