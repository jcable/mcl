/* $Id: mcl_fsm.h,v 1.4 2005/01/11 13:12:28 roca Exp $ */
/*
 *  Copyright (c) 1999-2003 INRIA - All rights reserved
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

#ifndef MCL_FSM_H  /* { */
#define MCL_FSM_H

/*
 * SENDER's FSM (Finite State Machine)
 */

enum mcl_tx_states {
	TSTATE_NIL = 0,
	TSTATE_READY,
	TSTATE_IN_TX,
	TSTATE_FINISH_TX,
	TSTATE_END,
	TSTATE_CLOSED,
	TSTATE_INVALID
};

enum mcl_tx_events {
	TEVENT_NIL = 0,
	TEVENT_OPEN_CALLED,	/* application has called mcl_open */
	TEVENT_NEW_ADU,		/* tx: mcl_send called, rx: NEW ADU received */
	TEVENT_ALL_DU_SENT,	/* all the DUs have been sent */
	TEVENT_CLOSE_CALLED,	/* application has called mcl_close */
	TEVENT_CLOSE_SENT,	/* CLOSE sent to peer */
	TEVENT_CLOSE_RETURN,	/* mcl_close just returned */
	TEVENT_ABORT,		/* mcl_abort called */
	TEVENT_RESET		/* mcl_abort called */	
};


/*
 * RECEIVER's FSM (Finite State Machine)
 */

enum mcl_rx_states {
	RSTATE_NIL = 0,
	RSTATE_READY,
	RSTATE_IN_RX,
	RSTATE_FINISH_RX,
	RSTATE_END,
	RSTATE_CLOSE_RX,	/* CLOSE has been received */
	RSTATE_CLOSED,
	RSTATE_INVALID
};

enum mcl_rx_events {
	REVENT_NIL = 0,
	REVENT_OPEN_CALLED,	/* application has called mcl_open */
	REVENT_NEW_ADU,		/* NEW_ADU received */
	REVENT_NO_NEW_ADU,	/* NO_NEW_ADU received */
	REVENT_DU_RECV,		/* DU received */
	REVENT_ALL_DU_RECV,	/* all the DUs have been received */
	REVENT_CLOSE_CALLED,	/* application has called mcl_close */
	REVENT_CLOSE_RECV,	/* CLOSE sent to peer */
	REVENT_CLOSE_RETURN	/* mcl_close just returned */
};


/**
 * Class defining the Finite state machine of either a sender or a receiver.
 */
class mcl_fsm {

public:
	/****** Public Members ************************************************/
	/** Default constructor. */
	mcl_fsm ();

	/** Default destructor. */
	~mcl_fsm ();

	/**
	 * Update the current tx (resp. rx) state with a new event.
	 * @param mclcb
	 * @param event	new tx (resp. rx) event
	 * @return	returns nothing but calls exit in case of invalid
	 *		transition
	 */
	void	update_tx_state (class mcl_cb *const mclcb, mcl_tx_events event);
	void	update_rx_state (class mcl_cb *const mclcb, mcl_rx_events event);
	/**
	 * Get current state.
	 * @param mclcb
	 * @return	State
	 */
	mcl_tx_states	get_tx_state (class mcl_cb *const mclcb) const;
	mcl_rx_states	get_rx_state (class mcl_cb *const mclcb) const;

	/**
	 * Create a string for the current state.
	 * The buffer containing the string is statically allocated in
	 * the mcl_fsm class. Do not free it!
	 * @param	mclcb
	 * @return	String
	 */
	const char	*print_tx_state (class mcl_cb *const mclcb) const;
	const char	*print_rx_state (class mcl_cb *const mclcb) const;

	/**
	 * Create a string for the event.
	 * The buffer containing the string is statically allocated in
	 * the mcl_fsm class. Do not free it!
	 * @param mclcb
	 * @param event	tx (resp. rx) event to print
	 * @return	String
	 */
	const char	*print_tx_event (class mcl_cb *const mclcb, mcl_tx_events event) const;
	const char	*print_rx_event (class mcl_cb *const mclcb, mcl_rx_events event) const;

	/**
	 * Is the session (SENDER or RECEIVER) created, i.e. mcl_open() called?
	 * @param	mclcb
	 * @return	boolean
	 */
	bool	is_opened (mcl_cb *const mclcb) const;

	/**
	 * Can we send data on this session?
	 * @param	mclcb
	 * @return	boolean
	 */
	bool	can_send_data (mcl_cb *const mclcb) const;

	/**
	 * Is the receiver in a state where data can be received?
	 * Receiver specific function.
	 * @param	mclcb
	 * @return	boolean
	 */
	bool	can_recv_data (mcl_cb *const mclcb) const;

#if 0
	/**
	 * Are we in FINISH_TX state?
	 * Sender specific function.
	 * @param	mclcb
	 * @return	boolean
	 */
	bool	finish_tx (mcl_cb *const mclcb) const;
#endif

	/**
	 * Do we expect new ADUs or not?
	 * Sender and receiver function.
	 * @param	mclcb
	 * @return	boolean
	 */
	bool	no_new_adu (mcl_cb *const mclcb) const;

	/**
	 * Do we expect new DUs or do we have everything we need?
	 * Receiver specific function.
	 * @param	mclcb
	 * @return	boolean
	 */
	bool	no_new_undup_du (mcl_cb *const mclcb) const;

	/**
	 * Did we already receive a CLOSE sig from the source or did we
	 * already receive all the expected DUs and appli issued a close()?
	 * If yes, then we can ignore a new CLOSE sig.
	 * Receiver specific function.
	 * @param	mclcb
	 * @return	boolean
	 */
	bool	close_already_rx (mcl_cb *const mclcb) const;

	/**
	 * Should a call to mcl_close() return control to the application?
	 * Sender and receiver function.
	 * @param	mclcb
	 * @return	boolean
	 */
	bool	close_can_return (mcl_cb *const mclcb) const;

	/**
	 * Is the session (SENDER or RECEIVER) closed?
	 * @param	mclcb
	 * @return	boolean
	 */
	bool	is_closed (mcl_cb *const mclcb) const;

	/****** Public Attributes *********************************************/
  
private:
	/****** Private Members ***********************************************/
	/****** Private Attributes ********************************************/
	mcl_tx_states	tx_state;
	mcl_rx_states	rx_state;

};


//------------------------------------------------------------------------------
// Inlines for all classes follow
//------------------------------------------------------------------------------

inline mcl_tx_states
mcl_fsm::get_tx_state (class mcl_cb *const mclcb) const
{
	return this->tx_state;
}

inline mcl_rx_states
mcl_fsm::get_rx_state (class mcl_cb *const mclcb) const
{
	return this->rx_state;
}

#endif /* }  MCL_FSM_H */
