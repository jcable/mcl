/* $Id: mcl_flid_sl.h,v 1.9 2005/01/11 13:12:28 roca Exp $ */
/*
 *  Copyright (c) 1999-2004 INRIA - All rights reserved
 *  (main authors: Julien Laboure - julien.laboure@inrialpes.fr)
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
 * This file contains headers for FLID SL congestion control module
 * (Static Layer).
 * See Internet Draft LCC for more informations.
 */

#ifndef MCL_FLID_SL_H  /* { */
#define MCL_FLID_SL_H

#ifdef FLIDS /* { */


/**
 * The 32 bit, ALC compliant, FLID-SL congestion control header block.
 */
typedef struct {
#ifdef _BIT_FIELDS_LTOH
	UINT8	tsi:7,		/* Time Slot Index (TSI) */
		trigger:1;	/* Increase signal Trigger (T) */
#else
	UINT8	trigger:1,
		tsi:7;
#endif
	UINT8	gn;		/* packet's Group Number (GN) */
	UINT16	seqno;		/* packet's Sequence Number (SEQNO) */
} flids_hdr_t;


/**
 * FLID-SL control block.
 * Contains all information required for a sending or receiving FLID-SL
 * session.
 */
typedef struct {
	/**
	 * The session state.
	 *	1: initial state
	 *	2: working state
	 */
	UINT8	SessionState;

	/**
	 * The FLID-SL Time Slot ID.
	 * WARNING: this is not the ALC/LCT Transport Session ID!
	 * WARNING: TSI must use only 7 bits.
	 */
	UINT8	tsi;	

	/**
	 * The long version of the TSI.
	 */
	INT32	long_tsi;

	/**
	 * Time Slot Duration (TSD).
	 */
	INT32	tsd;

	/**
	 * For each layer, the value of the current sequence number.
	 */
	UINT16	tx_LayerSeq[MAX_NB_TX_LAYERS];


	UINT8	tx_LayerTrig[MAX_NB_TX_LAYERS];

	/**
	 * For each layer, the current state.
	 *	1: initial state
	 *	2: TIMESLOT init state
	 *	3: working state
	 */
	UINT8	rx_LayerState[MAX_NB_TX_LAYERS];

	/**
	 * For each layer, the Increase Trigger value.
	 */
	UINT8	rx_IncreaseTrigger[MAX_NB_TX_LAYERS];

	/**
	 * For each layer, the following sequence number waited for.
	 */
	UINT16	rx_WaitFor[MAX_NB_TX_LAYERS];

	/**
	 * 
	 */
	UINT8	rx_DeafPeriod;

	/**
	 * 
	 */
	UINT8	rx_CongestionDetected;

	/**
	 * Remaining time_count till next call to FLIDs_TxTimer function.
	 */
	mcl_itime_t	flids_tx_timer_count;

	/**
	 * Deaf Period Duration in multiple of TSD
	 */
	UINT8	flids_deaf_period;
} flids_cb_t;


/* For infos about all these constants,		*/
/* refer to the descriptions of the		*/
/* corresponding flids_cb member variables	*/

#define FLIDS_TSD		1000000	/* TimeSlot Duration in milliseconds. */
					/* should be either 0.5, 1.0, 2.0 sec */
#define FLIDS_DEAF_PERIOD	9	/* Deaf Period Duration in multiple of*/
					/* FLIDS_TSD */
					/* Default value: 9 seconds */


/*
 * FLID-SL function prototypes.
 */

/**
 * Initializes the FLID-SL congestion control mechanism for a given session.
 * This must be the called before any other calls to FLID-SL functions
 * @param mclcb         session Control Block
 */
extern void	FLIDs_InitSession	(class mcl_cb *mclcb);


/**
 * Must be called when ending a session.
 * @param mclcb         session Control Block
 */
extern void	FLIDs_EndSession	(class mcl_cb *mclcb);


/**
 * Timer function, called periodically by the sender.
 * @param mclcb         session Control Block
 */
extern void	FLIDs_TxTimer		(class mcl_cb *mclcb);


/**
 * Increase the Time Slot Index (TSI) periodically (the period is TSD).
 * @param mclcb         session Control Block
 */
extern void	FLIDs_NewTimeSlot	(class mcl_cb *mclcb);

/**
 * Build the FLID-SL header for each packet to send.
 * @param mclcb         session Control Block
 * @param hdr_buff	points to a buffer receiving the FLIDs header
 * @param grp_idx	index of the group within the set of groups used for
 * 			that session
 * @return		MCL_OK (0) if success, else the corresponding error code
 */
extern int	FLIDs_tx_FillHeader	(class mcl_cb *mclcb,
					 flids_hdr_t *hdr_buff, UINT8 grp_idx);

/**
 * Analyze the packet FLID-SL header.
 * @param mclcb         session Control Block
 * @param hdr_buff	pointer to the FLID-SL header.
 * @return		the layer number (>=0) if success, else the error
 *			code as defined in mcl_error.h.
 */
extern int	FLIDs_rx_AnalyzePacket	(class mcl_cb *mclcb,
					 flids_hdr_t *hdr_buff);

/**
 * Used to set various FLID-SL options and parameters.
 * @param mclcb		session Control Block
 * @param optname	a defined option name -> FLIDS_OPT_*
 * @param optvalue	pointer to the value for this option.
 * @param optlen	size of the argument pointed by optvalue.
 * @return		Completion status (MCL_OK or MCL_ERROR)
 */
extern mcl_error_status	FLIDs_ctl	(class mcl_cb *mclcb, INT32 optname,
					 void *optvalue, INT32 optlen);


#endif /* } FLIDS */

#endif /* }  MCL_FLID_SL_H */
