/* $Id: mcl_stats.cpp,v 1.10 2005/01/11 13:12:34 roca Exp $ */
/*
 *  Copyright (c) 1999-2004 INRIA - All rights reserved
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


/****** Trace functions ******/

/*
 * Print a sent DU in an understandable way.
 */
void
mcl_print_sent_du (mcl_cb	*mclcb,
		   INT32	type,		/* 0 (data) or EH_SIG */
		   INT32	val,		/* sub-type or layer */
		   hdr_infos_t	*hdr_infos)
{
	if (type > 0) {
		return;
	}
	if (mclcb->get_verbosity() == 2) {
		/*
		 * produce traces for timely analysis
		 */
		struct timeval	time;
		time = mcl_get_tvtime();
		PRINT_OUT((mcl_stdout, "\n%ld.%06ld\tpkt_sent lay=%d aseq=%d bseq=%d dseq=%d %s\n",
			time.tv_sec, time.tv_usec, val,
			hdr_infos->idf_adu,
			hdr_infos->idf_block, hdr_infos->idf_du,
                       	(hdr_infos->is_fec ? "fec" : "data")))
	} else if (mclcb->get_verbosity() >= 3) {
		PRINT_OUT((mcl_stdout,
			"pkt on lay=%d time=%d aseq=%d/bseq=%d/dseq=%d/%s\n",
			val, mcl_time_count, hdr_infos->idf_adu,
			hdr_infos->idf_block, hdr_infos->idf_du,
			(hdr_infos->is_fec ? "fec" : "data")))
	}
}


/*
 * Print a received DU in an understandable way.
 */
void
mcl_print_recvd_du (mcl_cb	*mclcb,
		    INT32	type,		/* 0 (data) or EH_SIG */
		    INT32	val,		/* sub-type or layer */
		    hdr_infos_t	*hdr_infos)
{
	static int	prev_level=-1;		/* for verbose mode only */

	ASSERT(EH_SIG > 0);			/* to avoid confusion ! */
	/*
	if (type == EH_SIG && val == EXT_FTI) {
		PRINT_OUT((mcl_stdout, "New ADU (%d: %d bytes, %d blocks)\n",
			hdr_infos->idf_adu, hdr_infos->adu_len,
			(int)ceil((double)hdr_infos->adu_len /
			(double)hdr_infos->block_len)))
	} else if (type == EH_SIG && val == EXT_NONEWADU)
	*/
	if (type == EH_SIG && val == EXT_NONEWADU) {
		PRINT_OUT((mcl_stdout, "No New ADU (seq: %d to %d)\n",
			mcl_iss, hdr_infos->max_idf_adu))
	} else if (type == EH_SIG && val == SIG_CLOSE) {
		PRINT_OUT((mcl_stdout, "\nClose\n"))
	} else {
		ASSERT(type != EH_SIG);
		if (mclcb->get_verbosity() == 1) {
			/*
			 * produce traces for standard compact analysis
			 */
			if (val == 0) {
				PRINT_OUT((mcl_stdout, "\n\t"))
			}
			if (val != prev_level && val > 0) {
				PRINT_OUT((mcl_stdout, "\t\t"))
			}
			prev_level = val;
			if (hdr_infos->is_fec) {
				PRINT_OUT((mcl_stdout, "((%d/%d/%d)) ",
				hdr_infos->idf_adu,
				hdr_infos->idf_block, hdr_infos->idf_du))
			} else {
				PRINT_OUT((mcl_stdout, "[%d/%d/%d] ",
				hdr_infos->idf_adu,
				hdr_infos->idf_block, hdr_infos->idf_du))
			}
		} else if (mclcb->get_verbosity() == 2) {
			/*
			 * produce traces for timely analysis
			 */
			struct timeval	time;
			time = mcl_get_tvtime();
			PRINT_OUT((mcl_stdout, "\n%ld.%06ld\tpkt_rcvd lay=%d aseq=%d bseq=%d dseq=%d %s\n",
				time.tv_sec, time.tv_usec, val,
				hdr_infos->idf_adu,
				hdr_infos->idf_block, hdr_infos->idf_du,
                        	(hdr_infos->is_fec ? "fec" : "data")))
		} else {
			/*
			 * produce traces for detailed analysis
			 */
			PRINT_OUT((mcl_stdout,
			"pkt on lay=%d time=%d aseq=%d/bseq=%d/dseq=%d/%s\n",
				val, mcl_time_count, hdr_infos->idf_adu,
				hdr_infos->idf_block, hdr_infos->idf_du,
                        	(hdr_infos->is_fec ? "fec" : "data")))
		}
	}
}


