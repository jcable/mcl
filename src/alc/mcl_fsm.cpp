/* $Id: mcl_fsm.cpp,v 1.7 2005/01/11 13:12:28 roca Exp $ */
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


#include "mcl_includes.h"


/*
 * SENDER's FSM (Finite State Machine)
 *
 *                NIL
 *                 |
 *                 | appli issues mcl_open()
 *  mcl_abort()    v                                appli issues mcl_close()
 *  +----------- READY --------------------------------------------+
 *  |              |                                               |
 *  |              | first ADU submission                          |
 *  |              v                                               |
 *  |            IN_TX :   send NEW_ADU; send DU                   |
 *  |            | | |     ^                                       |
 *  |            | | |     |  continue to tx or wait new ADU       |
 *  +------------+ | +-----+                                       |
 *  |              |                                               |
 *  |              | appli issues mcl_close() and is blocked       |
 *  |              v                                               |
 *  |           FINISH_TO_TX :   send NO_NEW_ADU; send DU          |
 *  |            | | |     ^                                       |
 *  |            | | |     |  there are DUs remaining              |
 *  +------------+ | +-----+                                       |
 *  |              |                                               |
 *  |              | all DUs have been tx                          |
 *  |              |                                               |
 *  +------------+ | +---------------------------------------------+
 *               | | |                                                      
 *               v v v                                                      
 *                END :   send CLOSE                                        
 *                 |                                                        
 *                 | nothing else to do                                     
 *                 v                                                        
 *              CLOSED :   mcl_close() returns                              
 *                 |                                                        
 *                 v
 *                NIL
 */

/*static tx_states_t tx_state = TSTATE_NIL;*/	/* state in the FSM graph */

#define xx	TSTATE_INVALID	/* for clarity */
static const mcl_tx_states	next_tx_state[6/*state*/][9/*event*/] = {
	     /*	NIL	OPEN_CALLED	NEW_ADU		ALL_DU_SENT	CLOSE_CALLED	CLOSE_SENT	CLOSE_RETURN	ABORT 		RESET*/
     /* NIL */ {xx,	TSTATE_READY,	xx,		xx,		xx,		xx,		xx,		xx,		xx},
   /* READY */ {xx,	xx,		TSTATE_IN_TX,	xx,		TSTATE_END,	xx,		xx,		TSTATE_END,	TSTATE_READY},
   /* IN_TX */ {xx,	xx,		TSTATE_IN_TX,	xx,		TSTATE_FINISH_TX,xx,		xx,		TSTATE_END,	TSTATE_READY},
/* FINISH_TX*/ {xx,	xx,		xx,		TSTATE_END,	xx,		xx,		xx,		TSTATE_END,	TSTATE_READY},
     /* END */ {xx,	xx,		xx,		xx,		xx,		TSTATE_CLOSED,	xx,		TSTATE_END,	TSTATE_READY},
  /* CLOSED */ {xx,	xx,		xx,		xx,		xx,		xx,		TSTATE_NIL,	TSTATE_NIL,	xx}
};
#undef xx

/*
 * RECEIVER's FSM (Finite State Machine)
 *
 *                NIL
 *                 |
 *                 | appli issues mcl_open()
 *                 v
 *       recv +- READY
 * NO_NEW_ADU |  | | |
 *            |  | | |
 *          +----+ | +-------------------------------+
 *     recv | |    |                                 | appli calls
 *    CLOSE | |    | first NEW_ADU received          | mcl_close()
 *          | |    v                                 |
 *          | | IN_RX :                              |
 *          | | | | | |     ^                        |
 *          | | | | | |     | recv NEW_ADU; recv DU  |
 *          +---+ | | +-----+                        |
 *     recv | |   | +--------------------------------+
 *    CLOSE | |   |                                  | appli calls
 *          | +---+ recv NO_NEW_ADU from peer        | mcl_close()
 *          |     |                                  |
 *          |     v                                  |
 *          |   FINISH_RX :                          |
 *          |    | | | |     ^                       |
 *          |    | | | |     | recv DU; **NB_1**     |
 *          +----+ | | +-----+                       |
 *     recv |      | +-------------------------------+
 *    CLOSE |      |                                 | appli calls
 *          |      | all DUs have been received      | mcl_close()
 *          |      v                                 |
 *          |     END :                              |
 *          |     | | |     ^                        |
 *          |     | | |     | recv DU (ignored)      |
 *     recv +-----+ | +-----+                        |
 *    CLOSE |       |                                |
 *          v       |                                |
 *      CLOSE_RX    |                                |
 *          |       |                                |
 *          +-------+--------------------------------+
 *                  | appli calls mcl_close()
 *                  |
 *                  v
 *               CLOSED :   mcl_close() returns
 *                  | 
 *                  v
 *                 NIL
 *
 * State END:
 *	we have received everything and the sender will not send any new ADU
 *	we may still receive (duplicated) DUs but we can stop at any time
 *	this is the normal (almost) final state when everything is ok
 *
 * NB_1:
 * 	in FINISH_RX state, in FLUTE mode and in aggregated objects mode,
 * 	the reception of new FDT instance IDs (similarly new MOID), ie. for
 * 	TOI==0, is possible even if the FINISH_RX state traditionally
 * 	prevents it. This is managed by the callee (who does not call the
 * 	FSM update function), not by the present FSM update funtion itself.
 */

