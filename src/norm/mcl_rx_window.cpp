/* $Id: mcl_rx_window.cpp,v 1.3 2004/01/30 16:27:43 roca Exp $ */
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

#include "mcl_includes.h"


/**
 * default constructor
 */
mcl_rx_window::mcl_rx_window ()
{
	memset(this, 0, sizeof(*this));
	this->delivery_mode_to_appli = MCL_IMMEDIATE_DELIVERY;
}


/**
 * default destructor
 */
mcl_rx_window::~mcl_rx_window ()
{
	/* TODO: free all remaining ADUs and DUs... */
	memset(this, 0, sizeof(*this));
}


/**
 * Set the delivery mode.
 */
mcl_error_status
mcl_rx_window::set_delivery_mode_to_appli (mcl_cb		*const mclcb,
					   mcl_delivery_mode_to_appli	mode)
{
	TRACELVL(5, (mcl_stdout, "-> mcl_cb::set_delivery_mode: %s",
		(mode == MCL_IMMEDIATE_DELIVERY) ? "IMMEDIATE_DELIVERY" :
						   "ORDERED_DELIVERY"))
	this->delivery_mode_to_appli = mode;
	return MCL_OK;
}


#if 0
/**
 * Register a new FEC or data DU in the receiving window.
 * @return Completion status (MCL_OK or MCL_ERROR).
 */
mcl_error_status
mcl_rx_window::insert_du (mcl_cb	*const mclcb,
			mcl_du	*const du)
{
	return MCL_OK;
}
#endif // 0

/**
 * Process an incoming ADU announcement and insert it in the list.
 * The ADU must not already be in list.
 * @param mclcb
 * @param hddr_infos	structure containing the new ADU infos (plus
 * 			many other info not used)
 * @return	completion status (MCL_OK or MCL_ERROR).
 */
mcl_error_status
mcl_rx_window::process_adu_announcement
				(mcl_cb			*const mclcb,
				 mcl_addr		* saddr,
				 mcl_data_hdr_infos_t	* dhdr_infos)
{
	mcl_adu		*adu;

	TRACELVL(5, (mcl_stdout, "-> mcl_rx_window::process_adu_announcement:\n"))
	ASSERT(this->find_adu(mclcb, dhdr_infos->idf_adu) == NULL);
	if (mclcb->get_verbosity() >= 1) {
		mcl_print_recvd_du(mclcb, false, MCL_SIG_NEWADU, dhdr_infos);
	}
	/*
	 * completely new ADU.
	 * update the state first...
	 */
	if (mclcb->fsm.no_new_adu(mclcb) == true) {
		/*
		 * no new ADU is expected, so check that the one announced
		 * has actually been implicitely announced, i.e. its idf is
		 * inferior or equal to the highest possible ADU id.
		 */
		if (dhdr_infos->max_idf_adu > mclcb->rx.get_highest_adu_seq_of_session()) {
			/* error, FSM state update will fail... */
			mclcb->fsm.update_rx_state(mclcb, REVENT_NEW_ADU);
		} /* else implicitely announced, do not update the FSM state */
	} else {
		/* normal case, we are still expecting new ADUs */
		mclcb->fsm.update_rx_state(mclcb, REVENT_NEW_ADU);
	}
	/*
	 * allocate/init the adu and all of its constituting blocks...
	 */
	adu	 = new mcl_adu (mclcb, dhdr_infos, saddr);
	/*
	 * insert the adu in the expected adu list
	 */
	this->insert_adu(mclcb, adu);

	mclcb->stats.adus_announced++;
	TRACELVL(5, (mcl_stdout, "<- mcl_rx_window::process_adu_announcement:\n"))
	return MCL_OK;
}


/**
 * Check if all ADUs have been completed.
 * @return	boolean
 */
bool
mcl_rx_window::check_if_all_adu_completed (mcl_cb	*const mclcb)
{
	mcl_adu		*adu;

	TRACELVL(5, (mcl_stdout, "-> mcl_rx_window::check_if_all_adu_completed:\n"))
	for (adu = this->adu_head->get_prev(); ; adu = adu->get_prev()) {
		if (adu->get_rx_status() < ADU_STATUS_COMPLETED) {
			TRACELVL(5, (mcl_stdout, "<- mcl_rx_window::check_if_all_adu_completed: no\n"))
			return false;
		}
		if (adu == this->adu_head) {
			TRACELVL(5, (mcl_stdout, "<- mcl_rx_window::check_if_all_adu_completed: yes\n"))
			return true;
		}
	}
}	


