/* $Id: mcl_tx_ctrl.cpp,v 1.3 2004/02/18 07:56:18 roca Exp $ */
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
 * Default constructor.
 */
mcl_tx_ctrl:: mcl_tx_ctrl()
{
	this->blk_of_last_du = NULL;
	this->FLUSH_DONE_tevent_head = NULL;
	this->RETX_DONE_tevent_head = NULL;
}


/**
 * See if any control packet must be sent or not, and send it if
 * necessary.
 * @param mclcb
 * @param du	DU that has just been sent
 * @param end_session	boolean true after the transmission of
 * 		the last DU of the last block of a session.
 * 		Used to trigger the tx of a FLUSH
 */
void
mcl_tx_ctrl::check_if_ctrl_pkt_must_be_sent (mcl_cb	*const mclcb,
					     mcl_du	*du,
					     bool	end_session)
{
	TRACELVL(5, (mcl_stdout,
		"-> mcl_tx_ctrl::check_if_ctrl_pkt_must_be_sent:\n"))
	if (end_session == false) {
		/*
		 * normal case: a DU has just been sent, see if we need
		 * to send a FLUSH.
		 */
		ASSERT(du);
		if (du->is_fec) {
			/* ignore */
			TRACELVL(5, (mcl_stdout,
				"<- mcl_tx_ctrl::check_if_ctrl_pkt_must_be_sent: ignored (FEC DU)\n"))
			return;
		}
		if (du->block == this->blk_of_last_du || this->blk_of_last_du == NULL) {
			/* do nothing... */
			this->blk_of_last_du = du->block;
			TRACELVL(5, (mcl_stdout,
				"<- mcl_tx_ctrl::check_if_ctrl_pkt_must_be_sent: nothing done\n"))
			return;
		}
	}
	/*
	 * we need to send a FLUSH.
	 * So create and send a FLUSH packet for the previous block.
	 */
	this->send_flush_and_register_tevent(mclcb, this->blk_of_last_du);
	/*
	 * finish now
	 */
	if (end_session) {
		TRACELVL(5, (mcl_stdout,
		"<- mcl_tx_ctrl::check_if_ctrl_pkt_must_be_sent: end of session, FLUSH sent\n"))
		return;
	} else if (this->blk_of_last_du->adu == du->block->adu) {
		/*
		 * same ADU, so end of processing
		 */
		this->blk_of_last_du = du->block;
		TRACELVL(5, (mcl_stdout,
		"<- mcl_tx_ctrl::check_if_ctrl_pkt_must_be_sent: moved to new block, FLUSH sent\n"))
		return;
	} else {
		/*
		 * transmission for a new ADU
		 * mark the previous ADU as in its final step...
		 */
		this->blk_of_last_du->adu->set_tx_status(ADU_TSTATUS_FINISH_TX);
		this->blk_of_last_du = du->block;
		TRACELVL(5, (mcl_stdout,
		"<- mcl_tx_ctrl::check_if_ctrl_pkt_must_be_sent: moved to new ADU\n"))
		return;
	}
}


/**
 * Send NORM_CMD(FLUSH) packet(s) and register a FLUSH_DONE
 * timer event.
 * @param mclcb
 * @param blk	block for which we need to send a FLUSH
 */
void
mcl_tx_ctrl::send_flush_and_register_tevent (mcl_cb	*const mclcb,
					     mcl_block	*blk)
{
	mcl_tx_ctrl_FLUSH_DONE_tevent	*te,		// FLUSH timer event
					*prev_te;	// previous timer event

	TRACELVL(5, (mcl_stdout, "-> mcl_tx_ctrl::send_flush_and_register_tevent:\n"))
	/*
	 * create and send a FLUSH packet for this block (pick
	 * up any of its du, e.g. the first of list)
	 */
	this->send_cmd_pkt(mclcb, NORM_CMD_FLUSH, blk->get_du_head());
	/*
	 * create a new FLUSH_DONE timer event
	 */
	te = new mcl_tx_ctrl_FLUSH_DONE_tevent(blk->adu->get_seq(), blk->seq);
	/*
	 * check if a FLUSH_DONE tevent already exists, and if yes
	 * delete the previous event and replace if with the new one.
	 */
	prev_te = te->find_in_list(mclcb, &(this->FLUSH_DONE_tevent_head));
	if (prev_te) {
		/* already present, so drop the previous one */
		TRACELVL(5, (mcl_stdout,
			"   mcl_tx_ctrl::send_flush_and_register_tevent: replace old FLUSH_DONE tevent by the new one\n"))
		/* remove the FLUSH_DONE from list */
		if (prev_te->remove_from_list(mclcb,
				&(this->FLUSH_DONE_tevent_head)) == MCL_ERROR) {
			PRINT_ERR((mcl_stderr,
			"mcl_tx_ctrl::check_if_ctrl_pkt_must_be_sent: remove_from_list() failed\n"))
			mcl_exit(-1);
		}
		/* and cancel the timer */
		if (mclcb->timer.cancel_timer((mcl_timer_handler *)this,
		    (INT32)mclcb, (INT32)prev_te) == MCL_ERROR) {
			PRINT_ERR((mcl_stderr,
			"mcl_tx_ctrl::check_if_ctrl_pkt_must_be_sent: cancel_timer() failed for FLUSH_DONE event, blk %d/%d\n",
			blk->adu->get_seq(), blk->seq))
			// do not exit but do not delete the tevent!
		} else {
			/* now we can delete the tevent */
			delete prev_te;
		}
	}
	/*
	 * insert it in the tevent list and set a timer for
	 * a delayed FLUSH_DONE management...
	 */
	te->insert_in_list(mclcb, &(this->FLUSH_DONE_tevent_head));
	mclcb->timer.set_timer(FLUSH_DONE_TIMEOUT, (mcl_timer_handler *)this,
				(INT32)mclcb, (INT32)te);
	TRACELVL(5, (mcl_stdout, "<- mcl_tx_ctrl::send_flush_and_register_tevent:\n"))
}


