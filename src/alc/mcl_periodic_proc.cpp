/* $Id: mcl_periodic_proc.cpp,v 1.5 2005/05/18 14:37:56 roca Exp $ */
/*
 *  Copyright (c) 2003 INRIA - All rights reserved
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


mcl_periodic_proc::mcl_periodic_proc ()
{
	/** Time_count of last do_periodic_proc call. */
	this->last_periodic_proc_tc = 0;
	/** itimer of last do_periodic_proc call. */
	this->last_periodic_proc_it = 0;
	/** fractional tick_nb in do_periodic_proc. */
	this->remaining_tx_tick_nb = 0.0;
}


mcl_periodic_proc::~mcl_periodic_proc ()
{
}


void
mcl_periodic_proc::scan_all_sessions (void)
{
	INT32		ses;		// session id
	mcl_cb		*mclcb;		// session control block
#ifdef GET_SYSINFO
	mcl_cb		*mclcb_tmp = NULL;/* remember a valid mclcb */
	INT32		mcl_sysinfo_count = MCL_SYSINFO_PERIOD / MCL_PERIODIC_TIMER_PERIOD;
#endif

	/*
	 * scan all the sessions
	 */
	for (ses = 0;  ses < MAX_NB_MCLCB; ses++) {
		if ((mclcb = mclcb_tab[ses]) == NULL)
			continue;	/* unused entry! */
		mclcb->lock();
		if (!mclcb->is_fully_initialized()) {
			mclcb->unlock();
			continue;
		}
		TRACELVL(6, (mcl_stdout, "mcl_periodic_proc::scan_all_sessions: time_count=%d, ses=%d, states=%s/%s\n",
			mcl_time_count, ses,
			mclcb->fsm.print_tx_state(mclcb),
			mclcb->fsm.print_rx_state(mclcb)))
		/* everything is done here... */
		mclcb->periodic_proc.do_periodic_proc(mclcb);
#ifdef GET_SYSINFO
		mclcb_tmp = mclcb; /* remember for GET_SYSINFO needs */
#endif
		mclcb->unlock();
	}
#ifdef GET_SYSINFO
	/*
	 * print system information if required
	 * precise timing is not required here...
	 */
	if (mclcb_tmp && mclcb_tmp->verbose == 2 && --mcl_sysinfo_count == 0) {
		/* one trace every approx. MCL_SYSINFO_PERIOD usec */
		mcl_print_sysinfo(mclcb_tmp);
		mcl_sysinfo_count = MCL_SYSINFO_PERIOD / MCL_PERIODIC_TIMER_PERIOD;
	}
#endif /* GET_SYSINFO */
}


