/* $Id: mcl_flid_sl.cpp,v 1.17 2005/01/11 13:12:27 roca Exp $ */
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
 * LCC congestion control module FLID SL (Static Layer).
 */

#include "mcl_includes.h"

#ifdef FLIDS /* { */


/*
 * Local private function prototypes
 */
static INT32	CheckSequence	(mcl_cb *mclcb, UINT8 layer, UINT16 seqid,
				 UINT8 tsi);
static INT32	AddLate		(mcl_cb *mclcb, INT32 layer, INT32 nseq);
static INT32	RemoveLate	(mcl_cb *mclcb, INT32 layer, INT32 nseq);
static void	ComputeTSI	(mcl_cb *mclcb);


/****** PUBLIC FUNCTIONS ******************************************************/


/**
 * Initializes the RLC congestion control mechanism for a given session.
 * => See header file for more informations.
 */
void
FLIDs_InitSession (mcl_cb	*mclcb)
{
	INT32		i;
	flids_cb_t	*cb;

	TRACELVL( 5, (mcl_stdout,
		"-> FLIDs: FLIDs_InitSession: mclcb=x%x\n", (int) mclcb))
	/* WARNING: no use of modes? */
	cb = &(mclcb->flids_cb);
	memset((void *)cb, 0, sizeof(flids_cb_t));

	cb->SessionState = 1; /* Initial State */
	cb->tsd = FLIDS_TSD;
	cb->tsi = 0;
	cb->long_tsi = 0;
	cb->rx_CongestionDetected = 0;
	cb->rx_DeafPeriod = 0;
	cb->flids_deaf_period = FLIDS_DEAF_PERIOD;
	cb->flids_tx_timer_count = 0;

	for( i=0; i<MAX_NB_TX_LAYERS; i++)
	{
		cb->rx_LayerState[i] = 1;
		cb->tx_LayerSeq[i] = 0;
		cb->rx_IncreaseTrigger[i] = 0;
		cb->rx_WaitFor[i] = 0;
		cb->tx_LayerTrig[i] = 0;
	}

	TRACELVL(5, (mcl_stdout, "<- FLIDs: FLIDs_InitSession\n"))
}


/**
 * Must be called when ending a session.
 * => See header file for more informations.
 */
void
FLIDs_EndSession (mcl_cb	*mclcb )
{
	TRACELVL(5, (mcl_stdout, "-> FLIDs: FLIDs_EndSession\n"))
	// Nothing...
	TRACELVL(5, (mcl_stdout, "<- FLIDs: FLIDs_EndSession\n"))
}


/**
 * Timer function, called periodically by the sender.
 * => See header file for more informations.
 */
void
FLIDs_TxTimer (mcl_cb	*mclcb)
{
	FLIDs_NewTimeSlot(mclcb);
}


/**
 * Increase the Time Slot Index (TSI) periodically (the period is TSD).
 * => See header file for more informations.
 */
void
FLIDs_NewTimeSlot (mcl_cb	*mclcb)
{
	flids_cb_t	*cb;

	TRACELVL(5, (mcl_stdout, "-> FLIDs: FLIDs_NewTimeSlot\n"))
	ASSERT(mclcb!=NULL)
	cb = &(mclcb->flids_cb);

	cb->tsi = (cb->tsi+1)%128; /* WARNING: why not using 8bits for TSI? */
	cb->long_tsi++;
	if (mclcb->get_verbosity() == 2) {
		struct timeval	time;
		time = mcl_get_tvtime();
		PRINT_OUT((mcl_stdout, "\n%ld.%06ld\tnew_TS %ld\n",
		time.tv_sec, time.tv_usec, cb->long_tsi))
	}
	ComputeTSI(mclcb);
	TRACELVL(5, (mcl_stdout, "<- FLIDs: FLIDs_NewTimeSlot\n"))
}


/**
 * Build the FLID-SL header for each packet to send.
 * => See header file for more informations.
 */