/**
 * Send NORM_CMD(FLUSH) packet(s).
 * @param mclcb
 * @param blk	block for which we need to send a FLUSH
 */
void
mcl_tx_ctrl::send_flush_pkt (mcl_cb	*const mclcb,
			     mcl_block	*blk)
{
	TRACELVL(5, (mcl_stdout, "-> mcl_tx_ctrl::send_flush_pkt:\n"))
	/*
	 * create and send a FLUSH packet for the previous block (pick
	 * up any of its du, e.g. the first of list)
	 */
	this->send_cmd_pkt(mclcb, NORM_CMD_FLUSH, blk->get_du_head());
	TRACELVL(5, (mcl_stdout, "<- mcl_tx_ctrl::flush_pkt:\n"))
}


/**
 * Send NORM_CMD(NO_NEW_OBJECT) packet(s).
 */
void
mcl_tx_ctrl::send_no_new_object_pkt (mcl_cb	*const mclcb)
{
	TRACELVL(5, (mcl_stdout, "-> mcl_tx_ctrl::send_no_new_object_pkt:\n"))
	this->send_cmd_pkt(mclcb, NORM_CMD_NO_NEW_OBJECT, NULL);
	TRACELVL(5, (mcl_stdout, "<- mcl_tx_ctrl::send_no_new_object_pkt:\n"))
}


/**
 * Send NORM_CMD(CLOSE) packet(s).
 */
void
mcl_tx_ctrl::send_close_pkt (mcl_cb	*const mclcb)
{
	TRACELVL(5, (mcl_stdout, "-> mcl_tx_ctrl::send_close_pkt:\n"))
	this->send_cmd_pkt(mclcb, NORM_CMD_CLOSE, NULL);
	TRACELVL(5, (mcl_stdout, "<- mcl_tx_ctrl::send_close_pkt:\n"))
}


/**
 * Send NORM_CMD(any flavor) packet(s).
 * Sends MCL_TX_ROBUSTNESS_FACTOR copies of the packet.
 * Used by send_no_new_object_pkt() and send_close_pkt() functions.
 * @param mclcb
 * @param flavor	flavor of the command packet
 * @param du		only required for NORM_CMD_FLUSH
 */
void
mcl_tx_ctrl::send_cmd_pkt (mcl_cb		*const mclcb,
			   norm_cmd_flavor	flavor,
			   mcl_du		*du)
{
	norm_common_hdr_t	*hdr;		// norm header
	INT32			hlen = 0;	// norm header length
	INT32			robustness;	// number of tx

	TRACELVL(5, (mcl_stdout, "-> mcl_tx_ctrl::send_cmd_pkt:\n"))
	/* create the NORM packet */
	switch (flavor) {
	case NORM_CMD_NO_NEW_OBJECT:
		if ((hlen = mclcb->norm_pkt_mgmt.create_cmd_no_new_object_hdr
							(mclcb, &hdr)) <= 0) {
			PRINT_ERR((mcl_stderr,
			"mcl_tx_ctrl::send_cmd_pkt: norm_pkt_mgmt.create_cmd_no_new_object_hdr() failed\n"))
			ASSERT(0);	// in debug mode, stop immediately...
			return;
		}
		break;

	case NORM_CMD_CLOSE:
		if ((hlen = mclcb->norm_pkt_mgmt.create_cmd_close_hdr
							(mclcb, &hdr)) <= 0) {
			PRINT_ERR((mcl_stderr,
			"mcl_tx_ctrl::send_cmd_pkt: norm_pkt_mgmt.create_cmd_close_hdr() failed\n"))
			ASSERT(0);	// in debug mode, stop immediately...
			return;
		}
		break;

	case NORM_CMD_FLUSH:
		ASSERT(du);
		if ((hlen = mclcb->norm_pkt_mgmt.create_cmd_flush_hdr
					(mclcb, &hdr, du)) <= 0) {
			PRINT_ERR((mcl_stderr,
				"mcl_tx_ctrl::send_cmd_pkt: norm_pkt_mgmt.create_cmd_flush_hdr() failed\n"))
			ASSERT(0);	// in debug mode, stop immediately...
			return;
		}
		break;

	default:
		mcl_exit(-1);
	}

	/* then send the packet */
	switch (flavor) {
	case NORM_CMD_NO_NEW_OBJECT:
	case NORM_CMD_CLOSE:
		for (robustness = MCL_TX_ROBUSTNESS_FACTOR; robustness > 0;
		     robustness--) {
			mclcb->ses_channel.send_pkt(mclcb, hdr, hlen, NULL);
			if (mclcb->get_verbosity() >= 1) {
				switch (flavor) {
				case NORM_CMD_NO_NEW_OBJECT:
					PRINT_OUT((mcl_stdout, "\tNO_NEW_OBJECT\n"))
					break;
				case NORM_CMD_CLOSE:
					PRINT_OUT((mcl_stdout, "\tCLOSE\n"))
					break;
				default:
					mcl_exit(-1);
				}
			}
		}
		break;

	case NORM_CMD_FLUSH:
		for (robustness = MCL_TX_CMD_FLUSH_ROBUSTNESS_FACTOR;
		     robustness > 0; robustness--) {
			mclcb->ses_channel.send_pkt(mclcb, hdr, hlen, NULL);
			PRINT_LVL(1, (mcl_stdout,"\tFLUSH sent for blk %d/%d\n",
					du->block->adu->get_seq(), du->seq))
		}
		break;

	default:
		mcl_exit(-1);
	}

	TRACELVL(5, (mcl_stdout, "<- mcl_tx_ctrl::send_cmd_pkt:\n"))
}


