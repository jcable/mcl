/* $Id: mcl_stats.h,v 1.4 2005/01/11 13:12:39 roca Exp $ */
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

#ifndef MCL_STATS_H
#define MCL_STATS_H


/*
 * statistics collected at the receiver and sender sides
 */
#define MAX_FIN_TIME		512

#define	STATS_FIN_TIME		0
#define STATS_SEQ		1


/**
 * Statistics class.
 */
class mcl_stats {

public:
	/****** Public Members ************************************************/
	mcl_stats ();
	~mcl_stats ();

	void	print_tx_stats (const mcl_cb *const mclcb);
	void	print_rx_stats (const mcl_cb *const mclcb);
	void	print_final_stats (const mcl_cb *const mclcb);

	/****** Public Attributes *********************************************/

	/*
	 * outgoing traffic stats
	 * NB: byte counters include both data and NORM headers
	 */
	UINT32	tx_data_pkts;		/* nb of data packets sent */
					/* (doesn't include fec or retx pkts) */
	UINT32	tx_data_bytes;		/* nb of data bytes sent*/
					/* (doesn't include fec or retx pkts) */
	UINT32	tx_retx_pkts;		/* nb of data pkts re-transmited */
	UINT32	tx_retx_bytes;		/* nb of data bytes sent*/
	UINT32	tx_sig_pkts;		/* signaling only packets sent*/
	UINT32	tx_sig_bytes;		/* nb signaling bytes sent */
	UINT32	tx_fec_pkts;		/* nb of fec data pkts sent */
	UINT32	tx_fec_bytes;		/* total nb of fec bytes sent */
	UINT32	tx_tot_pkts;		/* total data/sig/fec pkts sent */
	UINT32	tx_tot_bytes;		/* total data/sig/fec bytes sent */
#if defined(SIMUL_TX_LOSSES) || defined(SIMUL_RX_LOSSES)
	UINT32	tx_simul_loss_sent;	/* total nb of non lost pkts */
	UINT32	tx_simul_loss_lost;	/* total nb of simul lost pkts*/
#endif
	/*
	 * incoming traffic stats
	 * NB: byte counters include both data and NORM headers
	 */
	UINT32	rx_data_pkts;		/* total nb of packets recvd */
					/* (doesn't include fec or retx pkts) */
	UINT32	rx_data_bytes;		/* nb of data bytes received */
					/* (doesn't include fec or retx pkts) */
	UINT32	rx_sig_pkts;		/* signaling only packets recvd */
	UINT32	rx_sig_bytes;		/* nb signaling bytes recvd */
	UINT32	rx_fec_pkts;		/* nb of fec pkts recvd */
	UINT32	rx_fec_bytes;		/* nb of fec bytes recvd */
	UINT32	rx_undup_pkts;		/* unduplicated pkts recvd */
	UINT32	rx_dupl_pkts;		/* # pkts recvd several times */
	UINT32	rx_dupl_bytes;		/* # bytes recvd several times*/
	UINT32	rx_tot_pkts;		/* total data/sig/fec pkts recvd */
	UINT32	rx_tot_bytes;		/* total data/sig/fec bytes recvd */

	UINT32	rx_lost_pkts;		/* total nb of lost pkts */

	ushort	finish_index;		/* next available entry in tab*/
	UINT32	finish_times[MAX_FIN_TIME][2];	/* circular buffer of finish */
					/* times [][0] & seq nb [][1] */
	UINT32	adus_completed;		/* nb of ADUs completed */
	/*
	 * errors
	 */
	UINT32	errors;			/* nb of erroneous DUs recvd */
	UINT32	bad_hdr;		/* error in header */
	//UINT32	bad_demux_label;	/* wrong LCT demux label */
	UINT32	other_errors;		/* non packet related errors */
	/*
	 * common stats
	 */
	UINT32	buf_space;		/* current allocated buf space*/
	UINT32	max_buf_space;		/* max allocated buffer space */
	UINT32	adus_announced;		/* nb of ADUs announced */
 
private:
	/****** Private Members ***********************************************/
	/****** Private Attributes ********************************************/
 
};


/**
 * Print a sent DU in an understandable way.
 * @param mclcb
 * @param type
 * @param val
 * @param hdr_infos	pointer to the header_info structure, that contains
 *			all the information concerning the sent DU.
 */
extern void	mcl_print_sent_du	(class mcl_cb *const mclcb,
					 class mcl_du *const du);

/**
 * Print a received DU in an understandable way.
 * @param mclcb
 * @param type
 * @param val
 * @param hdr_infos	pointer to the header_info structure, that contains
 *			all the information concerning a recevied DU.
 */
extern void	mcl_print_recvd_du	(class mcl_cb *const mclcb,
					 bool is_data, UINT32 val,
					 struct mcl_data_hdr_infos *dhdr_infos);


//------------------------------------------------------------------------------
// Inlines for all classes follow
//------------------------------------------------------------------------------

#endif // !MCL_STATS_H