/****** Statistical functions ******/


/*
 * Print sending side statistics
 */
void
mcl_print_tx_stats (mcl_cb	*mclcb)
{
	int		i;
	int		max_i = MAX_NB_TX_LAYERS;
	stats_t		*sp;
	struct timeval	tv;

	ASSERT(mclcb->get_stats_level());
	sp = &(mclcb->stats);
	tv = mcl_get_tvtime();
	PRINT_OUT((stdout, "\n--- TX STATISTICS at t=%ld.%06ld ---\n",
		   tv.tv_sec, tv.tv_usec))
#ifdef NEVERDEF
	PRINT_OUT((mcl_stdout, "\n--- TX STATISTICS at t=%d ---\n",
		   mcl_time_count))
#endif
	PRINT_OUT((mcl_stdout, "\tPayload: tx_src_bytes=%ld  tx_fec_bytes=%ld\n\ttx bytes per layer:\t",
		sp->tx_bytes, sp->tx_fec_bytes))
	for (i=0; i < max_i; i++) {
		if (sp->tx_bytes_per_lvl[i] == 0) {
			/* no need to consider layers above and including
			 * this one, they are not used */
			max_i = i;
		} else {
			PRINT_OUT((mcl_stdout, "%ld\t",
				sp->tx_bytes_per_lvl[i]))
		}
	}
	PRINT_OUT((mcl_stdout, "\n\ttx_src_pkts=%ld  tx_fec_pkts=%ld\n\ttx pkts per layer:\t",
		sp->tx_pkts, sp->tx_fec_pkts))
	for (i=0; i < max_i; i++) {
		PRINT_OUT((mcl_stdout, "%ld\t", sp->tx_pkts_per_lvl[i]))
	}
	PRINT_OUT((mcl_stdout, "\n\tPayload+SIG: tot_snt=%ld  tot_rx=%ld bytes\n\tbuf_space=%ld  max_buf_space=%ld  adu_announced=%ld\n",
		sp->tx_totbytes, sp->rx_totbytes,
		sp->buf_space, sp->max_buf_space,
		sp->adus_announced))
#ifdef SIMUL_LOSSES
	if (sp->tx_simul_loss_sent > 0) { 
		PRINT_OUT((mcl_stdout, "\tsimul_losses: lost=%ld  loss_ratio=%.3f\n",
		sp->tx_simul_loss_lost,
		(float)sp->tx_simul_loss_lost/(float)sp->tx_simul_loss_sent))
	}
#endif
	// mclcb->stats_time_count = STATS_PERIOD / MCL_TIMER_PERIOD;
						/* for periodic stats print */
}


/*
 * Print receiving side statistics
 */
