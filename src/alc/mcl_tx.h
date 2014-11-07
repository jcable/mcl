/* $Id: mcl_tx.h,v 1.2 2005/02/25 14:13:11 moi Exp $ */
/*
 *  Copyright (c) 2005 INRIA - All rights reserved
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


/**
 * This Class controls many aspects related to transmissions.
 */
class mcl_tx {
 
public:
	/****** Public Members ************************************************/
	/**
	 * Default constructor.
	 */
	mcl_tx ();

	/**
	 * Default destructor.
	 */
	~mcl_tx ();

	/**
	 * Set the TOI (AKA ADU sequence number) to use for the next
	 * submitted adu.
	 * @param toi	toi to use for the next submitted adu.
	 * @return	completion status (MCL_OK or MCL_ERROR).
	 */
	mcl_error_status	set_next_toi (UINT32	toi);

	/**
	 * Return the TOI (AKA ADU sequence number) to use for a newly
	 * submitted adu.
	 * @return	returns the adu sequence number to use
	 */
	UINT32		get_next_toi ();

	/**
	 * Increment the TOI (AKA ADU sequence number) to use for a
	 * newly submitted adu.
	 */
	void		incr_next_toi ();

	/**
	 * Set the "keep data" mode.
	 * The keep_data mode, when set, postpones the scheduling of ADUs.
	 * Usefull to increase transmission efficiency when dealing
	 * with a large number of objects.
	 * @param mclcb
	 * @return	completion status (MCL_OK or MCL_ERROR).
	 */
	mcl_error_status	keep_data (mcl_cb	*const mclcb);

	/**
	 * End the "keep data" mode, or said differently, push all accumulated
	 * data.
	 * @param mclcb
	 * @return	completion status (MCL_OK or MCL_ERROR).
	 */
	mcl_error_status	push_data (mcl_cb	*const mclcb);

	/**
	 * Are we in "keep data" mode?
	 * @param mclcb
	 * @return	returns true if the "keep data" mode is set.
	 */
	bool		is_in_keep_data_mode ();

	/**
	 * Register a new ADU when in "keep data" mode.
	 * Here scheduling will be done later when the application
	 * issues a PUSH, so remember this ADU in the meantime.
	 * @param mclcb
	 * @param adu	ADU to register.
	 */
	void		register_adu_in_keep_data_mode (mcl_cb	*const mclcb,
							adu_t	*adu);

#if 0
	/**
	 * Return the highest adu seq number ever submitted by the sending app.
	 * @return	highest adu seq number submitted
	 */
	UINT32		get_highest_submitted_adu_seq () const;
#endif

	/**
	 * Informs there won't be any more ADU.
	 * This function is called as soon as the sending application
	 * issues a mcl_close().
	 * @param mclcb
	 */
	void		set_no_new_adu (mcl_cb	*const mclcb);

	/**
	 * Insert the adu in the tx adu list.
	 * This ADU list is ordered in increasing TOI number.
	 * @param mclcb
	 * @param adu
	 * @return	completion status (MCL_OK or MCL_ERROR).
	 */
	mcl_error_status	insert_adu (mcl_cb *const mclcb,
					    adu_t *const adu);
	/**
	 * Remove the adu from the tx adu list.
	 * @param mclcb
	 * @param adu
	 * @return	completion status (MCL_OK or MCL_ERROR).
	 */
	mcl_error_status	remove_adu (mcl_cb *const mclcb,
					    adu_t *const adu);

	/**
	 * Get the first adu of the tx adu list.
	 * @param mclcb
	 */
	adu_t		*get_first_adu (mcl_cb *const mclcb);

	/**
	 * Get the last adu of the tx adu list.
	 * @param mclcb
	 * @return	last adu of the list.
	 */
	adu_t		*get_last_adu (mcl_cb *const mclcb);

	/**
	 * Get the highest TOI of the tx adu list.
	 * This is also the last ADU's TOI since the list is ordered.
	 * @param mclcb
	 * @return	TOI of the last adu of the list.
	 */
	UINT32		get_highest_toi (mcl_cb *const mclcb);

	/**
	 * Free all the adus of the tx adu list.
	 * This function is only called at session close/abort.
	 * @param mclcb
	 */
	void		free_all_adu (mcl_cb *const mclcb);


	/**
	 * Find an ADU in the tx list with its sequence number and instance id.
	 * If parameter FDT_instanceid is -1, don't care for instance ID when 
	 * searching ADU, and take the first ADU with the specified sequence
	 * number seq.
	 * @param mclcb
	 * @param idf_adu		ADU seq number
	 * @param FDT_instanceid	FDT instance id
	 * @return	returns a pointer to the adu if found, NULL otherwise
	 */
	adu_t		*find_adu (mcl_cb	*const mclcb,
				   UINT32	idf_adu,
				   INT32	FDT_instanceid);

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
	 * Copy data from the buffers/files specified by the msghdr to the ADU.
	 * Depending on the storage mode, data may be copied to an internal
	 * buffer, or on the contrary kept on disk.
	 * @param mclcb
	 * @param msg	Message control struct used by mcl_sndmsg/mcl_recvmsg.
	 * @param flags	Flags used by mcl_sndmsg/mcl_recvmsg.
	 * @return	Completion status (MCL_OK or MCL_ERROR).
	 */
	mcl_error_status	copy_from_iovec_to_adu
					(mcl_cb			*const mclcb,
					 struct mcl_msghdr	*msg,
					 adu_t			*adu);

#if 0
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
	 * @param mclcb
	 * @param desired_rate desired rate in bits per second
	 * @return	rate actually set
	 */
	float		set_bps_tx_rate (mcl_cb		*const mclcb,
					 const float	desired_rate);
#endif

#if 0	// not yet
	/**
	 * Set a pre-defined transmission profile.
	 * @param mclcb
	 * @return	completion status (MCL_OK or MCL_ERROR).
	 */
	mcl_error_status  set_tx_profile (mcl_cb		*const mclcb,
					  mcl_tx_profile	prof);
#endif

