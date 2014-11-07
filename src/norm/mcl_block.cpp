/* $Id: mcl_block.cpp,v 1.6 2004/03/09 15:16:21 roca Exp $ */
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
 * default destructor
 */
mcl_block::~mcl_block ()
{
	this->remove_and_free_all_data_dus(NULL);
	this->remove_and_free_all_fec_dus(NULL);
}


bool
mcl_block::check_if_completed_and_process (mcl_cb	*const mclcb)
{
	INT32	nb;     /* total nb of DUs received */

	if (this->get_rx_status() >= BLK_STATUS_COMPLETED) {
		TRACELVL(5, (mcl_stdout,
		"   mcl_block::check_if_completed_and_process: already completed\n"))
		return true;
	}
	ASSERT(this->get_rx_status() == BLK_STATUS_IN_PROGRESS);
	nb = this->du_in_list + this->fec_du_in_list;
	if (nb >= this->du_nb) {
		if (mclcb->rx_window.postpone_fec_decoding) {
			/* ok, we rx enough data or FEC DU, just mark it */
			this->rx_status = BLK_STATUS_COMPLETED;
		} else {
			/* ok, we rx enough data or FEC DU, so decode */
			mclcb->fec.decode(mclcb, this);
			this->rx_status = BLK_STATUS_DECODED;
		}
		TRACELVL(5, (mcl_stdout,
			"   mcl_block::check_if_completed_and_process: yes\n"))
		return true;
	} else {
		TRACELVL(5, (mcl_stdout,
			"   mcl_block::check_if_completed_and_process: %d DUs expected, %d recvd\n",
			this->du_nb, nb))
		return false;
	}
}


/**
 * Check if this block is completed and return the number of missing
 * DUs if any.
 * Used by a receiver, in particular during NACK/ACK processing.
 * @param mclcb
 * @return	number of missing DUs, or 0 if already completed.
 */
INT32	
mcl_block::get_number_of_missing_dus (mcl_cb	*const mclcb)
{
	INT32	missing;     /* nb of missing DUs */

	if (this->get_rx_status() >= BLK_STATUS_COMPLETED) {
		TRACELVL(5, (mcl_stdout,
		"   mcl_block::get_number_of_missing_dus: already completed\n"))
		return 0;
	}
	missing = this->du_nb - (this->du_in_list + this->fec_du_in_list);
	if (missing <= 0) {
		/* for an unknown reason the block status has not been
		 * updated, so do it */
		this->check_if_completed_and_process(mclcb);
		TRACELVL(5, (mcl_stdout,
		"   mcl_block::get_number_of_missing_dus: already completed\n"))
		return 0;
	}
	TRACELVL(5, (mcl_stdout,
		"   mcl_block::get_number_of_missing_dus: %d DUs expected, %d missing\n",
		this->du_nb, missing))
	return missing;
}


void
mcl_block::remove_and_free_all_data_dus (mcl_cb	*const mclcb)
{

	INT32	rem;		// remaining nb of DUs that need to be free'ed
	mcl_du	*du;		// DU being free'ed
	mcl_du	*ndu;		// next DU to free

	if ((rem = this->get_du_nb_in_list()) <= 0) {
		/* nothing in list */
		return;
	}
	du = this->du_head;
	ASSERT(du);
	if (mclcb) {
		TRACELVL(5, (mcl_stdout, "-> mcl_block::remove_and_free_all_data_dus: %d to free\n", this->get_du_nb_in_list()))
		/* update stats simply, assuming all DUs of same size */
		mclcb->stats.buf_space -= rem * du->len;
	}
	for (; rem > 0; rem--, du = ndu) {
		ASSERT(du);
		ndu = du->get_next();
		delete du;
	}
	this->du_head = NULL;
	this->du_in_list = 0;
	if (mclcb) {
		TRACELVL(5, (mcl_stdout, "<- mcl_block::remove_and_free_all_data_dus:\n"));
	}
}


/**
 * Remove and free all FEC DUs of the list.
 * Can be called both by a sender or a receiver.
 */
void
mcl_block::remove_and_free_all_fec_dus (mcl_cb	*const mclcb)
{
	INT32	rem;		// remaining nb of DUs that need to be free'ed
	mcl_du	*du;		// DU being free'ed
	mcl_du	*ndu;		// next DU to free

	if ((rem = this->get_fec_du_nb_in_list()) <= 0) {
		/* nothing in list */
		return;
	}
	du = this->fec_du_head;
	ASSERT(du);
	if (mclcb) {
		TRACELVL(5, (mcl_stdout,
		"-> mcl_block::remove_and_free_all_fec_dus: %d to free\n", rem))
		/* update stats simply, assuming all DUs of same size */
		mclcb->stats.buf_space -= rem * du->len;
	}
	for (; rem > 0; rem--, du = ndu) {
		ndu = du->get_next();
		delete du;
	}
	this->fec_du_head = NULL;
	this->fec_du_in_list = 0;
	if (mclcb) {
		TRACELVL(5, (mcl_stdout, "<- mcl_block::remove_and_free_all_fec_dus:\n"));
	}
}