void
mcl_print_rx_stats (mcl_cb	*mclcb)
{
	int		i;
	int		max_i = MAX_NB_TX_LAYERS;
	stats_t		*sp;
	struct timeval	tv;

	ASSERT(mclcb->get_stats_level());
	sp = &(mclcb->stats);
	tv = mcl_get_tvtime();
	PRINT_OUT((stdout, "\n--- RX STATISTICS at t=%ld.%06ld ---\n",
		   tv.tv_sec, tv.tv_usec))
#ifdef NEVERDEF
	PRINT_OUT((mcl_stdout, "\n--- RX STATISTICS at t=%d ---\n",
		   mcl_time_count))
#endif
	PRINT_OUT((mcl_stdout, "\tADUs completed/announced: %ld / %ld\n",
		   sp->adus_compl, sp->adus_announced))
	/*
	 * Byte stats
	 */
	PRINT_OUT((mcl_stdout, "\trx_src_bytes=%ld  rx_fec_bytes=%ld  including dup_bytes=%ld\n\tbytes per layer:\t",
		sp->rx_bytes, sp->rx_fec_bytes, sp->rx_dupl_bytes))
	for (i=0; i < max_i; i++) {
		if (sp->rx_bytes_per_lvl[i] == 0) {
			/* no need to consider layers above and including
			 * this one, they are not used */
			max_i = i;
		} else {
			PRINT_OUT((mcl_stdout, "%ld\t",
				sp->rx_bytes_per_lvl[i]))
		}
	}
	/*
	 * Packet stats
	 */
#if defined(RLC) || defined(FLIDS)	 
	PRINT_OUT((mcl_stdout, "\n\trx_src_pkts=%ld  rx_fec_pkts=%ld  including dup_pkts=%ld  lost_pkts=%ld\n\tglobal_ineff_ratio=%.3f\n\tpkts per layer:      \t",
		sp->rx_pkts, sp->rx_fec_pkts,
		sp->rx_dupl_pkts, 
		sp->rx_lost_pkts,
		(float)(sp->rx_pkts + sp->rx_fec_pkts) / (float)(sp->rx_pkts  + sp->rx_fec_pkts - sp->rx_dupl_pkts)))
#else
	PRINT_OUT((mcl_stdout, "\n\trx_src_pkts=%ld  rx_fec_pkts=%ld  including dup_pkts=%ld \n\tglobal_ineff_ratio=%.3f\n\tpkts per layer:      \t",
		sp->rx_pkts, sp->rx_fec_pkts,
		sp->rx_dupl_pkts, 
		(float)(sp->rx_pkts + sp->rx_fec_pkts) / (float)(sp->rx_pkts  + sp->rx_fec_pkts - sp->rx_dupl_pkts)))
#endif	
	for (i=0; i < max_i; i++) {
		PRINT_OUT((mcl_stdout, "%ld\t", sp->rx_pkts_per_lvl[i]))
	}
	PRINT_OUT((mcl_stdout, "\n\tundup pkts per layer:\t"))
	for (i=0; i < max_i; i++) {
		PRINT_OUT((mcl_stdout, "%ld\t", sp->rx_undup_pkts_per_lvl[i]))
	}
#if defined(RLC) || defined(FLIDS)
	PRINT_OUT((mcl_stdout, "\n\tlost pkts per layer:\t"))
	for (i=0; i < max_i; i++) {
		PRINT_OUT((mcl_stdout, "%ld\t", sp->rx_lost_per_lvl[i]))
	}
#endif	
	PRINT_OUT((mcl_stdout, "\n\tData+SIG: tot_snt=%ld  tot_rx=%ld bytes\n\tbuf_space=%ld  max_buf_space=%ld\n\terrors=%ld  bad_demux_label=%ld  adu_announced=%ld  adu_completed=%ld\n",
		sp->tx_totbytes, sp->rx_totbytes,
		sp->buf_space, sp->max_buf_space,
		sp->errors, sp->bad_demux_label,
		sp->adus_announced, sp->adus_compl))
	// mclcb->stats_time_count = STATS_PERIOD / MCL_TIMER_PERIOD;
						/* for periodic stats print */
}


/*
 * End of reception stats
 */
void
mcl_print_final_stats (mcl_cb	*mclcb)
{
	int	i;
	stats_t	*sp;

	ASSERT(mclcb->get_stats_level());
	sp = &(mclcb->stats);
	PRINT_OUT((mcl_stdout, "\n--- END OF RX TIMES ---\n"))

	i = sp->finish_index - 1;
	if (i < 0)
		i = MAX_FIN_TIME - 1;
	do {
		ASSERT(i >= 0 && i < MAX_FIN_TIME);
		if (sp->finish_times[i][STATS_FIN_TIME] == 0 &&
		    sp->finish_times[i][STATS_SEQ] == 0)
			return;
		PRINT_OUT((mcl_stdout, "\t\tADU seq=%d  time=%d\n",
			(int)sp->finish_times[i][STATS_SEQ],
			(int)sp->finish_times[i][STATS_FIN_TIME]))
		i--;
		if (i < 0)
			i = MAX_FIN_TIME - 1;
	} while (i != sp->finish_index);
}


#ifdef GET_SYSINFO
/*
 * 
 */
void
mcl_print_sysinfo (mcl_cb	*mclcb)
{
	struct timeval	time;

	time = mcl_get_tvtime();
	PRINT_OUT((mcl_stdout, "\n%ld.%06ld\tsysinfo  ", time.tv_sec, time.tv_usec))
	/*
	 * edit as required... choose user and appli name...
	 * ugly but simple!
	 */
#if defined(SOLARIS)
	system("ps -uroca -o pcpu,pmem,vsz,comm | grep mcl");
	/*
	system("top -Uroca -b -d1 | grep mcl &");
	*/
#elif defined(LINUX)
	/* broken on linux... */
	/*
	 * if (mclcb->sender)
	 * 	system("top -b -n1 | grep mclsend &");
	 * else
	 * 	system("top -b -n1 | grep mclrecv &");
	 */
#endif
}
#endif /* GET_SYSINFO */