/**
 * Process an incoming NORM_NACK packet.
 * @param mclcb
 * @param pkt           pointer to packet
 * @param saddr         source address for this packet
 * @param chdr_infos    pointer to the common_infos struct
 */
mcl_error_status
mcl_tx_ctrl::process_nack_pkt  (mcl_cb			*const mclcb,
				class mcl_rx_pkt	*pkt,
				mcl_addr		*saddr,
				mcl_common_hdr_infos_t	*chdr_infos)
{
	mcl_data_hdr_infos_t	dhdr_infos; // infos of NACK hdr, in host format
	norm_nack_content_hdr_t	*nc;	// NACK content hdr, in host format
	mcl_adu			*adu;	// adu concerned by the NACK 
	mcl_block		*blk;	// block concerned by NACK if applicable
	INT32			fec_required;	// # fec req. for repair, i.e.
						// asked by the receiver
	INT32			fec_avail;	// # fec already available but
						// not yet sent
	INT32			add_possible_fec;// # fec that can still be
						// created
	INT32			fec_nb;	// # fec packets actually created and
					// that will be sent
	mcl_tx_ctrl_RETX_DONE_tevent	*te; // timer event
	mcl_tx_ctrl_FLUSH_DONE_tevent	*tmp_flush_te;	// temporary tevent
	mcl_tx_ctrl_FLUSH_DONE_tevent	*prev_flush_te;	// FLUSH timer event 
					// that triggered this NACK


	TRACELVL(5, (mcl_stdout, "-> mcl_tx_ctrl::process_nack_pkt:\n"))
	/*
	 * process the NORM_NACK header
	 */
	memset(&dhdr_infos, 0, sizeof(mcl_data_hdr_infos_t));
	if (mclcb->norm_pkt_mgmt.parse_nack_hdr(mclcb, pkt, &dhdr_infos)
	    == MCL_ERROR) {
		PRINT_ERR((mcl_stderr,
		"mcl_tx_ctrl::process_nack_pkt: ERROR, bad NORM header\n"))
		goto bad_hdr;
	}
	/*
	 * process each norm_nack_content_hdr_t block...
	 */
	while ((nc = mclcb->norm_pkt_mgmt.get_next_nack_content_block(mclcb, pkt)) != NULL) {
		/* from now on, the various nc fields are in host format and
		 * can be used directly... */
		switch (nc->form) {
		case NORM_NACK_ERASURES:
			break;
		default:
			PRINT_ERR((mcl_stderr,
			"mcl_tx_ctrl::process_nack_pkt: ERROR, unknown nack_content_hdr form %d\n", nc->form))
#ifdef DEBUG
			mcl_exit(-1);
#else
			goto bad_hdr;
#endif
		}

		switch (nc->flags) {
		case NORM_NACK_BLOCK:
			break;
		default:
			PRINT_ERR((mcl_stderr,
			"mcl_tx_ctrl::process_nack_pkt: ERROR, unknown nack_content_hdr flags %d\n", nc->flags))
#ifdef DEBUG
			mcl_exit(-1);
#else
			goto bad_hdr;
#endif
		}
		/* find the adu first */
		if (!(adu = mclcb->tx.find_adu(mclcb, nc->obj_transp_id))) {
			/*
			 * for some reason this ADU is not in tx window,
			 * probably because it has already been free'ed...
			 */
			TRACELVL(5, (mcl_stdout,
				"<- mcl_tx_ctrl::process_nack_pkt: WARNING, adu %d not found in tx window\n",
				nc->obj_transp_id))
			PRINT_ERR((mcl_stderr,
				"mcl_tx_ctrl::process_nack_pkt: ERROR, TODO\n"))
			goto bad_hdr;
		}
		/* then find the block */
		if (!(blk = adu->find_block(mclcb, nc->fec_block_nb))) {
			/*
			 * for some reason this block is not in tx window,
			 * probably because it has already been free'ed, or
			 * because of an error at the receiver.
			 */
			TRACELVL(5, (mcl_stdout,
				"<- mcl_tx_ctrl::process_nack_pkt: WARNING, block %d not found in tx window\n",
				nc->fec_block_nb))
			PRINT_ERR((mcl_stderr,
				"mcl_tx_ctrl::process_nack_pkt: ERROR, TODO\n"))
			goto bad_hdr;
		}
		/*
		 * create a new tevent
		 */
		te = new mcl_tx_ctrl_RETX_DONE_tevent(adu->get_seq(), blk->seq);
		/*
		 * avoid considering a NACK for a block which is currently
		 * being repaired, i.e. for which already exists a RETX_DONE
		 * tevent.
		 */	
		if (te->find_in_list(mclcb, &(this->RETX_DONE_tevent_head))) {
			/* NACK already processed, ignore */
			TRACELVL(5, (mcl_stdout,
			"   mcl_tx_ctrl::process_nack_pkt: NACK ignored (under repair)\n"))
			delete te;
			return MCL_OK;
		}
		/*
		 * now schedule enough additional FEC packets.
		 * needs to take into account the receiver needs, the
		 * available fec that has not yet been sent, and the
		 * remaining fec packets that can still be created...
		 */
		fec_required = nc->fec_symbol_id_or_erasure_count + 2;
			// two extra packets for security, in case of losses
		fec_avail = mclcb->fec.get_nb_fresh_fec_pkts(mclcb, blk);
		add_possible_fec = mclcb->fec.get_rem_nb_fec_pkts_to_create(mclcb, blk);
		if (fec_required <= fec_avail) {
			/*
			 * enough available FEC, no need to create more.
			 * schedule transmission of available fec packets
			 */
			PRINT_ERR((mcl_stderr,
				"mcl_tx_ctrl::process_nack_pkt: ERROR, TODO, sched avail fec pkts\n"))
			// TODO
			mcl_exit(-1);
		} else if (fec_required <= fec_avail + add_possible_fec) {
			/* we need to create additional fec and it is possible*/
			/*
			 * create additional fec pkts first.
			 * these FEC packets will automatically be scheduled
			 * once created
			 */
			fec_nb = mclcb->fec.encode(mclcb, blk,
						   fec_required - fec_avail);
			if (fec_nb < 0) {
				PRINT_ERR((mcl_stderr,
				"mcl_tx_ctrl::process_nack_pkt: ERROR, FEC encoding failed\n"))
				goto encoding_error;
			}
#ifdef DEBUG
			if (fec_nb < fec_required - fec_avail) {
				/*
				 * this is not serious! It can happen because
				 * of pending FEC creation requests.
				 */
				TRACELVL(5, (mcl_stdout,
				"<- mcl_tx_ctrl::process_nack_pkt: WARNING, %d fec produced instead of %d\n",
					fec_nb, fec_required - fec_avail))
			}
#endif /* DEBUG */
			/* schedule transmission of available fec packets */
			if (fec_avail > 0) {
				PRINT_ERR((mcl_stderr,
				"mcl_tx_ctrl::process_nack_pkt: ERROR, TODO, sched avail fec pkts 2\n"))
				// TODO
				mcl_exit(-1);
			}
		} else {
			/*
			 * Retransmit everything, data plus available fec.
			 *
			 * The idea is to try to retransmit all FEC DUs
			 * available, even if already tx before. If this
			 * is possible, no data pkts will be retransmitted.
			 * If not sufficient, then the idea is to transmit
			 * all data DUs, without FEC DUs.
			 */
			INT32	du_to_tx;	// nb of data DUs to tx
			INT32	fec_du_to_tx;	// nb of FEC DUs to tx

			if (blk->get_fec_du_nb_in_list() >= fec_required) {
				/* tx only FEC */
				du_to_tx = 0;
				fec_du_to_tx = fec_required;
			} else {
				du_to_tx = blk->du_nb;
				fec_du_to_tx = 0;
			}
			if (mclcb->tx_window.reschedule_tx_for_block(mclcb, blk,
					du_to_tx, fec_du_to_tx) == MCL_ERROR) {
				PRINT_ERR((mcl_stderr, "mcl_tx_ctrl::process_nack_pkt: ERROR, reschedule_tx_for_block() failed\n"))
				goto encoding_error;
			}
		}
		/*
		 * insert it in the tevent list and set a timer for
		 * a delayed FLUSH transmission...
		 */
		te->insert_in_list(mclcb, &(this->RETX_DONE_tevent_head));
		mclcb->timer.set_timer(RETX_DONE_TIMEOUT,
				(mcl_timer_handler *)this,
				(INT32)mclcb, (INT32)te);
		/*
		 * remove the corresponding FLUSH_DONE tevent for the
		 * same block which triggered this NACK (there should be one!)
		 */
		tmp_flush_te = new mcl_tx_ctrl_FLUSH_DONE_tevent(
					blk->adu->get_seq(), blk->seq);
		prev_flush_te = tmp_flush_te->find_in_list(mclcb,
					&(this->FLUSH_DONE_tevent_head));
		if (prev_flush_te) {
			/* remove the FLUSH_DONE from list */
			if (prev_flush_te->remove_from_list(mclcb,
					&(this->FLUSH_DONE_tevent_head)) == MCL_ERROR) {
				PRINT_ERR((mcl_stderr,
				"mcl_tx_ctrl::process_nack_pkt: remove_from_list() failed\n"))
				mcl_exit(-1);
			}
			/* and cancel the timer */
			if (mclcb->timer.cancel_timer((mcl_timer_handler *)this,
			    (INT32)mclcb, (INT32)prev_flush_te) == MCL_ERROR) {
				PRINT_ERR((mcl_stderr,
				"mcl_tx_ctrl::process_nack_pkt: cancel_timer() failed for FLUSH_DONE event, blk %d/%d\n",
				adu->get_seq(), blk->seq))
				// do not exit but do not delete the tevent!
				//mcl_exit(-1);
			} else {
				/* now we can delete the tevent */
				delete prev_flush_te;
			}
		}
		delete tmp_flush_te;

		PRINT_LVL(1, (mcl_stdout,
			"\tNACK recvd for %d/%d, missing %d\n",
			adu->get_seq(), blk->seq, fec_required))
	}
	delete pkt;             // free the now useless packet buffer
	TRACELVL(5, (mcl_stdout, "<- mcl_tx_ctrl::process_nack_pkt:\n"))
	return MCL_OK;


bad_hdr:
	mclcb->stats.bad_hdr++;
	mclcb->stats.errors++;
	delete pkt;             // free the now useless packet buffer
	TRACELVL(5, (mcl_stdout,
		"<- mcl_tx_ctrl::process_nack_pkt: ERROR, bad hdr\n"))
	return MCL_ERROR;

encoding_error:
	mclcb->stats.errors++;
	delete pkt;             // free the now useless packet buffer
	TRACELVL(5, (mcl_stdout,
		"<- mcl_tx_ctrl::process_nack_pkt: ERROR\n"))
	return MCL_ERROR;
}