INT32
FLIDs_tx_FillHeader (mcl_cb		*mclcb,
		     flids_hdr_t	*hdr_buff,
		     UINT8		grp_idx)
{
	UINT16		hdr_seqno;
	flids_cb_t	*cb;

	TRACELVL(5, (mcl_stdout,
		"-> FLIDs: FLIDs_tx_FillHeader: mclcb=x%x, grp_idx=%d\n",
		(int) mclcb, grp_idx))
	ASSERT(mclcb!=NULL)
	cb = &(mclcb->flids_cb);

	ASSERT(grp_idx < MAX_NB_TX_LAYERS && grp_idx < mclcb->nb_layers)
	ASSERT(hdr_buff!=NULL)

	hdr_seqno = cb->tx_LayerSeq[grp_idx];
	(cb->tx_LayerSeq[grp_idx])++;

	hdr_buff->seqno = htons(hdr_seqno);	/* Sequence number */
	hdr_buff->gn = grp_idx;				/* Index of the group */
	hdr_buff->tsi = cb->tsi;			/* Time Slot Index */

	hdr_buff->trigger = cb->tx_LayerTrig[grp_idx];
	
	if (mclcb->get_verbosity() == 2 && (hdr_seqno%10==0)) {
		struct timeval	time;
		time = mcl_get_tvtime();
		PRINT_OUT((mcl_stdout, "\n%ld.%06ld\tist_bit %d %d\n",
		time.tv_sec, time.tv_usec, hdr_buff->trigger, grp_idx))
	}

	TRACELVL(5, (mcl_stdout, "<- FLIDs: FLIDs_tx_FillHeader: ok\n"))
	return MCL_OK;
}


/**
 * Analyze the packet FLID-SL header.
 * => See header file for more informations.
 */
