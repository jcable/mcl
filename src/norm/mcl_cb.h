/* $Id: mcl_cb.h,v 1.3 2004/01/30 16:27:42 roca Exp $ */
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

#ifndef MCL_CB_H
#define MCL_CB_H

#include "mcl_node.h"
#include "mcl_fsm.h"
#include "mcl_timer.h"
#include "../common/mcl_periodic_timer.h"
#include "mcl_periodic_proc.h"
#include "mcl_fec.h"
#include "mcl_stats.h"
#include "mcl_ses_channel.h"
#include "mcl_norm_pkt_mgmt.h"

#include "mcl_tx.h"
#include "mcl_tx_window.h"
#include "mcl_tx_ctrl.h"
#include "mcl_tx_storage.h"
#include "mcl_group_mgmt.h"

#include "mcl_rx.h"
#include "mcl_rx_window.h"
#include "mcl_rx_thread.h"
#include "mcl_rx_ctrl.h"
#include "mcl_rx_storage.h"


/** Possible session modes. */
enum mcl_tx_rx_mode {
	MCL_INVALID,
	MCL_IS_A_SENDER_ONLY,
	MCL_IS_A_RECEIVER_ONLY
	//MCL_IS_SENDER_AND_RECEIVER	not yet supported
};


/**
 * Control Block Class for an MCL_NORM session.
 * Contains all the variables, classes, member functions etc. required for
 * a session.
 */
class mcl_cb {
  
public:

	/****** Public Members ************************************************/
	/**
	 * Constructor.
	 * Perform pre-initialization of the session.
	 * At completion, the session is NOT fully initialized, but
	 * sufficiently to enable most tasks to be performed (e.g all
	 * threads are created).
	 * @param mode	determines if this is a sender or receiver
	 */
	mcl_cb (mcl_tx_rx_mode mode);

	/** Destructor */
	~mcl_cb ();

	/**
	 * Returns the local MCL session id.
	 * The session identifier is set once, at session creation.
	 * This is different from the node NORM id.
	 * @return	session id
	 */
	INT32	get_id();

	/**
	 * Get the tx/rx mode for this session.
	 * @return	mode
	 */
	mcl_tx_rx_mode	get_mode ();

	/**
	 * Is this session a sender's session.
	 * @return	boolean
	 */
	bool	is_a_sender ();

	/**
	 * Is this session a receiver's session.
	 * @return	boolean
	 */
	bool	is_a_receiver ();

	/**
	 * Set the verbosity level.
	 * @param level
	 * @return	Completion status (MCL_OK or MCL_ERROR).
	 */
	mcl_error_status	set_verbosity (INT32 level);

	/**
	 * Get the verbosity level.
	 * @return	verbosity level
	 */
	INT32	get_verbosity ();

	/**
	 * Set the statistic level
	 * @param level
	 * @return	Completion status (MCL_OK or MCL_ERROR).
	 */
	mcl_error_status	set_stats_level (INT32 level);

	/**
	 * Get the statistic level.
	 * @return	statistic level
	 */
	INT32	get_stats_level ();

	/**
	 * Set the session lock.
	 * There is one lock per MCL session. Their total number is equal
	 * to the number of concurrent sessions.
	 */
	void	lock ();

	/**
	 * Try to set the session lock.
	 * There is one lock per MCL session. Their total number is equal
	 * to the number of concurrent sessions.
	 * @return	return EBUSY if not possible, 0 if ok
	 *
	 */
	INT32	trylock ();

	/**
	 * Release the session lock.
	 */
	void	unlock ();

	/**
	 * Finish the initialization of the session if not already done.
	 * This end of init is done only once.
	 * @return	Completion status (MCL_OK or MCL_ERROR).
	 */
	mcl_error_status	finish_session_init_if_needed ();

	/**
	 * True once the session if fully initialized.
	 * @return	boolean
	 */
	bool			is_fully_initialized ();

	/**
	 * Free everything.
	 * This is done at the very last stage, at the end of session close
	 * or abort.
	 */
	void			free_everything ();

	/**
	 * Set the maximum datagram size (in bytes) and adjusts the payload
	 * size in consequence.
	 * A datagram includes the payload (if any) plus the NORM header.
	 * @return	Completion status (MCL_OK or MCL_ERROR).
	 */
	mcl_error_status	set_max_datagram_size (INT32 sz);

	/**
	 * Get the maximum datagram size (in bytes).
	 * A datagram includes the payload (if any) plus the NORM header.
	 * @return	maximum datagram size in bytes
	 */
	INT32			get_max_datagram_size () const;

	/**
	 * Set the payload size (in bytes).
	 * Only the data payload of the NORM datagram is considered here.
	 * @return	Completion status (MCL_OK or MCL_ERROR).
	 *
	 */
	mcl_error_status	set_payload_size (INT32 sz);