/**
 * timer callback method.
 * @param arg1		mclcb
 * @param arg2		pointer to an tx_ctrl_timer_event_t struct
 */
void
mcl_tx_ctrl::timer_callback (INT32	arg1,
			     INT32	arg2)
{
	mcl_cb			*mclcb;	// session of this event
	mcl_tx_ctrl_tevent	*tevent;// timer event

	/*
	 * WARNING, there is a risk that the mclcb context has disappeared
	 * when this function is called. Should check the validity of the
	 * mclcb arg quickly...
	 */
	mclcb = (mcl_cb*)arg1;
	ASSERT(mclcb);
	mclcb->lock();
	tevent = (mcl_tx_ctrl_tevent *)arg2;
	ASSERT(tevent);
	TRACELVL(5, (mcl_stdout, "-> mcl_tx_ctrl::timer_callback: %s\n",
		tevent->get_type_string()))
	/*
	 * Timer processing...
	 */
	switch (tevent->type) {
	case TX_CTRL_TEVENT_FLUSH_DONE: {
		mcl_tx_ctrl_FLUSH_DONE_tevent	*te = (mcl_tx_ctrl_FLUSH_DONE_tevent*)tevent;
		/*
		 * check if the tevent is still in list. If not it means
		 * that it has been removed after a NACK reception
		 */
		if ((te->find_in_list(mclcb,
				      &(mclcb->tx_ctrl.FLUSH_DONE_tevent_head)))
		     == NULL) {
			/* so ignore... */
			TRACELVL(5, (mcl_stdout,
			"   mcl_tx_ctrl::timer_callback: FLUSH_DONE already removed\n"))
			delete te;
			break;
		}
		ASSERT(te == te->find_in_list(mclcb, &(mclcb->tx_ctrl.FLUSH_DONE_tevent_head)));
		/*
		 * and then perform tevent processing
		 */
		if (te->process(mclcb) == true) {
			/*
			 * processing of this FLUSH_DONE event is over...
			 * free and remove from list.
			 */
			te->remove_from_list(mclcb, &(mclcb->tx_ctrl.FLUSH_DONE_tevent_head));
			delete te;
		} /* else do nothing */
		break;
		}

	case TX_CTRL_TEVENT_RETX_DONE: {
		mcl_tx_ctrl_RETX_DONE_tevent	*te = (mcl_tx_ctrl_RETX_DONE_tevent*)tevent;
		/* remove the tevent from the list first */
		if ((te->find_in_list(mclcb,
				      &(mclcb->tx_ctrl.RETX_DONE_tevent_head)))
		    != NULL) {
			ASSERT(te == te->find_in_list(mclcb,
				&(mclcb->tx_ctrl.RETX_DONE_tevent_head)));
			te->remove_from_list(mclcb,
				&(mclcb->tx_ctrl.RETX_DONE_tevent_head));
		}
		/* and then perform tevent processing */
		te->process(mclcb);
		delete te;
		break;
		}

	default:
		ASSERT(0);
	}
	TRACELVL(5, (mcl_stdout, "<- mcl_tx_ctrl::timer_callback:\n"))
	mclcb->unlock();
	return;

#if 0
bad:
	TRACELVL(5, (mcl_stdout, "<- mcl_rx_ctrl::timer_callback: ERROR\n"))
	mclcb->unlock();
	return;
#endif
}