/*static rx_states_t rx_state = RSTATE_NIL;*/	/* state in the FSM graph */

#define xx	RSTATE_INVALID	/* for clarity */
static const mcl_rx_states	next_rx_state[7/*state*/][9/*event*/] = {
	     /*	NIL,	OPEN_CALLED	NEW_ADU		NO_NEW_ADU	DU_RECV		ALL_DU_RECV	CLOSE_CALLED	CLOSE_RECV	CLOSE_RETURN*/
     /* NIL */ {xx,	RSTATE_READY,	xx,		xx,		xx,		xx,		xx,		xx,		xx},
   /* READY */ {xx,	xx,		RSTATE_IN_RX,	RSTATE_FINISH_RX,xx,		xx,		RSTATE_CLOSED,	RSTATE_CLOSE_RX,xx},
   /* IN_RX */ {xx,	xx,		RSTATE_IN_RX,	RSTATE_FINISH_RX,RSTATE_IN_RX,	xx,		RSTATE_CLOSED,	RSTATE_CLOSE_RX,xx},
/* FINISH_RX*/ {xx,	xx,		xx,		RSTATE_FINISH_RX,RSTATE_FINISH_RX,RSTATE_END,	RSTATE_CLOSED,	RSTATE_CLOSE_RX,xx},
     /* END */ {xx,	xx,		xx,		RSTATE_END,	RSTATE_END,	xx,		RSTATE_CLOSED,	RSTATE_CLOSE_RX,xx},
  /*CLOSE_RX*/ {xx,	xx,		xx,		xx,		xx,		xx,		RSTATE_CLOSED,	RSTATE_CLOSE_RX, RSTATE_NIL},
  /* CLOSED */ {xx,	xx,		xx,		xx,		xx,		xx,		xx,		RSTATE_CLOSED,	RSTATE_NIL}
};
#undef xx


/*
 * strings for debug
 */
static const char * tx_states_msg[] = {
		"TSTATE_NIL",
		"TSTATE_READY",
		"TSTATE_IN_TX",
		"TSTATE_FINISH_TX",
		"TSTATE_END",
		"TSTATE_CLOSED",
		"*INVALID*"};

static const char * tx_events_msg[] = {
		"TEVENT_NIL",
		"TEVENT_OPEN_CALLED",
		"TEVENT_NEW_ADU",
		"TEVENT_ALL_DU_SENT",
		"TEVENT_CLOSE_CALLED",
		"TEVENT_CLOSE_SENT",
		"TEVENT_CLOSE_RETURN",
		"TEVENT_ABORT",
		"TEVENT_RESET"};

static const char * rx_states_msg[] = {
		"RSTATE_NIL",
		"RSTATE_READY",
		"RSTATE_IN_RX",
		"RSTATE_FINISH_RX",
		"RSTATE_END",
		"RSTATE_CLOSE_RX",
		"RSTATE_CLOSED",
		"*INVALID*"};

static const char * rx_events_msg[] = {
		"REVENT_NIL",
		"REVENT_OPEN_CALLED",
		"REVENT_NEW_ADU",
		"REVENT_NO_NEW_ADU",
		"REVENT_DU_RECV",
		"REVENT_ALL_DU_RECV",
		"REVENT_CLOSE_CALLED",
		"REVENT_CLOSE_RECV",
		"REVENT_CLOSE_RETURN",
		"REVENT_RESET"};


mcl_fsm::mcl_fsm ()
{
	this->tx_state = TSTATE_NIL;
	this->rx_state = RSTATE_NIL;
}


mcl_fsm::~mcl_fsm ()
{
	this->tx_state = TSTATE_NIL;
	this->rx_state = RSTATE_NIL;
}


void
mcl_fsm::update_tx_state (mcl_cb *const mclcb, mcl_tx_events event)
{
	mcl_tx_states	new_state;

	TRACELVL(5, (mcl_stdout, "-> mcl_fsm::update_tx_state: event=%s\n",
		this->print_tx_event(mclcb, event)))
	ASSERT (event >= TEVENT_OPEN_CALLED && event <= TEVENT_RESET);
	new_state = next_tx_state[this->tx_state][event];
	if (new_state == TSTATE_INVALID) {
		PRINT_ERR((mcl_stderr,
		"mcl_fsm::update_tx_state: ERROR, event %s invalid in state %s\n",
		this->print_tx_event(mclcb, event),
		this->print_tx_state(mclcb)))
		mcl_exit(1);
	}
	this->tx_state = new_state;
	TRACELVL(5, (mcl_stdout, "<- mcl_fsm::update_tx_state: new_state %s\n",
		this->print_tx_state(mclcb)))
}