/**
 * @return	next ADU fully received and decoded, complying with the
 * 		reception mode (IMMEDIATE_DELIVERY/ORDERED_DELIVERY) of
 * 		the application, if any, NULL otherwise.
 */
mcl_adu *
mcl_rx_window::get_next_ready_adu (mcl_cb  *const mclcb)
{
	mcl_adu		*adu;

	TRACELVL(5, (mcl_stdout, "-> mcl_rx_window::get_next_ready_adu:\n"))
	if (this->nb_of_ready_adu <= 0) {
		/* nothing ready */
		TRACELVL(5, (mcl_stdout, "<- mcl_rx_window::get_next_ready_adu: no adu available\n"))
		return NULL;
	}
	if (this->use_immediate_delivery_to_appli()) {
		/*
		 * IMMEDIATE_DELIVERY
		 */
		/*
		 * there is necessarily a ready adu.
		 * search by increasing seq numbers, from the start...
		 */
		adu = this->adu_head;
		do {
			if (adu->get_rx_status() == ADU_STATUS_DECODED) {
				/* found a ready adu */
				break;
			}
			adu = adu->get_next();
		} while (adu != this->adu_head);
		ASSERT(adu->get_rx_status() == ADU_STATUS_DECODED);
	} else {
		/*
		 * ORDERED_DELIVERY
		 */
		ASSERT(0);	// not yet implemented!
#if 0	// not yet implemented!
		ASSERT(mclcb->ordered_delivery());
		if (this->next_ordered_adu_to_return != NULL) {
			/* the next adu to return to appli is already known */
			adu = mclcb->rxlvl.next_adu2give;
			ASSERT(adu->seq == seq);
			if (adu->rx_status != ADU_STATUS_DECODED) {
				TRACELVL(5, (mcl_stdout,
				"<- mcl_rx_window::get_next_ready_adu: next adu not ready (state=%d)\n", adu->rx_status))
				return -1;	/* not ready */
			}
			/* ready... continues after the else... */
		} else {
			/* search for a ready adu in the list */
			for (adu = mclcb->rxlvl.adu_head->next; ; adu = adu->next) {
				if (adu->seq == seq) {
					/* found the next adu */
					if (adu->rx_status != ADU_STATUS_DECODED) {
						TRACELVL(5, (mcl_stdout,
						"<- mcl_rx_window::get_next_ready_adu: next adu not ready 2 (state=%d)\n", adu->rx_status))
						mclcb->rxlvl.next_adu2give = adu;
						return -1;	/* not ready */
					} else {
						break;		/* ready */
						/* continues after the else...*/
					}
				}
				if (adu == mclcb->rxlvl.adu_head) {
					/* we have cycled => no adu ready */
					TRACELVL(5, (mcl_stdout, "<- mcl_rx_window::get_next_ready_adu: next adu not found\n"))
					return -1;
				}
			}
		}
#endif // 0
	}
	TRACELVL(5, (mcl_stdout,
		"<- mcl_rx_window::get_next_ready_adu: found adu seq=%d\n",
		adu->get_seq()))
	return adu;
}


/**
 * Update nb of ready adus plus the next adu to deliver info
 * (ORDERED_DELIVERY) and change the ADU status to delivered.
 */
void
mcl_rx_window::mark_adu_delivered(mcl_adu *adu)
{
	this->nb_of_ready_adu--;
	adu->set_rx_status(ADU_STATUS_DELIVERED);

	// TO DO ... 
#if 0
	mclcb->rxlvl.next_adu2give_seq++;
	if (n_adu && n_adu->seq == mclcb->rxlvl.next_adu2give_seq) {
		mclcb->rxlvl.next_adu2give = n_adu;
	} else {
		mclcb->rxlvl.next_adu2give = NULL;
	}
#endif	
}
