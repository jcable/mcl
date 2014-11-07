/* $Id: mcl_rlc.h,v 1.7 2005/01/11 13:12:32 roca Exp $ */
/*
 *  Copyright (c) 1999-2004 INRIA - All rights reserved
 *  (main authors: Julien Laboure - julien.laboure@inrialpes.fr
 *                 Vincent Roca - vincent.roca@inrialpes.fr)
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

/*
 * This file contains the headers for RLC congestion control module.
 */
#ifndef MCL_RLC_H  /* { */
#define MCL_RLC_H

#ifdef RLC

/**
 * The 32 bit, ALC compliant, RLC congestion control header block.
 */
typedef struct {
#ifdef  _BIT_FIELDS_LTOH
	UINT8	rlc_reserved:7,	/* Unused, must be 0x55 (1010101) */
		rlc_sp:1;	/* Is this pkt a Synchronisation Point (SP)? */
#else
	UINT8	rlc_sp:1,	/* Is this pkt a Synchronisation Point (SP)? */
		rlc_reserved:7;	/* Unused, must be 0x55 (1010101) */
#endif
	UINT8  rlc_layer; /* packet's layer (indice) */
	UINT16 rlc_seqid; /* packet's Sequence number (per layer sequence) */
} rlc_hdr_t;


typedef struct late_list {
	struct late_list *next;	/* next late */
	UINT16	seq_num;/* RLC Sequence Number */
	int		ttw;	/* Time To Wait before considering lost */
} late_list_t;


typedef struct lost_list {
	struct lost_list *next;	/* next missing */
	int pkt_remaining; /* Number of packets to receive before we forget
				this one */
} lost_list_t;


/**
 * RLC control block.
 * Contains all information required for a sending or receiving RLC session.
 */
typedef struct rlccb {
	/** Interval between 2 SPs at layer 0 (in µsec). */
	INT32 rlc_sp_cycle;

	/** Default Time To Wait for a late packet before assuming it's lost.*/
	INT32 rlc_pkt_timeout;

	/** Time for Deaf period after dropping a layer (in µsec). */
	INT32 rlc_deaf_period;

	/**
	 * If the amount of late packets between 2 SPs at top layer is
	 * <= rlc_late_accepted then a layer can be nonetheless added.
	 */
	INT32 rlc_late_accepted;

	/**
	 * If the amount of lost packets between 2 SPs at top layer is
	 * <= rlc_loss_accepted then a layer can be nonetheless added.
	 */
	INT32 rlc_loss_accepted;

	/**
	 * (rlc_loss_limit/rlc_loss_timeout) is the max loss rate for packet.
	 * If this rate is reached then we should drop the highest layer.
	 */
	INT32 rlc_loss_limit;

	/**
	 * (rlc_loss_limit/rlc_loss_timeout) is the max loss rate for packet.
	 * If this rate is reached then we should drop the highest layer.
	 */
	INT32 rlc_loss_timeout;

	/** Aggressive congestion ctrl setup for LAN tx. */
	INT32 rlc_lan_cc;
	
	/**
	 * For each layer, this table contains the value of the current
	 * sequence number.
	 */
	UINT16	tx_layers_seq[MAX_NB_TX_LAYERS];

	/** For each layer, this table contains the date for the next SP. */
	UINT32 	tx_next_sp[MAX_NB_TX_LAYERS];

	/**
	 * For each layer, this table contains true if we are waiting
	 * for the first packet
	 */
	bool 	rx_first_pkt[MAX_NB_TX_LAYERS];

	/**
	 * For each layer, this table contains true if we are waiting
	 * for the first SP after deaf.
	 */
	bool 	rx_first_sp[MAX_NB_TX_LAYERS];

	/**
	 * For each layer, this table contains the expected sequence number
	 * of the next packet to receive.
	 */
	UINT16 	rx_wait_for[MAX_NB_TX_LAYERS];

	/**
	 * For each layer, this table contains a list of missing seq numbers.
	 */
	late_list_t	rx_missing[MAX_NB_TX_LAYERS];

	/** Number of late packets since the last SP. */
	UINT16	rx_nblate_since_sp;

	/** Number of recent late packets. */
	UINT16	rx_nblate;

	/** Number of lost packets since the last SP. */
	UINT16	rx_nblost_since_sp;

	/** Number of of recent lost packets */
	UINT16	rx_nblost;

	/** Current list of lost packets. */
	lost_list_t	rx_lost;

	/**
	 * When in deaf period, it specifies the number of calls to 
	 * rlc_rx_timer remaining before the end of the deaf period.
	 */	
	mcl_itime_t	rx_deaf_wait;

	/** Remaining time_count till next call to rlc_rx_timer function. */
	mcl_itime_t	rlc_rx_timer_count;
} rlccb_t;