/**
 * See if there is one or more registered pending control
 * operations (i.e. tevent).
 * @param mclcb
 * @return	true if there is one or more pending ctrl operation
 */
bool
mcl_tx_ctrl::is_there_pending_ctrl (mcl_cb	*const mclcb)
{
	if (this->RETX_DONE_tevent_head != NULL ||
	    this->FLUSH_DONE_tevent_head != NULL)
		return true;
	else
		return false;
}


/****** mcl_tx_ctrl_tevent base class *****************************************/

#if 0
/**
 * Default constructor
 */
mcl_tx_ctrl_tevent::mcl_tx_ctrl_tevent (mcl_tx_ctrl_tevent_types	t)
{
	this->type = t;
}
#endif


/**
 * Default destructor
 */
mcl_tx_ctrl_tevent::~mcl_tx_ctrl_tevent ()
{
}


#ifdef DEBUG
/**
 * Return the tevent type as a static string.
 */ 
char *
mcl_tx_ctrl_tevent::get_type_string () const
{
	// WARNING: must be compliant with mcl_tx_ctrl_tevent_types definition!
	static char	*type_strings[] = {
		"TX_CTRL_TEVENT_INVALID",
		"TX_CTRL_TEVENT_FLUSH_DONE",
		"TX_CTRL_TEVENT_RETX_DONE"
	};

	return type_strings[this->type];
}
#endif // DEBUG