	/**
	 * Get the payload size (in bytes).
	 * Only the data payload of the NORM datagram is considered here.
	 * Can be set independantly of the max_datagram_size but cannot
	 * be larger than this latter.
	 * @return	payload size in bytes
	 */
	INT32			get_payload_size () const;


	/****** Public Attributes *********************************************/
	/*
	 * All the classes required by a session follow...
	 */
	/*
	 * Shared by senders and receivers
	 */
	class mcl_node		node;
	class mcl_fsm		fsm;
	class mcl_timer		timer;
	class mcl_periodic_proc	periodic_proc;
	class mcl_fec		fec;
	class mcl_stats		stats;
	class mcl_ses_channel	ses_channel; // session channel
	class mcl_norm_pkt_mgmt	norm_pkt_mgmt;

	/*
	 * Sender specific
	 */
	class mcl_tx		tx;		// Transmission class
	class mcl_group_mgmt	group_mgmt;
	class mcl_tx_ctrl	tx_ctrl;	// Performs all NACK/ACK/INFO
       						// control tasks
	class mcl_tx_window	tx_window;	// Store ADUs/DUs to send
	//class mcl_tx_pgmcc	tx_pgmcc;	not yet...
	//class mcl_tx_tfmcc	tx_tfmcc;	not yet...
	class mcl_tx_storage	tx_storage;

	/*
	 * Receiver specific
	 */
	class mcl_rx		rx;		// Reception class
	class mcl_rx_thread	rx_thread;	// Reception thread
	class mcl_rx_ctrl	rx_ctrl;	// Performs all NACK/ACK/INFO
						// control tasks
	class mcl_rx_window	rx_window;	// Store ADUs/DUs received
	//class mcl_rx_pgmcc	rx_pgmcc;	not yet...
	//class mcl_rx_tfmcc	rx_tfmcc;	not yet...
	class mcl_rx_storage	rx_storage;
  

private:
	/****** Private Members ***********************************************/
	/**
	 * Finish the initialization of the session.
	 * This function is called by finish_session_init_if_needed()
	 * after that all the appropriate params have been set up with
	 * mcl_ctl().
	 * @return	Completion status (MCL_OK or MCL_ERROR).
	 */
	mcl_error_status	finish_session_init ();

	/****** Private Attributes ********************************************/
	bool		fully_initialized; // true when session is fully init'ed
					// ie. after finish_session_init()
	INT32		id;		// MCL session identifier
	mcl_tx_rx_mode	tx_rx_mode;
	INT8		verbosity_level;// debug trace level 
	INT8		stats_level;	// statistics level
	mcl_mutex_t	session_lock;	// global session lock
 
	INT32		max_datagram_size; // max datagram size for tx and rx
	INT32		payload_size;	// default payload size for tx and rx
};


/**
 * Table containing the various control blocks of the sessions.
 * There can be at most MCLCB_MAX_ID simultaneous sessions in an MCL-NORM
 * instance.
 */
extern mcl_cb	*mclcb_tab[MAX_NB_MCLCB];

/** One time initialization function for the mclcb table. */
extern void mcl_init_mclcb_tab (void);


//------------------------------------------------------------------------------
// Inlines for all classes follow
//------------------------------------------------------------------------------


inline INT32
mcl_cb::get_id()
{
	return this->id;
}

inline mcl_tx_rx_mode
mcl_cb::get_mode ()
{
	return this->tx_rx_mode;
}

inline bool
mcl_cb::is_a_sender ()
{
	return (this->tx_rx_mode == MCL_IS_A_SENDER_ONLY);
}

inline bool
mcl_cb::is_a_receiver ()
{
	return (this->tx_rx_mode == MCL_IS_A_RECEIVER_ONLY);
}

inline INT32
mcl_cb::get_verbosity ()
{
	return (INT32) this->verbosity_level;
}

inline INT32
mcl_cb::get_stats_level ()
{
	return (INT32) this->stats_level;
}

inline void
mcl_cb::lock ()
{
	mcl_lock(&(this->session_lock));
}

inline INT32
mcl_cb::trylock ()
{
	return mcl_trylock(&(this->session_lock));
}

inline void
mcl_cb::unlock ()
{
	mcl_unlock(&(this->session_lock));
}

inline mcl_error_status
mcl_cb::finish_session_init_if_needed ()
{
	if (this->fully_initialized)
		return MCL_OK;
	else
		return (this->finish_session_init());
}

inline bool
mcl_cb::is_fully_initialized ()
{
	return this->fully_initialized;
}

inline INT32
mcl_cb::get_max_datagram_size () const
{
	return this->max_datagram_size;
}

inline INT32
mcl_cb::get_payload_size () const
{
	return this->payload_size;
}

#endif // MCL_CB_H