INT32
FLIDs_rx_AnalyzePacket (mcl_cb		*mclcb,
			flids_hdr_t	*hdr_buff)
{
	UINT8	groupn;	
	UINT16	seqno;
	UINT8	pkt_tsi;
	UINT8	new_tsi;
	UINT8	trigger;
	flids_cb_t	*cb;

	ASSERT(hdr_buff!=NULL)
	TRACELVL(5, (mcl_stdout, "-> FLIDs: FLIDs_rx_AnalyzePacket\n"))
	ASSERT(mclcb!=NULL)
	cb = &(mclcb->flids_cb);

	pkt_tsi = hdr_buff->tsi;
	new_tsi = pkt_tsi;
	trigger = hdr_buff->trigger;
	groupn = hdr_buff->gn;
	seqno = ntohs(hdr_buff->seqno);

	// printf("FLIDs_PKT layer=%d seq=%d\n", groupn, seqno);
	if(groupn >= mclcb->nb_layers) {
		/* Bad group number, so ignore... */
		TRACELVL(5, (mcl_stdout, "<- FLIDs: FLIDs_rx_AnalyzePacket (BAD_LAYER)\n"))
		return ERR_BAD_LAYER;
	}

	if (mclcb->get_verbosity() == 2 && (seqno%10==0)) {
		struct timeval	time;
		time = mcl_get_tvtime();
		PRINT_OUT((mcl_stdout, "\n%ld.%06ld\tist_bit %d %d\n",
		time.tv_sec, time.tv_usec, trigger, groupn))
	}

	if( cb->SessionState == 1 ) {
		/* Session First Packet */
		cb->tsi = pkt_tsi;
		cb->SessionState = 2;
		cb->rx_WaitFor[groupn] = seqno + 1;
		cb->rx_LayerState[groupn] = 3;
		cb->rx_IncreaseTrigger[groupn] = trigger;
		goto end;
	}
	
	if( cb->rx_LayerState[groupn] == 1) {
		/* First packet on this group...  (or first packet for a
		 * readded group) */
		cb->rx_WaitFor[groupn] = seqno + 1;
		cb->rx_LayerState[groupn]=3;
		cb->rx_IncreaseTrigger[groupn] = trigger;
		goto end;
	} else {
		/* check if some pkts have been lost or not */
		new_tsi = CheckSequence(mclcb, groupn, seqno, pkt_tsi);
	}

	/* First Packet (no matter the group) for this TimeSlot... DECISION
	 * HERE */
	if( new_tsi != cb->tsi ) {
		INT32 i = 0;

		if( new_tsi != (cb->tsi+1)%128 ) {
			TRACELVL(4, (mcl_stdout,
			"FLIDs WARNING! Unexpected TSI Value (Old=%d, New=%d), packets have probably been lost\n",cb->tsi, new_tsi))
		}

		cb->tsi = new_tsi;
		TRACELVL(3, (mcl_stdout,
		"FLIDs: Entering NEW Time Slot (tsi=%d, trigger was %d)\n",
		cb->tsi, cb->rx_IncreaseTrigger[mclcb->nb_layers-1]))

		if (mclcb->get_verbosity() == 2) {
			struct timeval	time;
			time = mcl_get_tvtime();
			PRINT_OUT((mcl_stdout, "\n%ld.%06ld\tnew_TS %d\n",
			time.tv_sec, time.tv_usec, cb->tsi))
		}

		if (cb->rx_DeafPeriod > 0 ) {
			cb->rx_DeafPeriod--;
			if (cb->rx_DeafPeriod == 0 && mclcb->get_verbosity() == 2) {
				struct timeval	time;
				time = mcl_get_tvtime();
				PRINT_OUT((stdout,
					"\n%ld.%06ld\tend_deaf\n",
					time.tv_sec, time.tv_usec))
			}
		}

		for(i=0; i<MAX_NB_TX_LAYERS; i++) {
			/* Set all LayerStates to 2 (new timeslot) */
			cb->rx_LayerState[i] = 2;
		}

		if (cb->rx_CongestionDetected) {
			if(!cb->rx_DeafPeriod) {
				/* too many losses... drop a layer! */
				if (mcl_drop_layer(mclcb, MCL_HIGHEST_LAYER, MCL_DO_IT ) < 0) {
					/* error, cannot drop layer */
					TRACELVL(3, (mcl_stdout,
					"<- FLIDs: cant drop layer, ignore\n"))
				} else {
					TRACELVL(2, (mcl_stdout,
					"    FLIDs: congestion detected, highest layer dropped, now receiving from %d layers\n", mclcb->nb_layers))
					/* Entering Deaf Period... */
					cb->rx_DeafPeriod = cb->flids_deaf_period;
					/* initial state for futur potential
					 * use of this layer*/
					cb->rx_LayerState[mclcb->nb_layers]=1;
				}
			}
			cb->rx_CongestionDetected = 0;
			goto end;

		} else if(cb->rx_IncreaseTrigger[mclcb->nb_layers-1]) {
			if  (mcl_add_layer(mclcb, MCL_HIGHEST_LAYER) == 0)
			{
				cb->rx_LayerState[mclcb->nb_layers-1]=1;
				TRACELVL(2, (mcl_stdout,
					"   FLIDs: adding a new layer, now receiving from %d layers\n", mclcb->nb_layers))
			}
		}
		cb->rx_CongestionDetected = 0;

	}

	ASSERT(cb->tsi == new_tsi)

	// HACK: if no packets received during a complete timeslot then
	// FLIDs should [MAYBE] crash HERE ;)

	if(cb->rx_LayerState[groupn]==2) {
		// First Packet ON THIS GROUP FOR THIS TIMESLOT
		cb->rx_IncreaseTrigger[groupn] = trigger;
		cb->rx_LayerState[groupn]=3;
		TRACELVL(3, (mcl_stdout,
		"First Packet for Layer %d for Timeslot %d\n", groupn, new_tsi))
	} else {
		ASSERT(cb->rx_LayerState[groupn]==3)

		if(cb->rx_IncreaseTrigger[groupn] != trigger) {
			TRACELVL(5, (mcl_stdout,
			"<- FLIDs: FLIDs_rx_AnalyzePacket (BAD_TRIGGER)\n"))
			return ERR_BAD_TRIGGER;
		}
	}

end:
	TRACELVL(5, (mcl_stdout, "<- FLIDs: FLIDs_rx_AnalyzePacket (ok)\n"))
	return groupn;
}


/**
 * Used to set various FLID-SL options and parameters.
 * => See header file for more informations.
 */