/**
 * Insert a tevent in an unordered list.
 * Called with the appropriate list head parameter.
 * @param mclcb
 * @param head  pointer to the list head pointer
 */
mcl_error_status
mcl_tx_ctrl_tevent::insert_in_list (mcl_cb		*const mclcb,
				    mcl_tx_ctrl_tevent	**head)
{
	ASSERT(head);
	TRACELVL(5, (mcl_stdout, "-> mcl_tx_ctrl_tevent::insert_in_list:\n"))
	if (!(*head)) {
		/*
		 * first tevent in list
		 */
		*head = this;
		this->next = this->prev = this;
		TRACELVL(5, (mcl_stdout, "<- mcl_tx_ctrl_tevent::insert_in_list: ok, inserted in empty list\n"))
	} else {
		/*
		 * insert in list tail.
		 */
		this->next = *head;
		this->prev = (*head)->prev;
		(*head)->prev->next = this;
		(*head)->prev = this;
		TRACELVL(5, (mcl_stdout, "<- mcl_tx_ctrl_tevent::insert_in_list:\n"))
	}
	return MCL_OK;
}


/**
 * Remove a tevent from an unordered list.
 * Called with the appropriate list head parameter.
 * @param mclcb
 * @param head  pointer to the list head pointer
 */
mcl_error_status 
mcl_tx_ctrl_tevent::remove_from_list (mcl_cb			*const mclcb,
				      mcl_tx_ctrl_tevent	**head)
{
	mcl_tx_ctrl_tevent	*p, *n;		// temp prev and next pointers

	ASSERT(head);
	TRACELVL(5, (mcl_stdout, "-> mcl_tx_ctrl_tevent::remove_from_list:\n"))
	ASSERT(*head);
	p = this->prev;
	n = this->next;
	if (p == this) {	/* only one tevent in list */
		ASSERT(n == this);
		*head = NULL;	/* list is now empty */
	} else {
		p->next = n;
		n->prev = p;
		if (*head == this) {
			*head = n;	/* adu was the first in list */
		}
	}
	TRACELVL(5, (mcl_stdout, "<- mcl_tx_ctrl_tevent::remove_from_list:\n"))
	return MCL_OK;
}


/****** FLUSH_DONE class ******************************************************/

/**
 * Default constructor.
 */
mcl_tx_ctrl_FLUSH_DONE_tevent::mcl_tx_ctrl_FLUSH_DONE_tevent
						(UINT32		adu_idf,
						 UINT32		block_idf)
{
	this->type = TX_CTRL_TEVENT_FLUSH_DONE;
	this->adu_id = adu_idf;
	this->block_id = block_idf;
	this->repeat_count = UNANSWERED_FLUSH_REPEAT_COUNT;
}


/**
 * Default destructor.
 */
