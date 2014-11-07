/* $Id: mcl_tx.cpp,v 1.4 2004/01/30 16:27:43 roca Exp $ */
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


mcl_tx::mcl_tx ()
{
	/* so that ADU seq effectively starts at MCL_ISS... */
	this->highest_adu_seq = (UINT32)(MCL_ISS - 1);
	this->reuse_appli_tx_buffer = false;
	this->adu_head = NULL;
	this->next_norm_pkt_seq = 0;
}


mcl_tx::~mcl_tx ()
{
	ASSERT(this->adu_head == NULL);
}


void
mcl_tx::free_all_adu (mcl_cb	*const mclcb)
{
	mcl_adu		*adu;

	TRACELVL(5, (mcl_stdout, "-> mcl_tx::free_all_adu:\n"))
	while ((adu = adu_head) != NULL) {
		this->remove_adu(mclcb, adu);
		delete adu;      /* free the adu and all of its blocks/dus */
	}
	TRACELVL(5, (mcl_stdout, "<- mcl_tx::free_all_adu:\n"))
}


/**
 * Set the transmission rate in bits per second (bps).
 * The desired rate can be adjusted so that the is an integral
 * number of packets per tick.
 * @param desired_rate
 * @return	rate actually set
 */
float
mcl_tx::set_bps_tx_rate (mcl_cb		*const mclcb,
			 const float	desired_rate)
{
	// first of all, calculate the rounded packet per tick rate, but
	// no less than 1 packet per tick
#ifdef SOLARIS
	this->ppt_tx_rate = max(1, floor((double)desired_rate / (double)(mclcb->get_payload_size() * 8 * mcl_periodic_timer::get_frequency())));
#else
	this->ppt_tx_rate = max(1, floorf(desired_rate / (mclcb->get_payload_size() * 8 * mcl_periodic_timer::get_frequency())));
#endif // OS
	
	// the other two rates derive from this ppt rate!
	this->bps_tx_rate = this->ppt_tx_rate * mclcb->get_payload_size() * 8 *
				mcl_periodic_timer::get_frequency();
	this->pps_tx_rate = this->ppt_tx_rate *
				mcl_periodic_timer::get_frequency();
	TRACELVL(5, (mcl_stdout,
		"mcl_tx::set_bps_tx_rate: desired=%.3f, ppt=%.3f, pps=%.3f, bps=%.3f, payload_sz=%d, freq=%.3f\n",
		desired_rate, this->ppt_tx_rate, this->pps_tx_rate,
		this->bps_tx_rate, mclcb->get_payload_size(),
		mcl_periodic_timer::get_frequency()))
	return (this->bps_tx_rate);
}


/**
 * Set a pre-defined transmission profile.
 * @return	completion status (MCL_OK or MCL_ERROR).
 */
mcl_error_status 
mcl_tx::set_tx_profile (mcl_cb		*const mclcb,
			mcl_tx_profile	prof)
{
	TRACELVL(5, (mcl_stdout, "-> mcl_tx::set_tx_profile: %d\n", prof))
	switch (prof) {
	case MCL_TX_PROFILE_LOW_RATE_INTERNET:
		this->set_bps_tx_rate(mclcb, 32000);	// 32 kbps
		break;
	case MCL_TX_PROFILE_MID_RATE_INTERNET:
		this->set_bps_tx_rate(mclcb, 512000);	// 512 kbps
		break;
	case MCL_TX_PROFILE_HIGH_SPEED_INTERNET:
		this->set_bps_tx_rate(mclcb, 2000000);	// 2 Mbps
		break;
	case MCL_TX_PROFILE_HIGH_SPEED_LAN:
		this->set_bps_tx_rate(mclcb, 10000000);	// 10 Mbps
		break;
	default:
		PRINT_ERR((mcl_stderr, "<- mcl_tx::set_tx_profile: ERROR, invalid profil %d\n", prof))
		return MCL_ERROR;
	}
	TRACELVL(5, (mcl_stdout, "<- mcl_tx::set_tx_profile: ok\n"))
	return MCL_OK;
}


/**
 * Try to send as much data as possible in a tx tick for this session.
 */