mcl_error_status
FLIDs_ctl (mcl_cb *mclcb, INT32 optname, void *optvalue, INT32 optlen)
{
	flids_cb_t *cb = &(mclcb->flids_cb);

	TRACELVL(5, (mcl_stdout,
		"-> FLIDs: FLIDs_ctl: optname=%d, optvalue=x%x, optlen=%d\n",
		optname, (int)optvalue, optlen))

	switch (optname) {
	case FLIDS_OPT_DEAF_PERIOD:
		if (!optvalue || optlen != sizeof(int)) {
			PRINT_ERR((mcl_stderr,
			"FLIDs_ctl ERROR FLIDS_OPT_DEAF_PERIOD: null optvalue or bad optlen (got %d, expected %d)\n", optlen, sizeof(int)))
			goto error;
		}
		if ((*(int*)optvalue) < 0 ) {
			PRINT_ERR((mcl_stderr,
			"FLIDs_ctl ERROR: FLIDS_OPT_DEAF_PERIOD must be positive (got %d)\n",(*(int*)optvalue)))
			goto error;
		}
		cb->flids_deaf_period = *(int*)optvalue;
		break;

	case FLIDS_OPT_TSD:
		if (!optvalue || optlen != sizeof(int)) {
			PRINT_ERR((mcl_stderr,
			"FLIDs_ctl ERROR FLIDS_OPT_TSD: null optvalue or bad optlen (got %d, expected %d)\n", optlen, sizeof(int)))
			goto error;
		}
		if ((*(int*)optvalue) < 0 ) {
			PRINT_ERR((mcl_stderr,
			"FLIDs_ctl ERROR: FLIDS_OPT_TSD must be positive (got %d)\n",(*(int*)optvalue)))
			goto error;
		}
		cb->tsd = *(int*)optvalue;
		break;

	default:
		PRINT_ERR((mcl_stderr,
		"<- FLIDs: FLIDs_ctl: ERROR, Unknown option\n"))
		goto error;
	}

	TRACELVL(5, (mcl_stdout, "<- FLIDs: FLIDs_ctl: ok\n"))
	return MCL_OK;

error:
	TRACELVL(5, (mcl_stdout, "<- FLIDs: FLIDs_ctl: ERROR\n"))
	return MCL_ERROR;
}


/****** PRIVATE FUNCTIONS *****************************************************/


/**
 * Return NEW CURRENT TSI...
 */
static int
CheckSequence (mcl_cb	*mclcb,
		UINT8	layer,
		UINT16	seqid,
		UINT8	tsi)
{
	flids_cb_t	*cb;
	UINT16		Delta1;
	UINT16		Delta2 = 0;
	UINT16		i;		/* must be u_int16! */
	UINT8		NewTSI;

	ASSERT(mclcb)
	cb = &(mclcb->flids_cb);

	TRACELVL(5, (mcl_stdout,
		"-> FLIDs: CheckSequence: seqid=%d (expected seq %d)\n",
		seqid, cb->rx_WaitFor[layer]))

	if ( cb->rx_WaitFor[layer] == seqid) {
		/* This is the packet we're waiting for, let's go on! */
		//cb->rx_WaitFor[layer] = (cb->rx_WaitFor[layer] + 1) % 65536;
		cb->rx_WaitFor[layer]++; // u_int16, so wrapping is automatic
		TRACELVL(5, (mcl_stdout, "<- FLIDs: CheckSequence: good seq\n"))
		NewTSI = tsi;
		return NewTSI;
	}

	/* This is not the one we're waiting for... */
	if (cb->rx_WaitFor[layer] < seqid) {
		Delta1 = seqid - cb->rx_WaitFor[layer];
		Delta2 = 65535 - Delta1;
		if ( Delta1 < Delta2 ) {
			/* Some packet(s) are missing */
			/* eg. wait seq 1512 and get 1513, so 1512 is missing */
			for (i = cb->rx_WaitFor[layer]; i < seqid; i++)
			{
				AddLate(mclcb, layer, i);
			}
			//cb->rx_WaitFor[layer] = (seqid + 1) % 65536;
			cb->rx_WaitFor[layer] = seqid + 1;
			NewTSI = tsi;
		} else {
			/* Late arrival packet (uint16 overflow) */
			/* eg. we're waiting seq 4 and we get seq 65532 */
			RemoveLate(mclcb, layer, seqid);
			NewTSI = cb->tsi;
		}
	} else {
		/* rx_WaitFor > seqid */
		Delta1 = cb->rx_WaitFor[layer] - seqid;
		Delta2 = 65535 - Delta1;
		if ( Delta1 < Delta2 ) {
			/* Late arrival packet */
			/* eg. we're waiting seq 501 and we get seq 498 */
			RemoveLate(mclcb, layer, seqid);
			NewTSI = cb->tsi;
		} else {
			/* Some packet(s) are missing (uint16 overflow) */
			/* eg. we're waiting seq 65531 and we get seq 3 */
			for (i = cb->rx_WaitFor[layer]; i != (seqid-1); i++) {
				AddLate(mclcb, layer, i);
			}
			//cb->rx_WaitFor[layer] = (seqid + 1) % 65536;
			cb->rx_WaitFor[layer] = seqid + 1;
			NewTSI = tsi;
		}
	}

	/*
	 * All kinds of problems, including late packet arrivals and
	 * duplicated packets, are signs of problems... So raise the
	 * congestion status and take appropriate measures.
	 */
	if (!cb->rx_CongestionDetected) {
		cb->rx_CongestionDetected = 1;
		if (mclcb->get_verbosity() == 2) {
			struct timeval	time;
			time = mcl_get_tvtime();
			PRINT_OUT((mcl_stdout,
				"\n%ld.%06ld\tloss_detected %d %d %d\n",
				time.tv_sec, time.tv_usec,
				layer, seqid, cb->rx_CongestionDetected))
		}
	}

	TRACELVL(5, (mcl_stdout, "<- FLIDs: CheckSequence: seq broken\n"))
	return NewTSI;
}