void
mcl_periodic_proc::do_periodic_proc (mcl_cb	*mclcb)
{
	//mcl_periodic_timer_var *timer_var; // class containing per session var
	mcl_itime_t	now;		/* current itime */
	mcl_itime_t	elit;		/* elasped itime since last call */
	INT32		eltc;		/* elasped time_count since last call */
	double		true_tx_tick_nb;/* tx tick elapsed as a float */
	INT32		tx_tick_nb;	/* integral part of true_tx_tick_nb */

	ASSERT(mclcb->is_fully_initialized());
	TRACELVL(5, (mcl_stdout, "-> mcl_periodic_proc::mcl_do_periodic_proc: mclcb=x%x\n", (int)mclcb))

	//timer_var = &(mclcb->periodic_timer_var);
	now = mcl_get_itime();
	//if (timer_var->last_periodic_proc_tc == 0)
	if (this->last_periodic_proc_tc == 0) {
		/* first call for this mclcb => timing initialization */
		this->last_periodic_proc_it = now;
		this->last_periodic_proc_tc = mcl_time_count;
		this->remaining_tx_tick_nb = 0.0;
	}
	/*
	 * elapsed time
	 */
	elit = mcl_itime_sub(now, this->last_periodic_proc_it);
	this->last_periodic_proc_it = now;
	/*
	 * elapsed time_count since last call
	 * NB: no fractional part here, done in mcl_timer_thread
	 */
	eltc = mcl_time_count - this->last_periodic_proc_tc;
	this->last_periodic_proc_tc = mcl_time_count;

	/*
	 * RLC periodic processing
	 */
#ifdef RLC
	if (mclcb->is_a_receiver() && mclcb->congestion_control == RLC_CC &&
	    eltc > 0) {
		/* update counter */
		mclcb->rlccb.rlc_rx_timer_count -= eltc;
		/* call rlc_rx_timer as many times as required if eltc large */
		while (mclcb->rlccb.rlc_rx_timer_count < 0) {
			rlc_rx_timer(mclcb);
			mclcb->rlccb.rlc_rx_timer_count +=
				mcl_rlc_rx_period / MCL_PERIODIC_TIMER_PERIOD;
		}
	}
#endif
	/*
	 * FLID-SL periodic processing
	 */
#ifdef FLIDS
	if (mclcb->is_a_sender() && mclcb->congestion_control == FLID_SL_CC &&
	    eltc > 0) {
		/* update counter */
		mclcb->flids_cb.flids_tx_timer_count -= eltc;
		/* call flids_tx_timer as many times as required if eltc large*/
		while (mclcb->flids_cb.flids_tx_timer_count < 0) {
			FLIDs_TxTimer(mclcb);
			mclcb->flids_cb.flids_tx_timer_count +=
					mclcb->flids_cb.tsd / MCL_PERIODIC_TIMER_PERIOD;
		}
	}
#endif
	/*
	 * periodic transmissions
	 */
	if (mclcb->is_a_sender()) {
		/*
		 * how many tx ticks since last call?
		 * first calculate the float tick nb, then its integral part,
		 * and remember the decimal part for next time
		 */
		true_tx_tick_nb = (double)
			(mcl_it2sec(elit) / mcl_periodic_timer::get_period()
			 + this->remaining_tx_tick_nb);
		tx_tick_nb = (INT32)(true_tx_tick_nb);
		this->remaining_tx_tick_nb = true_tx_tick_nb - (double)tx_tick_nb;
		if (mcl_periodic_timer::must_reduce_activity()) {
			/*
			 * when in "no_time_to_sleep" mode, limit the
			 * tx_tick_nb to try to solve the problem...
			 */
			tx_tick_nb = 1;
		} else {
			/* limit tx_tick_nb anyway to avoid large tx bursts 
			 * unless we are in singlelayer mode in which case
			 * the number of packets may indeed be quite large
			 * on the single transmission layer */
			if (!(mclcb->single_layer_mode)) {
				tx_tick_nb = min(tx_tick_nb, 5);
			}

		}
		/* call the tx function for each tx_tick */
		for (int i = tx_tick_nb; i > 0; i--) {
			mclcb->tx.try_to_send(mclcb);
		}
	}

#if 0 // not yet
	/*
	 * launch memory cleanup function periodically
	 */
	if (mclcb->is_a_sender() && mclcb->delivery_mode == DEL_MODE_STREAMING
#ifdef VIRTUAL_TX_MEM
	    && !(mclcb->vtm_used) 
#endif  /* VIRTUAL_TX_MEM */
	    && eltc > 0) {
		/* update counter */
		mclcb->tx_mem_cleanup_count -= eltc;
		if (mclcb->tx_mem_cleanup_count < 0) {
			mclcb->tx_mem_cleanup_count =
				TX_MEM_CLEANUP_PERIOD / MCL_PERIODIC_TIMER_PERIOD;
			mcl_tx_cleanup(mclcb);
		}
	}
#endif

#if 0
	/*
	 * print stats periodically
	 */
	if (mclcb->statistics == 2) {
		mclcb->stats_time_count -= eltc;
		if (mclcb->stats_time_count < 0) {
			mclcb->stats_time_count = STATS_PERIOD / MCL_PERIODIC_TIMER_PERIOD;
			if (mclcb->sender)
				mcl_print_tx_stats(mclcb);
			if (mclcb->receiver)
				mcl_print_rx_stats(mclcb);
			/* nb: stats_period is init'ed in func*/
		}
		//mclcb->stat_rem_it = mcl_itime_sub(mclcb->stat_rem_it, elit);
		//if (mcl_itime_leq_0(mclcb->stat_rem_it)) {
		//	mclcb->stat_rem_it = mcl_user2it();
		//	if (mclcb->sender)
		//		mcl_print_tx_stats(mclcb);
		//	if (mclcb->receiver)
		//		mcl_print_rx_stats(mclcb);
		//	/* nb: stats_period is init'ed in func*/
		//}
	}
#endif // 0

	TRACELVL(5, (mcl_stdout, "<- mcl_periodic_proc::mcl_do_periodic_proc:\n"))
}