void
mcl_fsm::update_rx_state (class mcl_cb *const mclcb, mcl_rx_events event)
{
	mcl_rx_states	new_state;

	TRACELVL(5, (mcl_stdout, "-> mcl_fsm::update_rx_state: event=%s\n",
		this->print_rx_event(mclcb, event)))
	ASSERT (event >= REVENT_OPEN_CALLED && event <= REVENT_CLOSE_RETURN);
	new_state = next_rx_state[this->rx_state][event];
	if (new_state == RSTATE_INVALID) {
		PRINT_ERR((mcl_stderr,
		"mcl_fsm::update_rx_state: ERROR, event %s invalid in state %s\n",
		this->print_rx_event(mclcb, event),
		this->print_rx_state(mclcb)))
		mcl_exit(1);
	}
	this->rx_state = new_state;
	TRACELVL(5, (mcl_stdout, "<- mcl_fsm::update_rx_state: new_state %s\n",
		this->print_rx_state(mclcb)))
}


const char *
mcl_fsm::print_tx_state (class mcl_cb *const mclcb) const
{
	ASSERT(this->tx_state >= TSTATE_NIL && this->tx_state <= TSTATE_INVALID);
	return tx_states_msg[this->tx_state];

}


const char *
mcl_fsm::print_rx_state (class mcl_cb *const mclcb) const
{
	ASSERT(this->rx_state >= RSTATE_NIL && this->rx_state <= RSTATE_INVALID);
	return rx_states_msg[this->rx_state];
}


const char *
mcl_fsm::print_tx_event (class mcl_cb *const mclcb, mcl_tx_events event) const
{
	ASSERT(event >= TEVENT_NIL && event <= TEVENT_RESET);
	return tx_events_msg[event];

}


const char *
mcl_fsm::print_rx_event (class mcl_cb *const mclcb, mcl_rx_events event) const
{
	ASSERT(event >= REVENT_NIL && event <= REVENT_CLOSE_RETURN);
	return rx_events_msg[event];
}


bool
mcl_fsm::is_opened (mcl_cb *const mclcb) const
{
	if (mclcb->is_a_sender()) {
		return (this->tx_state >= TSTATE_READY);
	} else if (mclcb->is_a_receiver()) {
		return (this->rx_state >= RSTATE_READY);
	}
	mcl_exit(-1);	// impossible, unsupported mode
	return false;	// to avoid warning
}


bool
mcl_fsm::can_send_data (mcl_cb *const mclcb) const
{
	ASSERT(mclcb->is_a_sender());
	return (this->tx_state == TSTATE_IN_TX ||
		this->tx_state == TSTATE_FINISH_TX);
}


bool
mcl_fsm::can_recv_data (mcl_cb	*const mclcb) const
{
	ASSERT(mclcb->is_a_receiver());
	return (this->rx_state == RSTATE_READY || /* req to wait for 1st NEW_ADU */
		this->rx_state == RSTATE_IN_RX ||
		this->rx_state == RSTATE_FINISH_RX);
}


#if 0
bool
mcl_fsm::finish_tx (mcl_cb	*const mclcb) const
{
	ASSERT(mclcb->is_a_sender());
	return(this->tx_state == TSTATE_FINISH_TX);
}
#endif


bool
mcl_fsm::no_new_adu (mcl_cb	*const	mclcb) const
{
	if (mclcb->is_a_sender()) {
		return (this->tx_state == TSTATE_FINISH_TX ||
			this->tx_state == TSTATE_END ||
			this->tx_state == TSTATE_CLOSED);
	} else if (mclcb->is_a_receiver()) {
		return (this->rx_state == RSTATE_FINISH_RX ||
			this->rx_state == RSTATE_END ||
			this->rx_state == RSTATE_CLOSE_RX ||
			this->rx_state == RSTATE_CLOSED);
	}
	mcl_exit(-1);	// impossible, unsupported mode
	return false;	// to avoid warning
}


bool
mcl_fsm::no_new_undup_du (mcl_cb	*const mclcb) const
{
	ASSERT(mclcb->is_a_receiver());
	return (this->rx_state == RSTATE_END ||
		this->rx_state == RSTATE_CLOSE_RX ||
		this->rx_state == RSTATE_CLOSED);
}


bool
mcl_fsm::close_already_rx (mcl_cb	*const mclcb) const
{
	ASSERT(mclcb->is_a_receiver());
	return (this->rx_state == RSTATE_CLOSE_RX ||
		this->rx_state == RSTATE_CLOSED);
}


bool
mcl_fsm::close_can_return (mcl_cb	*const mclcb) const
{
	if (mclcb->is_a_sender()) {
		return (this->tx_state == TSTATE_CLOSED);
	} else if (mclcb->is_a_receiver()) {
		return (this->rx_state == RSTATE_CLOSED);
	}
	mcl_exit(-1);	// impossible, unsupported mode
	return false;	// to avoid warning
}


bool
mcl_fsm::is_closed (mcl_cb *const mclcb) const
{
	if (mclcb->is_a_sender()) {
		return (this->tx_state == TSTATE_CLOSED ||
			this->tx_state == TSTATE_NIL);
	} else if (mclcb->is_a_receiver()) {
		return (this->rx_state == RSTATE_CLOSED ||
			this->rx_state == RSTATE_NIL);
	}
	mcl_exit(-1);	// impossible, unsupported mode
	return false;	// to avoid warning
}

