/* $Id: mcl_tx_window.cpp,v 1.3 2004/02/18 07:56:18 roca Exp $ */
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
mcl_tx_window::mcl_tx_window ()
{
	memset((void*)this, 0, sizeof(*this));
	this->blist_size = DFLT_BLIST_SIZE;		// in blocks
	this->seq_tx_in_hp_count = MAX_SEQ_TX_FROM_HP;
}


/**
 * default destructor
 */
mcl_tx_window::~mcl_tx_window ()
{
}


/**
 * Is it possible to register a new ADU?
 * Performs flow control between the upper application and MCL.
 * For simplicity, does not take into account the ADU data size,
 * it just checks if there is any room left.
 * This is not required for DU registration, since they are
 * generated internally by MCL, not by the application.
 * @return	boolean
 */
bool
mcl_tx_window::can_register_new_adu (mcl_cb *const mclcb)
{
	if (mclcb->fsm.no_new_adu(mclcb)) {
		// not possible, mcl_close() already called.
		return false;
	}
	return (this->blist_eff_size < this->blist_size);
}


/**
 * Register a new adu (i.e. all of its data DUs and FEC DUs if any)
 * for transmission in the sending window.
 * The DUs are inserted in a NORMAL PRIORITY list.
 * @return Completion status (MCL_OK or MCL_ERROR).
 */
mcl_error_status
mcl_tx_window::register_new_adu (mcl_cb *const mclcb,
				 mcl_adu *const adu)
{
	mcl_block	*blk;
	mcl_du		*du;
	INT32		b_rem;		// remaining nb of blocks to process
	INT32		d_rem;		// remaining nb of DUs to process

	TRACELVL(5, (mcl_stdout, "-> mcl_tx_window::register_new_adu:\n"))
	for (b_rem = adu->get_block_nb(), blk = adu->get_block_head();
	     b_rem > 0; b_rem--, blk++) {
		ASSERT((blk))
		/* register data DUs */
		for (d_rem = blk->du_nb, du = blk->get_du_head();
		     d_rem > 0; d_rem--, du++) {
			ASSERT(du);
			this->insert_in_list(mclcb, du);
		}
#if 0
		/* register FEC DUs (if any) */
		for (d_rem = blk->get_fec_du_nb_in_list(), du = blk->get_fec_du_head();
		     d_rem > 0; d_rem--, du = du->get_next()) {
			ASSERT(du);
			this->insert_in_list(mclcb, du);
		}
#endif
	}
	// TODO: blist management...
	this->blist_eff_size += adu->get_block_nb();
	TRACELVL(5, (mcl_stdout, "<- mcl_tx_window::register_new_adu: ok\n"))
	return MCL_OK;
}


/**
 * Register a new FEC du for transmission in the sending window.
 * @return Completion status (MCL_OK or MCL_ERROR).
 */
mcl_error_status
mcl_tx_window::register_new_du (mcl_cb *const mclcb,
				mcl_du *const du)
{
	TRACELVL(5, (mcl_stdout, "-> mcl_tx_window::register_new_du:\n"))
	this->insert_in_hp_list(mclcb, du);
	TRACELVL(5, (mcl_stdout, "<- mcl_tx_window::register_new_du: ok\n"))
	return MCL_OK;
}


/**
 * Reschedule transmissions (data and/or FEC DUs) for a block.
 * Reschedule the transmission of all available FEC, and if not
 * sufficient, retransmit data.
 * @param mclcb
 * @param blk		block that must be retransmitted entirely
 * @param data_required	nb of data DUs to transmit (>= 0)
 * @param fec_required	nb of FEC DUs to transmit (>= 0)
 */