/* For infos about all these constants,	*/
/* refer to the descriptions of the	*/
/* corresponding rlccb member variables	*/
/*
 * Internet version
 */
#define RLC_SP_CYCLE		250000	 /* 0.25s, for fast layer addition */
#define RLC_DEAF_PERIOD		10000000 /* 10s, due to IGMP leave latency */
#define RLC_LATE_ACCEPTED	0
#define RLC_LOSS_ACCEPTED	0
#define RLC_PKT_TIMEOUT		500000
#define RLC_LOSS_LIMIT		1
#define RLC_LOSS_TIMEOUT	20
#define RLC_MAX_LATES		100
 
/*
 * Aggressive version
 *
 * This profile is more robust to packet losses and can be used
 * in situations where packet losses may be caused by something
 * else than congestion, or with multicast routing protocols that
 * can lead to an instable initial situation (e.g. with PIM-SM
 * while moving from the shared tree to the source specific tree).
 * Use with care...
 *
 * This mode can be set by changing the "rlc_aggr_cc" field in
 * file mcl_tx_prof.cpp.
 */
#define RLC_LAN_DEAF_PERIOD	1000000	/* small (1s) as a LAN reacts quickly */
#define RLC_LAN_LATE_ACCEPTED	1
#define RLC_LAN_LOSS_ACCEPTED	1
//#define RLC_LAN_LOSS_LIMIT	10	/* higher loss limit */
#define RLC_LAN_LOSS_LIMIT	2	/* higher loss limit */


/**
 * Period in microseconds between two calls to the RLC timer function at
 * a receiver.
 * No call is made to the RLC timer function at a sender!
 */
extern UINT32	mcl_rlc_rx_period;


/*
 * RLC function prototypes.
 */

/**
 * Initializes the RLC congestion control mechanism for a given session.
 * This must be the called before any other calls to RLC functions
 * @param mclcb		session Control Block
 */
extern void	rlc_init_session	(class mcl_cb *mclcb);


/**
 * Reset all SP variables at the sending side.
 * Done just after transmitting the first packet to have synchronized SPs
 * @param mclcb		session Control Block
 */
extern void	rlc_reset_tx_sp		(class mcl_cb *mclcb);


/**
 * Must be called when ending a session.
 * @param mclcb		session Control Block
 */
extern void	rlc_end_session		(class mcl_cb *mclcb );


/**
 * Fill the RLC header for each packet to send.
 * @param mclcb		session Control Block
 * @param hdr_buff	points to a buffer receiving the RLC header
 * @param layer		the layer for this packet
 * @return		Completion status (MCL_OK or an error code)
 */
extern INT32	rlc_tx_fill_header	(class mcl_cb *mclcb,
					 rlc_hdr_t *hdr_buff, UINT8 pkt_layer);


/**
 * Analyse the packet's RLC header.
 * @param mclcb		session Control Block
 * @param hdr_buff	pointer to the RLC header.
 * @return		the layer number (>=0) if success, else the error
 * 			code as defined in mcl_error.h.
 */
extern INT32	rlc_rx_analyze_packet	(class mcl_cb *mclcb,
					 rlc_hdr_t *hdr_buff);


/**
 * Timer function, called periodically by the receiver.
 * UINT32 mcl_rlc_rx_period = MCL_RLC_RX_PERIOD;
 * is the period in microseconds between two calls to this function.
 * @param mclcb		session Control Block
 * @return		Completion status (MCL_OK or MCL_ERROR)
 */
extern mcl_error_status	rlc_rx_timer	(class mcl_cb *mclcb);


/**
 * Used to set various RLC options and parameters.
 * @param mclcb		session Control Block
 * @param optname	a defined option name -> RLC_OPT_*
 * @param optvalue	pointer to the value for this option.
 * @param optlen	size of the argument pointed by optvalue.
 * @return		Completion status (MCL_OK or MCL_ERROR)
 */
extern mcl_error_status	rlc_ctl		(class mcl_cb *mclcb, INT32 optname,
					 void *optvalue, INT32 optlen);

#endif /* }  MCL_RLC_H */

#endif /* RLC */