mcl_tx_ctrl_FLUSH_DONE_tevent::~mcl_tx_ctrl_FLUSH_DONE_tevent ()
{
}


/**
 * Find a FLUSH_DONE_tevent in an unordered list with a type-dependant criteria.
 * @param mclcb
 * @param head  pointer to the list head pointer
 * @return      returns a pointer to the tevent if found, NULL otherwise
 */
mcl_tx_ctrl_FLUSH_DONE_tevent *
mcl_tx_ctrl_FLUSH_DONE_tevent::find_in_list (mcl_cb		*const mclcb,
					    mcl_tx_ctrl_tevent	**head)
{
	mcl_tx_ctrl_FLUSH_DONE_tevent		*tevent;
	mcl_tx_ctrl_FLUSH_DONE_tevent		*nhead;

	TRACELVL(5, (mcl_stdout, "-> mcl_tx_ctrl_tevent::find_in_list:\n"))
	ASSERT(head);
	if (!(nhead = (mcl_tx_ctrl_FLUSH_DONE_tevent*)(*head)))  {
		TRACELVL(5, (mcl_stdout,
			"<- mcl_tx_ctrl_tevent::find_in_list: empty list\n"))
		return NULL;
	}
	/*
	 * start from the list tail
	 */
	tevent = (mcl_tx_ctrl_FLUSH_DONE_tevent*)(nhead->prev);
	while (1) {
		ASSERT(tevent);
		if ((tevent->adu_id == this->adu_id) &&
		    (tevent->block_id == this->block_id)) {
			/* found */
			TRACELVL(5, (mcl_stdout,
				"<- mcl_tx_ctrl_tevent::find_in_list: found\n"))
			return tevent;
		} 
		if (tevent == nhead) {
			/* we have cycled and tevent is not in list */
			TRACELVL(5, (mcl_stdout,
				"<- mcl_tx_ctrl_tevent::find_in_list: not found\n"))
			return NULL;
		}
		tevent = (mcl_tx_ctrl_FLUSH_DONE_tevent*)tevent->prev;
	}
}


/**
 * Perform FLUSH_DONE_tevent specific processing.
 * @return	true if tevent is completed and should be free'ed and
 * 		removed from list, false if it has been re-scheduled.
 */
bool
mcl_tx_ctrl_FLUSH_DONE_tevent::process (mcl_cb	*const mclcb)
{
	mcl_adu			*adu;
	mcl_block		*blk;
	bool			free_adu;	// true if ADU can be free'ed
	mcl_block		*blk_tmp;
	UINT32			rem;

	TRACELVL(5, (mcl_stdout,
		"-> mcl_tx_ctrl_FLUSH_DONE_tevent::process:\n"))
	/* find the adu first */
	if (!(adu = mclcb->tx.find_adu(mclcb, this->adu_id))) {
		/*
		 * for some reason this ADU is not in tx window... give-up!
		 */
		TRACELVL(5, (mcl_stdout, "<- mcl_tx_ctrl_FLUSH_DONE_tevent::process: WARNING, adu not found in tx window\n"))
		goto error;
	}
	/* then find the block */
	if (!(blk = adu->find_block(mclcb, this->block_id))) {
		/*
		 * for some reason this block is not in tx window... give-up!
		 */
		TRACELVL(5, (mcl_stdout, "<- mcl_tx_ctrl_FLUSH_DONE_tevent::process: WARNING, block not found in tx window\n"))
		goto error;
	}
	/*
	 * should it be the last FLUSH?
	 */
	if (this->repeat_count > 0) {
		/*
		 * for improved robustness send the FLUSH once again,
		 * just in case the previous one(s) was lost, and
		 * schedule a new callback...
		 */
		this->repeat_count--;
		mclcb->tx_ctrl.send_flush_pkt(mclcb, blk);
		mclcb->timer.set_timer(FLUSH_DONE_TIMEOUT,
			(mcl_timer_handler *)&(mclcb->tx_ctrl),
			(INT32)mclcb, (INT32)this);
		/*
		 * done... do not delete the tevent object!
		 */
		TRACELVL(5, (mcl_stdout,
			"<- mcl_tx_ctrl_FLUSH_DONE_tevent::process: FLUSH for blk %d/%d rescheduled, repeat_count=%d\n",
			adu->get_seq(), blk->seq, this->repeat_count))
		return false;	/* tevent re-scheduled */
	}
	/*
	 * the FLUSH process is now over!
	 * no more FLUSH will be generated and the block is assumed to
	 * be reliably transmitted.
	 * do the cleanup work now...
	 * remove this block from tx_window and free memory.
	 */
	blk->set_tx_status(BLK_TSTATUS_DONE);
	mclcb->tx_window.remove_block(mclcb, blk, true);
	blk->remove_and_free_all_fec_dus(mclcb); // to do after remove_block
	/*
	 * if ADU is in status ADU_TSTATUS_FINISH_TX, then it's worth
	 * seeing if all blocks of this ADU have been reliably
	 * delivered or not. If yes, then remove this ADU altogether...
	 */
	free_adu = true;
	for (rem = adu->get_block_nb(), blk_tmp = adu->get_block_head();
	     rem > 0; rem--, blk_tmp++) {
		if (blk->get_tx_status() != BLK_TSTATUS_DONE) {
			free_adu = false;
			break;
		}
	}
	if (free_adu) {
		/* this function frees buffers but not the ADU object itself */
		adu->set_tx_status(ADU_TSTATUS_DONE);
		adu->remove_and_free_all_buffers(mclcb);
	}

	TRACELVL(5, (mcl_stdout,
		"<- mcl_tx_ctrl_FLUSH_DONE_tevent::process: blk %d/%d over\n",
		adu->get_seq(), blk->seq))
	return true;

error:
	TRACELVL(5, (mcl_stdout,
		"<- mcl_tx_ctrl_FLUSH_DONE_tevent::process: ERROR\n"))
	return true;
}