	/**
	 * Try to send as much data as possible in a tx tick for this session.
	 * This is the general sending function.
	 * @param mclcb
	 */
	void		try_to_send (mcl_cb *mclcb);


	/****** Public Attributes *********************************************/

	/**
	 * True if one or more DUs remain to be send.
	 * Used by the mcl_wait_event() function.
	 */
	bool		there_is_more_to_tx;
 
	/**
	 * True if next ADU is of high priority (e.g. control data,
	 *  FDT with FLUTE).
	 */
	bool		next_adu_is_of_high_prio;


private:
	/****** Private Members ***********************************************/
	/**
	 * Send this number of DUs for this session on this layer.
	 * @param mclcb
	 * @param du_nb		Number of DUs to send
	 */
	void		send_pkt (mcl_cb	*const mclcb,
				  struct txlay	*tl,
				  INT32		du_nb,
				  INT32		du_nb_high);

	/****** Private Attributes ********************************************/
	/** TOI (AKA ADU sequence number) to use for the next submitted ADU. */
	UINT32		next_toi;

	/** True to take control of buffer */
	bool		reuse_appli_tx_buffer;

	/** head of ADU doubly linkded list. */
	adu_t		*adu_head;

	/** the keep_data mode, when set, postpones the scheduling of ADUs. */
	bool		keep_data_mode_set;
	/** First ADU concerned by the keep_data mode. */
	adu_t		*adu_start;
	/** Last ADU concerned by the keep_data mode. */
	adu_t		*adu_end;

	/* initially true, false once a packet has been sent for this session */
	bool		first_tx_for_mclcb;
#if 0
	/** Transmission rate in bits per second (bps) */
	float		bps_tx_rate;
	/** Transmission rate in packets per second (pps) */
	float		pps_tx_rate;
	/** Transmission rate in packets per tick of the periodic timer (ppt).
	 * It must be an integer */
	float		ppt_tx_rate;
#endif

};


//------------------------------------------------------------------------------
// Inlines for all classes follow
//------------------------------------------------------------------------------

inline mcl_error_status
mcl_tx::set_next_toi (UINT32	toi)
{
	this->next_toi = toi;
	return MCL_OK;
}

inline UINT32
mcl_tx::get_next_toi ()
{
	return this->next_toi;
}

inline void
mcl_tx::incr_next_toi ()
{
	this->next_toi++;
}

inline  bool
mcl_tx::is_in_keep_data_mode ()
{
	return this->keep_data_mode_set;
}

#if 0
inline UINT32
mcl_tx::get_highest_submitted_adu_seq () const
{
	return this->highest_adu_seq;
}
#endif

inline void
mcl_tx::set_no_new_adu (mcl_cb	*const mclcb)
{
	//if (mclcb->fsm.no_new_adu(mclcb) == false)
	//	exit(-1);
	//ASSERT(mclcb->fsm.no_new_adu(mclcb));
}

inline mcl_error_status
mcl_tx::insert_adu (mcl_cb	*const mclcb,
		    adu_t	*const adu)
{
	//ASSERT(mclcb->fsm.no_new_adu(mclcb) == false);
	//return adu->insert_in_list(mclcb, &(this->adu_head));
	mcl_insert_adu(mclcb, adu, &(this->adu_head));
	return MCL_OK;
}

inline mcl_error_status
mcl_tx::remove_adu (mcl_cb	*const mclcb,
		    adu_t	*const adu)
{
	//adu->remove_from_list(mclcb, &(this->adu_head));
	mcl_remove_adu(mclcb, adu, &(this->adu_head));
	return MCL_OK;
}


inline adu_t *
mcl_tx::get_first_adu (mcl_cb *const mclcb)
{
	return this->adu_head;
}

inline adu_t *
mcl_tx::get_last_adu (mcl_cb *const mclcb)
{
	//return ((this->adu_head != NULL) ? this->adu_head->get_prev() : NULL);
	return ((this->adu_head != NULL) ? this->adu_head->prev : NULL);
}

inline UINT32
mcl_tx::get_highest_toi (mcl_cb *const mclcb)
{
	ASSERT(this->adu_head);
	return (mcl_get_highest_toi(mclcb, this->adu_head));
}

#if 0
inline adu_t *
mcl_tx::find_adu (mcl_cb	*const mclcb,
		  UINT32	idf_adu)
{
	return this->adu_head->find_in_list(mclcb, this->adu_head, idf_adu);
}
#endif

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

#if 0
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
#endif

#endif // !MCL_TX_H