mcl_error_status
mcl_tx_window::reschedule_tx_for_block (mcl_cb		*const mclcb,
					mcl_block	*blk,
					INT32		data_required,
					INT32		fec_required)
{
	mcl_du		*du;
	INT32		d_rem;		// remaining nb of DUs to process
	INT32		fec_to_tx;	// nb of FEC DUs to tx immediately

	TRACELVL(1, (mcl_stdout,
		"-> mcl_tx_window::reschedule_tx_for_block: requires %d/%d DUs for blk %d/%d\n",
		data_required, fec_required, blk->adu->get_seq(), blk->seq))
	ASSERT(blk);
	ASSERT(data_required <= blk->du_nb);
	if (data_required > 0) {
		/* register data DUs */
		for (d_rem = blk->du_nb, du = blk->get_du_head();
		     d_rem > 0; d_rem--, du++) {
			ASSERT(du);
			this->insert_in_list(mclcb, du);
		}
	}
	if (fec_required > 0) {
		/*
		 * calculate how many FEC DUs to create for future tx and
		 * to tx immediately
		 */
		if (fec_required > blk->get_fec_du_nb_in_list()) {
			/* not enough FEC available, schedule creation of new ones */
			// TODO:
			fec_to_tx = blk->get_fec_du_nb_in_list();
		} else
			fec_to_tx = fec_required;
		/* register FEC DUs (if any) */
		for (d_rem = fec_to_tx, du = blk->get_fec_du_head();
		     d_rem > 0; d_rem--, du = du->get_next()) {
			ASSERT(du);
			this->insert_in_list(mclcb, du);
		}
	}
	TRACELVL(5, (mcl_stdout, "<- mcl_tx_window::reschedule_tx_for_block:\n"))
	return MCL_OK;
}


/**
 * Remove a block from the transmission window.
 * Called usually when the block has been sent reliably (e.g.
 * at the end of a FLUSH_DONE process).
 * @param mclcb
 * @param blk	block to remove
 * @param fast_mode	in fast mode it is assumed there is no pending
 * 			DU transmission for this block
 * @return	completion status (MCL_OK or MCL_ERROR).
 */
mcl_error_status
mcl_tx_window::remove_block (mcl_cb	*const mclcb,
			     mcl_block	*const blk,
			     bool	fast_mode)
{
	UINT32		d_rem;	// remaining nb of DUs to process
	mcl_du		*du;

	TRACELVL(5, (mcl_stdout, "-> mcl_tx_window::remove_block:\n"))
	if (fast_mode == false) {
		/*
		 * go through the whole list, and if any pending transmission
		 * for this block exist, remove it.
		 */
		for (d_rem = blk->du_nb, du = blk->get_du_head();
		     d_rem > 0; d_rem--, du++) {
			ASSERT(du);
			if (this->remove_from_list(mclcb, du) == MCL_ERROR) {
				/* maybe it's on the HP list, try to remove */
				this->remove_from_hp_list(mclcb, du);
			}
		}
		for (d_rem = blk->get_fec_du_nb_in_list(), du = blk->get_fec_du_head();
		     d_rem > 0; d_rem--, du++) {
			ASSERT(du);
			this->remove_from_hp_list(mclcb, du);
		}
	}
	/*
	 * unregister...
	 */
	this->blist_eff_size--;
	TRACELVL(5, (mcl_stdout,
		"<- mcl_tx_window::remove_block: blist_eff_sz=%d, available room=%d blks\n",
		this->blist_eff_size, this->blist_size - this->blist_eff_size))
	return MCL_OK;
}


/**
 * Returns the next DU to send, in either of the two lists.
 * This DU is automatically removed from the associated list.
 */
