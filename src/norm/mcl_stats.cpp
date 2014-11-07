/* $Id: mcl_stats.cpp,v 1.4 2005/01/11 13:12:39 roca Exp $ */
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


mcl_stats::mcl_stats ()
{
	memset(this, 0, sizeof(*this));
}


mcl_stats::~mcl_stats ()
{
}


/*
 * Print sending side statistics
 */
void
mcl_stats::print_tx_stats (const mcl_cb *const mclcb)
{
	struct timeval	tv;

	//ASSERT(mclcb->get_stats_level() > 0);
	tv = mcl_get_tvtime();
	PRINT_OUT((stdout, "\n--- TX STATISTICS at t={%d,%d} ---\n",
		   (int)tv.tv_sec, (int)tv.tv_usec))
	PRINT_OUT((mcl_stdout, "\ttx_bytes=%u  retx_bytes=%u  tx_fec_bytes=%u  tx_sig_bytes=%u\n",
		this->tx_data_bytes, this->tx_retx_bytes,
		this->tx_fec_bytes, this->tx_sig_bytes))
	PRINT_OUT((mcl_stdout, "\ttx_pkts =%u  retx_pkts =%u  tx_fec_pkts =%u  tx_sig_pkts =%u\n",
		this->tx_data_pkts, this->tx_retx_pkts,
		this->tx_fec_pkts, this->tx_sig_pkts))
	PRINT_OUT((mcl_stdout, "\tData+SIG: tot_snt=%u  tot_rx=%u bytes\n\tbuf_space=%u  max_buf_space=%u  adu_announced=%u\n",
		this->tx_tot_bytes, this->rx_tot_bytes,
		this->buf_space, this->max_buf_space,
		this->adus_announced))
#if defined(SIMUL_TX_LOSSES) || defined(SIMUL_RX_LOSSES)
	if (this->tx_simul_loss_sent > 0) { 
		PRINT_OUT((mcl_stdout, "\tsimul_losses: lost=%u  loss_ratio=%.3f\n",
		this->tx_simul_loss_lost,
		(float)this->tx_simul_loss_lost/(float)this->tx_simul_loss_sent))
	}
#endif
}


void
mcl_stats::print_rx_stats (const mcl_cb *const mclcb)
{
	struct timeval	tv;

	//ASSERT(mclcb->get_stats_level() > 0);
	tv = mcl_get_tvtime();
	PRINT_OUT((stdout, "\n--- RX STATISTICS at t={%d,%d} ---\n",
		   (int)tv.tv_sec, (int)tv.tv_usec))
	PRINT_OUT((mcl_stdout, "\tADUs completed/announced: %u / %u\n",
		   this->adus_completed, this->adus_announced))
	/*
	 * Byte stats
	 */
	PRINT_OUT((mcl_stdout, "\trx_bytes=%u  rx_fec_bytes=%u  including dup_bytes=%u\n",
		this->rx_data_bytes, this->rx_fec_bytes, this->rx_dupl_bytes))
	/*
	 * Packet stats
	 */
	PRINT_OUT((mcl_stdout, "\trx_pkts=%u  rx_fec_pkts=%u  including dup_pkts=%u  lost_pkts=%u\n\tdup/rx=%.3f  rx/undup=%.2f\n",
		this->rx_data_pkts, this->rx_fec_pkts,
		this->rx_dupl_pkts, this->rx_lost_pkts,
		(float)this->rx_dupl_pkts / (float)(this->rx_data_pkts + this->rx_fec_pkts),
		(float)(this->rx_data_pkts + this->rx_fec_pkts) / (float)(this->rx_data_pkts  + this->rx_fec_pkts - this->rx_dupl_pkts)))
	PRINT_OUT((mcl_stdout, "\tData+SIG: tot_snt=%u  tot_rx=%u bytes\n\tbuf_space=%u  max_buf_space=%u\n\terrors=%u  adu_announced=%u  adu_completed=%u\n",
		this->tx_tot_bytes, this->rx_tot_bytes,
		this->buf_space, this->max_buf_space,
		this->errors,
		//this->bad_demux_label,
		this->adus_announced, this->adus_completed))
}


/*
 * End of reception stats
 */
