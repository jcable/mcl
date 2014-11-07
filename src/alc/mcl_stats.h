/* $Id: mcl_stats.h,v 1.4 2005/01/11 13:12:34 roca Exp $ */
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

#ifndef MCL_STATS_H  /* { */
#define MCL_STATS_H

/****** statistics ******/


/*
 * statistics collected at the receiver and sender sides
 */
#define MAX_FIN_TIME		512

#define	STATS_FIN_TIME		0
#define STATS_SEQ		1

typedef struct {
	/*
	 * outgoing traffic stats
	 */
	UINT32	tx_pkts;			/* nb of data packets sent */
	UINT32	tx_sig_pkts;			/* sig only packets sent */
	UINT32	tx_bytes;			/* nb of data bytes sent*/
	UINT32	tx_totbytes;			/* total data+sig bytes sent */
	UINT32	tx_fec_pkts;			/* nb of fec data pkts sent */
	UINT32	tx_fec_bytes;			/* total nb of fec bytes sent */
	UINT32	tx_pkts_per_lvl[MAX_NB_TX_LAYERS];	/* packets sent on each level */
	UINT32	tx_bytes_per_lvl[MAX_NB_TX_LAYERS];	/* data bytes sent per level */
#ifdef SIMUL_LOSSES
	UINT32	tx_simul_loss_sent;		/* total nb of non lost pkts */
	UINT32	tx_simul_loss_lost;		/* total nb of simul lost pkts*/
#endif
	/*
	 * incoming traffic stats
	 */
	UINT32	rx_pkts;			/* total nb of packets recvd */
						/* (doesn't include fec pkts) */
	//UINT32	rx_sig_pkts;		/* signaling packets recvd */
	UINT32	rx_bytes;			/* nb of data bytes received */
						/* (doesn't include fec pkts) */
	UINT32	rx_totbytes;			/* total data+sig bytes recvd */
	UINT32	rx_fec_pkts;			/* total nb of fec pkts recvd */
	UINT32	rx_fec_bytes;			/* total nb of fec bytes recvd*/
	UINT32	rx_pkts_per_lvl[MAX_NB_TX_LAYERS];	/* packets recvd on each layer */
						/* (includes data+fec packets)*/
	UINT32	rx_bytes_per_lvl[MAX_NB_TX_LAYERS];/* bytes received on each lay */
						/* (includes data+fec packets)*/
	UINT32	rx_undup_pkts_per_lvl[MAX_NB_TX_LAYERS];/* unduplicated pkts recvd*/
	UINT32	rx_dupl_pkts;			/* # pkts recvd several times */
	UINT32	rx_dupl_bytes;			/* # bytes recvd several times*/

#if defined(RLC) || defined(FLIDS)
	UINT32	rx_lost_pkts;			/* total nb of lost pkts recvd*/
	UINT32	rx_lost_per_lvl[MAX_NB_TX_LAYERS];/* # lost (or delayed!) pkts*/
#endif

	UINT16	finish_index;			/* next available entry in tab*/
	UINT32	finish_times[MAX_FIN_TIME][2];	/* circular buffer of finish */
						/* times [][0] & seq nb [][1] */
	UINT32	adus_compl;			/* nb of ADUs completed */
	/*
	 * errors
	 */
	UINT32	errors;				/* nb of erroneous DUs recvd */
	UINT32	bad_hdr;			/* error in header */
	UINT32	bad_demux_label;		/* wrong LCT demux label */
	UINT32	other_errors;			/* non packet related errors */
	/*
	 * common stats
	 */
	UINT32	buf_space;			/* current allocated buf space*/
	UINT32	max_buf_space;			/* max allocated buffer space */
	UINT32	adus_announced;			/* nb of ADUs announced */
} stats_t;


/*
 * Public function prototypes.
 */

/**
 * Print transmission statistics.
 * @param mclcb
 */
extern void	mcl_print_tx_stats	(class mcl_cb *mclcb);

/**
 * Print reception statistics (e.g. upon completion of an ADU).
 * @param mclcb
 */
extern void	mcl_print_rx_stats	(class mcl_cb *mclcb);

/**
 * Print final statistics.
 * @param mclcb
 */
extern void	mcl_print_final_stats	(class mcl_cb *mclcb);

#ifdef GET_SYSINFO
extern void	mcl_print_sysinfo	(class mcl_cb *mclcb);
#endif


/**
 * Print a sent DU in an understandable way.
 * @param mclcb
 * @param type
 * @param val
 * @param hdr_infos	pointer to the header_info structure, that contains
 *			all the information concerning the sent DU.
 */

extern void     mcl_print_sent_du (class mcl_cb *mclcb, INT32 type, INT32 val,
                                 hdr_infos_t *hdr_infos);

/**
 * Print a received DU in an understandable way.
 * @param mclcb
 * @param type
 * @param val
 * @param hdr_infos	pointer to the header_info structure, that contains
 *			all the information concerning a recevied DU.
 */
extern void	mcl_print_recvd_du	(class mcl_cb *mclcb, INT32 type,
					INT32 val, hdr_infos_t *hdr_infos);

#endif /* }  MCL_STATS_H */