mcl_du *
mcl_tx_window::get_next_du_to_tx (mcl_cb *const mclcb)
{
	mcl_du	*du;

	/*
	 * start with the HP list...
	 */
	if (!(this->hp_du_list_head)) {
		/* nothing available... */
		goto try_other_du_list;
	}
	/*
	 * serve MAX_SEQ_TX_FROM_HP pkts from HP list, then 1 from normal
	 * list (if one is available, otherwise stay with the HP list).
	 * Doing so avoids starvation.
	 */
	if ((this->seq_tx_in_hp_count <= 0) && (this->du_list_head)) {
		/* switch to the normal list */
		this->seq_tx_in_hp_count = this->MAX_SEQ_TX_FROM_HP;
		goto try_other_du_list;
	}
	this->seq_tx_in_hp_count = max(this->seq_tx_in_hp_count - 1, 0);
	/* remove from start of list */
	du = this->hp_du_list_head;
	this->nb_of_du_in_hp_list--;
	if (this->nb_of_du_in_hp_list >= 1) {
		this->hp_du_list_head = du->get_tx_next();
		du->get_tx_next()->set_tx_prev(du->get_tx_prev());
		du->get_tx_prev()->set_tx_next(du->get_tx_next());
	} else {
		/* list turns empty */
		this->hp_du_list_head = NULL;
	}
	ASSERT( (this->nb_of_du_in_hp_list > 0 && this->hp_du_list_head) ||
		(this->nb_of_du_in_hp_list == 0 && this->hp_du_list_head == NULL));
	TRACELVL(5, (mcl_stdout,
		"   mcl_tx_window::get_next_du_to_tx: HP list, du seq=%d\n", du->seq))
	return du;

try_other_du_list:
	/*
	 * and if needed continue with normal priority list...
	 */
	if (!(this->du_list_head)) {
		/* nothing available... */
		TRACELVL(5, (mcl_stdout,
			"   mcl_tx_window::get_next_du_to_tx: none\n"))
		return NULL;
	}
	/* remove from start of list */
	du = this->du_list_head;
	this->nb_of_du_in_list--;
	if (this->nb_of_du_in_list >= 1) {
		this->du_list_head = du->get_tx_next();
		du->get_tx_next()->set_tx_prev(du->get_tx_prev());
		du->get_tx_prev()->set_tx_next(du->get_tx_next());
	} else {
		/* list turns empty */
		this->du_list_head = NULL;
	}
	ASSERT( (this->nb_of_du_in_list > 0 && this->du_list_head) ||
		(this->nb_of_du_in_list == 0 && this->du_list_head == NULL));
	TRACELVL(5, (mcl_stdout,
		"   mcl_tx_window::get_next_du_to_tx: normal list, du seq=%d\n", du->seq))
	return du;
}


void
mcl_tx_window::insert_in_list (mcl_cb *const mclcb,
			       mcl_du *const du)
{
	mcl_du		*last;	// temp pointer to last du in list

	TRACELVL(5, (mcl_stdout,
		"-> mcl_tx_window::insert_in_list: du=x%x, seq=%d\n",
		(int)du, du->seq))
	this->nb_of_du_in_list++;
	if (!(this->du_list_head)) {
		/*
		 * first du in list
		 */
		ASSERT(!(this->du_list_snd_max));
		this->du_list_head = du;
		du->set_tx_next(du);
		du->set_tx_prev(du);
		TRACELVL(5, (mcl_stdout,
			"<- mcl_tx_window::insert_in_list: ok, inserted in empty list\n"))
		return;
	}
	/*
	 * add at end of list
	 */
	last = this->du_list_head->get_tx_prev();
	ASSERT(last);
	last->set_tx_next(du);
	du->set_tx_prev(last);
	du->set_tx_next(this->du_list_head);
	this->du_list_head->set_tx_prev(du);

	TRACELVL(5, (mcl_stdout, "<- mcl_tx_window::insert_in_list:\n"))
}