/****** RETX_DONE class *******************************************************/

/**
 * Default constructor.
 */
mcl_tx_ctrl_RETX_DONE_tevent::mcl_tx_ctrl_RETX_DONE_tevent
						(UINT32		adu_idf,
						 UINT32		block_idf)
{
	this->type = TX_CTRL_TEVENT_RETX_DONE;
	this->adu_id = adu_idf;
	this->block_id = block_idf;
}


/**
 * Default destructor.
 */
mcl_tx_ctrl_RETX_DONE_tevent::~mcl_tx_ctrl_RETX_DONE_tevent ()
{
}


/**
 * Find a RETX_DONE_tevent in an unordered list with a type-dependant criteria.
 * @param mclcb
 * @param head  pointer to the list head pointer
 * @return      returns a pointer to the tevent if found, NULL otherwise
 */
mcl_tx_ctrl_RETX_DONE_tevent *
mcl_tx_ctrl_RETX_DONE_tevent::find_in_list (mcl_cb		*const mclcb,
					    mcl_tx_ctrl_tevent	**head)
{
	mcl_tx_ctrl_RETX_DONE_tevent		*tevent;
	mcl_tx_ctrl_RETX_DONE_tevent		*nhead;

	TRACELVL(5, (mcl_stdout, "-> mcl_tx_ctrl_tevent::find_in_list:\n"))
	ASSERT(head);
	if (!(nhead = (mcl_tx_ctrl_RETX_DONE_tevent*)(*head)))  {
		TRACELVL(5, (mcl_stdout,
			"<- mcl_tx_ctrl_tevent::find_in_list: empty list\n"))
		return NULL;
	}
	/*
	 * start from the list tail
	 */
	tevent = (mcl_tx_ctrl_RETX_DONE_tevent*)(nhead->prev);
	while (1) {
		ASSERT(tevent);
		if ((tevent->adu_id == this->adu_id) &&
		    (tevent->block_id == this->block_id)) {
			/* found */
			TRACELVL(5, (mcl_stdout,
				"<- mcl_tx_ctrl_tevent::find_in_list: found\n"))
			return tevent;
		} 
		if (tevent == nhead) {
			/* we have cycled and tevent is not in list */
			TRACELVL(5, (mcl_stdout,
				"<- mcl_tx_ctrl_tevent::find_in_list: not found\n"))
			return NULL;
		}
		tevent = (mcl_tx_ctrl_RETX_DONE_tevent*)tevent->prev;
	}
}


/**
 * Perform RETX_DONE_tevent specific processing.
 */
void
mcl_tx_ctrl_RETX_DONE_tevent::process (mcl_cb	*const mclcb)
{
	mcl_adu			*adu;
	mcl_block		*blk;

	TRACELVL(5, (mcl_stdout, "-> mcl_tx_ctrl_RETX_DONE_tevent::process:\n"))
	/* find the adu first */
	if (!(adu = mclcb->tx.find_adu(mclcb, this->adu_id))) {
		/*
		 * for some reason this ADU is not in tx window... give-up!
		 */
		TRACELVL(5, (mcl_stdout, "<- mcl_tx_ctrl_RETX_DONE_tevent::process: WARNING, adu not found in tx window\n"))
		goto error;
	}
	/* then find the block */
	if (!(blk = adu->find_block(mclcb, this->block_id))) {
		/*
		 * for some reason this block is not in tx window... give-up!
		 */
		TRACELVL(5, (mcl_stdout, "<- mcl_tx_ctrl_RETX_DONE_tevent::process: WARNING, block not found in tx window\n"))
		goto error;
	}
	/*
	 * and now send a FLUSH packet for this block and create a new
	 * FLUSH_DONE tevent
	 */
	mclcb->tx_ctrl.send_flush_and_register_tevent(mclcb, blk);
	TRACELVL(5, (mcl_stdout,
		"<- mcl_tx_ctrl_RETX_DONE_tevent::process:\n"))
	return;

error:
	TRACELVL(5, (mcl_stdout,
		"<- mcl_tx_ctrl_RETX_DONE_tevent::process: ERROR\n"))
	return;
}