void
mcl_stats::print_final_stats (const mcl_cb	*const mclcb)
{
	INT32		i;

	PRINT_OUT((mcl_stdout, "\n--- END OF RX TIMES ---\n"))

	i = this->finish_index - 1;
	if (i < 0)
		i = MAX_FIN_TIME - 1;
	do {
		ASSERT(i >= 0 && i < MAX_FIN_TIME);
		if (this->finish_times[i][STATS_FIN_TIME] == 0 &&
		    this->finish_times[i][STATS_SEQ] == 0)
			return;
		PRINT_OUT((mcl_stdout, "\t\tADU seq=%d  time=%d\n",
			(int)this->finish_times[i][STATS_SEQ],
			(int)this->finish_times[i][STATS_FIN_TIME]))
		i--;
		if (i < 0)
			i = MAX_FIN_TIME - 1;
	} while (i != this->finish_index);
}


void
mcl_print_sent_du (mcl_cb		*const mclcb,
		   //common_norm_hdr_t	*hdr,
		   mcl_du		*const du)
{
	if (!du) {
		PRINT_OUT((mcl_stdout, "CMD pkt time=%d\n", mcl_time_count))
		return;
	}
	if (mclcb->get_verbosity() == 2) {
		/*
		 * produce traces for timely analysis
		 */
		struct timeval	time;
		time = mcl_get_tvtime();
		PRINT_OUT((mcl_stdout, "\n%ld.%06ld\tpkt_sent aseq=%d bseq=%d dseq=%d %s\n",
			time.tv_sec, time.tv_usec, 
			du->block->adu->get_seq(), du->block->seq, du->seq,
			(du->is_fec ? "fec" : "data")))
	} else if (mclcb->get_verbosity() >= 3) {
		PRINT_OUT((mcl_stdout,
			"pkt time=%d aseq=%d/bseq=%d/dseq=%d/%s\n",
			mcl_time_count, 
			du->block->adu->get_seq(), du->block->seq, du->seq,
			(du->is_fec ? "fec" : "data")))
	}
}


void
mcl_print_recvd_du (mcl_cb	*const mclcb,
		   bool		is_data,
		   UINT32	val,		// add info, eg NEW_ADU, CLOSE
		   mcl_data_hdr_infos_t	*dhdr_infos)
{
	if (mclcb->get_verbosity() <= 0)
		return;
	if (is_data) {
		switch (mclcb->get_verbosity()) {
		case 1:
			/*
			 * produce traces for standard compact analysis
			 */
			PRINT_OUT((mcl_stdout, "\t%s  %d/%d/%d\n",
				(dhdr_infos->is_fec ? "fec " : "data"),
				dhdr_infos->idf_adu,
				dhdr_infos->idf_block, dhdr_infos->idf_du))

			break;
		case 2:
			/*
			 * produce traces for timely analysis
			 */
			struct timeval	time;
			time = mcl_get_tvtime();
			PRINT_OUT((mcl_stdout, "\n%ld.%06ld\tpkt_rcvd aseq=%d bseq=%d dseq=%d %s\n",
				time.tv_sec, time.tv_usec, 
				dhdr_infos->idf_adu,
				dhdr_infos->idf_block, dhdr_infos->idf_du,
                        	(dhdr_infos->is_fec ? "fec" : "data")))
			break;
		default:
			/*
			 * produce traces for detailed analysis
			 */
			PRINT_OUT((mcl_stdout,
			"pkt time=%d aseq=%d/bseq=%d/dseq=%d/%s\n",
				mcl_time_count, dhdr_infos->idf_adu,
				dhdr_infos->idf_block, dhdr_infos->idf_du,
                        	(dhdr_infos->is_fec ? "fec" : "data")))
			break;
		}
	} else if (val == MCL_SIG_NEWADU) {
		PRINT_OUT((mcl_stdout, "New ADU (%d: %d bytes, %d blocks)\n",
			dhdr_infos->idf_adu, dhdr_infos->adu_len,
			(int)ceil((double)dhdr_infos->adu_len /
				  (double)dhdr_infos->block_len)))
	} else if (val == MCL_SIG_NONEWADU) {
		PRINT_OUT((mcl_stdout, "No New ADU (seq: %d to ?)\n", mcl_iss))
		//PRINT_OUT((mcl_stdout, "No New ADU (seq: %d to %d)\n",
		//	mcl_iss, dhdr_infos->max_idf_adu))
	} else if (val == MCL_SIG_CLOSE) {
		PRINT_OUT((mcl_stdout, "\nClose\n"))
	}
}