mcl_error_status
mcl_tx_window::remove_from_list (mcl_cb *const mclcb,
				 mcl_du *const du)
{
	mcl_du		*next, *prev;	// temp pointers
	UINT32		d_rem;		// remaining nb of DUs to process

	TRACELVL(5, (mcl_stdout,
		"-> mcl_tx_window::remove_from_list: du=x%x, seq=%d\n",
		(int)du, du->seq))
	if (!(next = this->du_list_head)) {
		/* no du in list */
		TRACELVL(5, (mcl_stdout,
			"<- mcl_tx_window::remove_from_list: ERROR, empty list\n"))
		return MCL_ERROR;
	}
	/*
	 * search in entire list
	 */
	for (d_rem = this->nb_of_du_in_list; d_rem > 0;
	     d_rem--, next = next->get_tx_next()) {
		if (next == du) {
			/* found */
			prev = du->get_prev();
			next = du->get_next();
			prev->set_tx_next(next);
			next->set_tx_prev(prev);
			if (du == this->du_list_head)
				this->du_list_head = next;
			if (du == this->du_list_snd_max)
				this->du_list_snd_max = next;
			this->nb_of_du_in_list--;
			TRACELVL(5, (mcl_stdout,
				"<- mcl_tx_window::remove_from_list: found and removed\n"))
			return MCL_OK;
		}
	}
	/* du not found in list */
	TRACELVL(5, (mcl_stdout,
		"<- mcl_tx_window::remove_from_list: ERROR, not found in list\n"))
	return MCL_ERROR;
}


void
mcl_tx_window::insert_in_hp_list (mcl_cb *const mclcb,
				  mcl_du *const du)
{
	mcl_du		*last;	// temp pointer to last du in list

	TRACELVL(5, (mcl_stdout,
		"-> mcl_tx_window::insert_in_hp_list: du=x%x, seq=%d\n",
		(int)du, du->seq))
	this->nb_of_du_in_hp_list++;
	if (!(this->hp_du_list_head)) {
		/*
		 * first du in list
		 */
		ASSERT(!(this->hp_du_list_snd_max));
		this->hp_du_list_head = du;
		du->set_tx_next(du);
		du->set_tx_prev(du);
		TRACELVL(5, (mcl_stdout,
			"<- mcl_tx_window::insert_in_hp_list: ok, inserted in empty list\n"))
		return;
	}
	/*
	 * add at end of list
	 */
	last = this->hp_du_list_head->get_tx_prev();
	ASSERT(last);
	last->set_tx_next(du);
	du->set_tx_prev(last);
	du->set_tx_next(this->hp_du_list_head);
	this->hp_du_list_head->set_tx_prev(du);

	TRACELVL(5, (mcl_stdout, "<- mcl_tx_window::insert_in_hp_list:\n"))
}


mcl_error_status
mcl_tx_window::remove_from_hp_list (mcl_cb *const mclcb,
				 mcl_du *const du)
{
	mcl_du		*next, *prev;	// temp pointers
	UINT32		d_rem;		// remaining nb of DUs to process

	TRACELVL(5, (mcl_stdout,
		"-> mcl_tx_window::remove_from_hp_list: du=x%x, seq=%d\n",
		(int)du, du->seq))
	if (!(next = this->hp_du_list_head)) {
		/* no du in hp_list */
		TRACELVL(5, (mcl_stdout,
			"<- mcl_tx_window::remove_from_hp_list: ERROR, empty list\n"))
		return MCL_ERROR;
	}
	/*
	 * search in entire hp_list
	 */
	for (d_rem = this->nb_of_du_in_hp_list; d_rem > 0; d_rem--, next = next->get_tx_next()) {
		if (next == du) {
			/* found */
			prev = du->get_prev();
			next = du->get_next();
			prev->set_tx_next(next);
			next->set_tx_prev(prev);
			if (du == this->hp_du_list_head)
				this->hp_du_list_head = next;
			if (du == this->hp_du_list_snd_max)
				this->hp_du_list_snd_max = next;
			this->nb_of_du_in_hp_list--;
			TRACELVL(5, (mcl_stdout,
				"<- mcl_tx_window::remove_from_hp_list: found and removed\n"))
			return MCL_OK;
		}
	}
	/* du not found in hp_list */
	TRACELVL(5, (mcl_stdout,
		"<- mcl_tx_window::remove_from_hp_list: ERROR, not found in list\n"))
	return MCL_ERROR;
}