/**
 * A packet is late, remember there is one.
 */
static int
AddLate (mcl_cb	*mclcb,
	INT32	layer,
	INT32	nseq)
{
	ASSERT(mclcb != NULL && layer <= mclcb->nb_layers)
	/* update stats: assume a late pkt is lost (corrected later if req.) */
	mclcb->stats.rx_lost_pkts++;
	mclcb->stats.rx_lost_per_lvl[layer]++;
	TRACELVL(3, (mcl_stdout,
		"   FLIDs: pkt %d of layer %d late\n", nseq, layer))
	return MCL_OK;
}


/**
 * We finally received the delayed packet.
 */
static int
RemoveLate (mcl_cb	*mclcb,
		INT32	layer,
		INT32	nseq)
{
	ASSERT(mclcb != NULL && layer <= mclcb->nb_layers)
	/* correct stats... */
	mclcb->stats.rx_lost_pkts--;
	mclcb->stats.rx_lost_per_lvl[layer]--;
	TRACELVL(3, (mcl_stdout,
		"   FLIDs: pkt %d of layer %d finally arrived\n", nseq, layer))
	return MCL_OK;
}


static void
ComputeTSI (mcl_cb	*mclcb)
{
	flids_cb_t *cb;
	double P = 0.0;
	long B = 0;
	double BB=0.0;
	txlay_t	*tl;
	//INT32 LevelTotBw = 0;
	double LevelTotBw = 0;	// cumulative tx rate up to and including
				// this layer in Bytes per second
	INT32 i=0;

	ASSERT(mclcb!=NULL)
	cb = &(mclcb->flids_cb);

	for( i=0; i<MAX_NB_TX_LAYERS; i++) {
		cb->tx_LayerTrig[i] = 0;
	}

	B = cb->long_tsi;
	for(i=1; B!=0; i++) {
		BB += ((double)(B%2)) / pow(2.0, i);
		B >>= 1;
	}

	for( i=0; i < mclcb->nb_layers; i++) {
		tl = &(mclcb->txlay_tab[i]);
		LevelTotBw = tl->cumul_du_per_tick * mclcb->get_payload_size() *
			mcl_periodic_timer::get_frequency(); // in Bytes/sec

		if ( i+1 == mclcb->nb_layers) {
			/* Last Layer */
			P = 0.0;
		} else {
			P = min(1.0,
				(20.0 * (double)mclcb->get_payload_size () *
				 (cb->tsd/1000000)) / (double)LevelTotBw );
		}

		if(BB <= P) {
			/* Increase Signal Trigger */
			cb->tx_LayerTrig[i] = 1;
		} else {
			cb->tx_LayerTrig[i] = 0;
		}
	}

#ifdef DEBUG
	for(i = mclcb->nb_layers-1; i>0; i--) {
		if (cb->tx_LayerTrig[i] == 1) {
			ASSERT(cb->tx_LayerTrig[i-1]==1)
		}
	}
#endif /* DEBUG */
}




#endif /* } FLIDS */