void
mcl_tx::try_to_send (mcl_cb	*const mclcb)
{
	int		du_nb;		/* number of DUs to send */
	bool		something_sent;	/* flag: is there any DU to tx? */
	static bool	no_new_obj_already_sent = false; /* send only once */

	TRACELVL(5, (mcl_stdout, "-> mcl_tx::try_to_send:\n"))
	if (!mclcb->is_a_sender() || mclcb->fsm.is_closed(mclcb)) {
		TRACELVL(5, (mcl_stdout, "<- mcl_tx::try_to_send: ERROR, this mclcb cannot tx\n"))
		return;
	}

	something_sent= false;		/* reset first */
	TRACELVL(5, (mcl_stdout,
		"   mcl_tx::try_to_send: ppt_rate=%.2f, %d DUs available\n", 
		this->get_pkt_per_tick_tx_rate(),
		mclcb->tx_window.get_nb_of_available_du_to_tx()))
	/*
	 * is there something to send?
	 */
	if ((du_nb = min((INT32)(this->get_pkt_per_tick_tx_rate()),
			 mclcb->tx_window.get_nb_of_available_du_to_tx())) > 0) {
		/* yes */
		this->send_pkt(mclcb, du_nb);
		something_sent = true;
	}
#if 0
	// if (tl->tot_rem == 0 &&
	//    mclcb->delivery_mode == DEL_MODE_ON_DEMAND)
	if (tl->tot_rem == 0) {
		/*
		 * get ready for a new tx cycle
		 * on that layer
		 */
		mcl_new_tx_cycle(mclcb, tl);
	}
#endif
	//if (something_sent == false && mclcb->fsm.no_new_adu(mclcb))
	if (something_sent == false && mclcb->fsm.finish_tx(mclcb)) {
		if (no_new_obj_already_sent == false) {
			/*
			 * first, send NO_NEW_OBJECT to inform receivers 
			 * no new objects will be sent.
			 * do it only once (but with MCL_TX_ROBUSTNESS_FACTOR).
			 *
			 * performed here rather than immediately when moving to
			 * FINISH_TX state to avoid the problem of implicitely
			 * announced ADUs that occur if NO_NEW_OBJECT is
			 * received before a NEW_OBJECT.
			 */
			mclcb->tx_ctrl.send_no_new_object_pkt(mclcb);
			/*
			 * then send a FLUSH for the last block
			 */
			mclcb->tx_ctrl.check_if_ctrl_pkt_must_be_sent(mclcb,
								NULL, true);
			no_new_obj_already_sent = true;
		}
	    	if (mclcb->tx_ctrl.is_there_pending_ctrl(mclcb) == false) {
			/*
			 * really finished, no control operation pending,
			 * so stop everything and send the CLOSE pkt
			 */
			mclcb->fsm.update_tx_state(mclcb, TEVENT_ALL_DU_SENT);
			mclcb->tx_ctrl.send_close_pkt(mclcb);
			mclcb->fsm.update_tx_state(mclcb, TEVENT_CLOSE_SENT);
		}
	}
	TRACELVL(5, (mcl_stdout, "<- mcl_tx::try_to_send:\n"))
}


/*
 * Send this number of DUs 
 */
void
mcl_tx::send_pkt (mcl_cb		*const mclcb,
		  INT32			du_nb)
{
	mcl_du			*du = NULL;
	norm_common_hdr_t	*hdr;	// norm header
	INT32			hlen;	// norm header length
	INT32			rem;	// remaining pkts waiting to be sent
	
	TRACELVL(5, (mcl_stdout, "-> mcl_tx::send_pkt: %d DUs\n", du_nb))
	ASSERT(du_nb > 0);
	ASSERT(mclcb->tx_window.get_nb_of_available_du_to_tx() >= du_nb);

	for (rem = du_nb; rem > 0; rem--) {
		/* retrieve the next du that should be sent */
		du = mclcb->tx_window.get_next_du_to_tx(mclcb);
		ASSERT(du);
		/* create the NORM packet */
		if ((hlen = mclcb->norm_pkt_mgmt.create_data_hdr(mclcb, du, &hdr)) <= 0) {
			PRINT_ERR((mcl_stderr, "mcl_tx::send_pkt: norm_pkt_mgmt.create_data_hdr() failed\n"))
			ASSERT(0);	// in debug mode, stop immediately...
			return;
		}
		/* then send the packet */
		mclcb->ses_channel.send_pkt(mclcb, hdr, hlen, du);
		PRINT_LVL(1, (mcl_stdout, "\t%s %d/%d/%d\n",
				(du->is_fec ? "fec" : "data"),
				du->block->adu->get_seq(), du->block->seq,
				du->seq))
#if 0
		{static UINT32	pp_adu = 66666666;
		 static UINT32	pp_blk = 66666666;
		 static UINT32	pp_du  = 66666666;

			if (du->block->adu->get_seq() == pp_adu &&
			    du->block->seq == pp_blk &&
			    du->seq == pp_du) {
				 printf("mcl_tx::send_pkt: ERROR, pkt sent just before!\n");
				 mcl_exit(-1);
			}
			pp_adu = du->block->adu->get_seq();
			pp_blk = du->block->seq;
			pp_du = du->seq;
		}
#endif
		/* see if any control packet must be issued or not */
		mclcb->tx_ctrl.check_if_ctrl_pkt_must_be_sent(mclcb, du, false);
	}
	TRACELVL(5, (mcl_stdout, "<- mcl_tx::send_pkt:\n"))
}


