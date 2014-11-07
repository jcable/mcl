/* $Id: mcl_rlc.cpp,v 1.10 2005/01/11 13:12:32 roca Exp $ */
/*
 *  Copyright (c) 1999-2004 INRIA - All rights reserved
 *  (main author: Julien Laboure - julien.laboure@inrialpes.fr
 *                Vincent Roca - vincent.roca@inrialpes.fr)
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
 *	This file contains implementations for RLC congestion control.
 */
#include "mcl_includes.h"

#ifdef RLC

/*
 * Definition of global RLC variables
 */
UINT32 mcl_rlc_rx_period = MCL_RLC_RX_PERIOD;


/*
 * Local private function prototypes
 */
static INT32	rlc_SP_spacing	(rlccb_t *rlccb, INT32 layer);
static INT32	CheckSequence	(mcl_cb *mclcb, UINT8 layer, UINT16 seqid);
static void	RxProcessSP	(mcl_cb *mclcb, rlccb_t *rlccb, UINT8 layer);
static INT32	AddLate		(mcl_cb *mclcb, INT32 layer, INT32 nseq);
static INT32	RemoveLate	(mcl_cb *mclcb, INT32 layer, INT32 nseq);
static INT32	UpdateLateList	(mcl_cb *mclcb);
static INT32	AddLost		(mcl_cb *mclcb);
static INT32	UpdateLossList	(mcl_cb *mclcb);
static void	FreeLists	(mcl_cb *mclcb);


/****** PUBLIC FUNCTIONS ******************************************************/


/**
 * Initializes the RLC congestion control mechanism for a given session.
 * => See header file for more informations.
 */
void
rlc_init_session (mcl_cb *mclcb)
{
	INT32	i;
	rlccb_t	*rlccb;

	TRACELVL( 5, (mcl_stdout, "-> RLC:rlc_init_session: mclcb=x%x\n", (int) mclcb))
	
	rlccb = &(mclcb->rlccb);
	memset((void *)rlccb, 0, sizeof(rlccb_t));

	rlccb->rlc_sp_cycle  =	 	RLC_SP_CYCLE;
	rlccb->rlc_pkt_timeout = 	RLC_PKT_TIMEOUT;
	rlccb->rlc_deaf_period = 	RLC_DEAF_PERIOD;
	rlccb->rlc_loss_accepted = 	RLC_LOSS_ACCEPTED;
	rlccb->rlc_late_accepted = 	RLC_LATE_ACCEPTED;
	rlccb->rlc_loss_limit  = 	RLC_LOSS_LIMIT;
	rlccb->rlc_loss_timeout = 	RLC_LOSS_TIMEOUT;
	/* do it now (and later on after the FIRST packet tx) */
	/*rlc_reset_sp(mclcb);*/
	for (i = 0 ; i < MAX_NB_TX_LAYERS; i++) {
		rlccb->rx_first_pkt[i] = 1;
		/*
		 * init first_sp to 0 to start normally, and
		 * with 1 for a really slow start (i.e. the first SP of
		 * each layer will be ignored)
		 */
		/*rlccb->rx_first_sp[i] = 1;*/
		rlccb->rx_first_sp[i] = 0;
		/* used to test overflows */
		/* rlccb->tx_layers_seq[i]=65500; */
	}
	TRACELVL(5, (mcl_stdout, "<- RLC:rlc_init_session\n"))
}


/**
 * Reset all SP variables at the sending side.
 * => See header file for more informations.
 */
void
rlc_reset_tx_sp (mcl_cb *mclcb)
{
	INT32	i;
	rlccb_t	*rlccb = &(mclcb->rlccb);

	for (i = 0 ; i < MAX_NB_TX_LAYERS; i++) {
		rlccb->tx_next_sp[i] = rlc_SP_spacing(rlccb, i);
		TRACELVL(3, (mcl_stdout,
		"   RLC: rlc_reset_tx_sp: layer=%d, SP_spacing=%d, next_SP=%d\n",
		i, rlccb->tx_next_sp[i] - mcl_time_count, rlccb->tx_next_sp[i]))
	}
}


/**
 * Must be called when ending a session.
 * => See header file for more informations.
 */
void
rlc_end_session (mcl_cb *mclcb)
{
	TRACELVL(5, (mcl_stdout, "-> RLC: rlc_end_session\n"))
	FreeLists(mclcb);
	TRACELVL(5, (mcl_stdout, "<- RLC: rlc_end_session\n"))
}


/**
 * Fill the RLC header for each packet to send.
 * => See header file for more informations.
 */
INT32
rlc_tx_fill_header (mcl_cb *mclcb, rlc_hdr_t *hdr_buff, UINT8 layer)
{
	UINT16	hdr_seqid;
	rlccb_t *rlccb;

	ASSERT(mclcb);
	rlccb = &(mclcb->rlccb);
	TRACELVL(5, (mcl_stdout,
		"-> RLC: rlc_fill_header: mclcb=x%x, layer=%d\n",
		 (int) mclcb, layer))
	ASSERT(layer < MAX_NB_TX_LAYERS && layer < mclcb->nb_layers)
	hdr_seqid = rlccb->tx_layers_seq[layer];
	(rlccb->tx_layers_seq[layer])++;

	ASSERT(hdr_buff!=NULL)
	hdr_buff->rlc_reserved = 0x55;	/* unused: rlc_reserved=1010101 */
	hdr_buff->rlc_layer = layer;
	hdr_buff->rlc_seqid = htons(hdr_seqid);

	/* no SP if we are in single_layer mode */
	if(!mclcb->single_layer_mode && rlccb->tx_next_sp[layer] <= mcl_time_count) {
		/* OK this is a new SP for this layer */
		hdr_buff->rlc_sp = 1;
		/* calculate when next SP will occur */
		rlccb->tx_next_sp[layer] = rlc_SP_spacing(rlccb, layer);
		if (mclcb->get_verbosity() == 2) {
			struct timeval	time;
			time = mcl_get_tvtime();
			PRINT_OUT((mcl_stdout, "\n%ld.%06ld\tSP_all %d\n",
				time.tv_sec, time.tv_usec, layer))
		}
		TRACELVL(3, (mcl_stdout,
			"   RLC hdr info: layer=%d, seq=%d, SP=yes, next_SP=%d\n",
			 layer, hdr_seqid, rlccb->tx_next_sp[layer]))
#if 0
		if (layer+1 < MAX_NB_TX_LAYERS &&
		    mclcb->txlay_tab[layer+1].wait_sp == 1) {
			/* start tx on upper layers (if not already done) */
			/* nb: do not start before the first packet tx */
			mclcb->txlay_tab[layer+1].wait_sp = 0;
			/* nb: wait_after_sp_count already initialized */
		}
#endif
	} else {	/* not a SP... */
		hdr_buff->rlc_sp = 0;
		TRACELVL(3, (mcl_stdout, "   RLC hdr info: layer=%d, seq=%d, SP=no\n",
			 layer, hdr_seqid))
	}

	TRACELVL(5, (mcl_stdout, "<- RLC: rlc_fill_header: ok\n"))
	return MCL_OK;
}


/**
 * Analyse the packet's RLC header.
 * => See header file for more informations.
 */
INT32
rlc_rx_analyze_packet (mcl_cb *mclcb, rlc_hdr_t *hdr_buff )
{
	UINT8		hdr_layer;	
	UINT16		hdr_seqid;
	char 		hdr_sp;
	rlccb_t		*rlccb;
	
	ASSERT(hdr_buff!=NULL)
	TRACELVL(5, (mcl_stdout, "-> RLC: rlc_rx_analyze_packet\n"))
	
	ASSERT(mclcb);
	rlccb = &(mclcb->rlccb);

	hdr_seqid = ntohs(hdr_buff->rlc_seqid);
	hdr_layer = hdr_buff->rlc_layer;
	hdr_sp = hdr_buff->rlc_sp;

	if(hdr_buff->rlc_reserved != 0x55) {
		/* Unrecognized/corrupt RLC header, so ignore... */
		TRACELVL(5, (mcl_stdout, "<- RLC: rlc_rx_analyze_packet (CORRUPT_HDR)\n"))
		return ERR_CORRUPT_HDR;
	}
	if(hdr_layer >= mclcb->nb_layers) {
		/* Bad layer, should not receive on it, so ignore... */
		TRACELVL(5, (mcl_stdout, "<- RLC: rlc_rx_analyze_packet (BAD_LAYER)\n"))
		return ERR_BAD_LAYER;
	}

	UpdateLossList(mclcb);

	if ( rlccb->rx_deaf_wait )
	{
		TRACELVL(3, (mcl_stdout, "rlc_rx_analyze_packet: in DEAF!!! rlccb->rx_deaf_wait=%d\n",
			rlccb->rx_deaf_wait))
		/* Do nothing more as we're in deaf period */
		rlccb->rx_wait_for[hdr_layer]= hdr_seqid + 1;
		goto end;
	}

	if(rlccb->rx_first_pkt[hdr_layer])
	{
		/* First packet on this layer... */
		rlccb->rx_wait_for[hdr_layer] = hdr_seqid + 1;
		rlccb->rx_first_pkt[hdr_layer] = 0;
		/* continue as this pkt may contain an SP */
	} else {
		/* check if some pkts have been lost or not */
		CheckSequence(mclcb, hdr_layer, hdr_seqid);
	}

	if(hdr_sp)
	{
		if (mclcb->get_verbosity() == 2) {
			struct timeval	time;
			time = mcl_get_tvtime();
			PRINT_OUT((mcl_stdout, "\n%ld.%06ld\tSP_all %d\n",
				time.tv_sec, time.tv_usec, hdr_layer))
		}
		if(hdr_layer == mclcb->nb_layers-1)
		{
			/* This is a Synchronisation Point */
			if(rlccb->rx_first_sp[hdr_layer] == 1) {
				/* skip 1st SP, especially after deaf-period */
				rlccb->rx_first_sp[hdr_layer] = 0;
			} else {
				RxProcessSP(mclcb, rlccb, hdr_layer);
			}
		}
	}

end:
	TRACELVL(5, (mcl_stdout, "<- RLC: rlc_rx_analyze_packet (ok)\n"))
	return hdr_layer;
}


/**
 * Timer function, called periodically by the receiver.
 * => See header file for more informations.
 */
mcl_error_status
rlc_rx_timer (mcl_cb *mclcb)
{
	rlccb_t *rlccb = &(mclcb->rlccb);

	TRACELVL(5, (mcl_stdout, "-> RLC: rlc_rx_timer mclcb=x%x\n",(int)mclcb))
	if ( rlccb->rx_deaf_wait > 0 ) {
		(rlccb->rx_deaf_wait)--;
		TRACELVL(3, (mcl_stdout, "rlc_rx_timer: new rlccb->rx_deaf_wait=%d\n",
			rlccb->rx_deaf_wait))
		if (rlccb->rx_deaf_wait == 0) {
			TRACELVL(3, (mcl_stdout, "   RLC: End of Deaf Period\n"))
			/*
			 * skip next SP to be sure to wait for a complete
			 * SP period
			 */
			/*for (i=0; i<MAX_NB_TX_LAYERS; i++)*/
			/*	rlccb->rx_first_sp[i] = 1;*/
			rlccb->rx_first_sp[mclcb->nb_layers-1] = 1;

			if (mclcb->get_verbosity() == 2 ) {
				struct timeval	time;
				time = mcl_get_tvtime();
				PRINT_OUT(( stdout, "\n%ld.%06ld\tend_deaf\n",
					  time.tv_sec, time.tv_usec))
			}
		}
	} else {
		UpdateLateList(mclcb);
	}

	TRACELVL(5, (mcl_stdout, "<- RLC: rlc_rx_timer (ok)\n"))
	return MCL_OK;
}


/**
 * Used to set various RLC options and parameters.
 * => See header file for more informations.
 */
mcl_error_status
rlc_ctl (mcl_cb *mclcb, INT32 optname, void *optvalue, INT32 optlen)
{
	rlccb_t *rlccb = &(mclcb->rlccb);

	TRACELVL(5, (mcl_stdout, "-> RLC: rcl_ctl: optname=%d, optvalue=x%x, optlen=%d\n",
	optname, (int)optvalue, optlen))

	switch (optname) {
	case RLC_OPT_SP_CYCLE:
		if (!optvalue || optlen != sizeof(int)) {
			PRINT_ERR((mcl_stderr,
			"rlc_ctl ERROR RLC_OPT_SP_CYCLE: null optvalue or bad optlen (got %d, expected %d)\n", optlen, sizeof(int)))
			goto error;
		}
		if ((*(int*)optvalue) < 0 ) {
			PRINT_ERR((mcl_stderr,"rlc_ctl ERROR: RLC_OPT_SP_CYCLE must be positive (got %d)\n",(*(int*)optvalue)))
			goto error;
		}
		rlccb->rlc_sp_cycle = *(int*)optvalue;
		break;

	case RLC_OPT_PKT_TIMEOUT:
		if (!optvalue || optlen != sizeof(int)) {
			PRINT_ERR((	stderr,	"rlc_ctl ERROR RLC_OPT_PKT_TIMEOUT: null optvalue or bad optlen (got %d, expected %d)\n", optlen, sizeof(int)))
			goto error;
		}
		if ((*(int*)optvalue) < 0 ) {
			PRINT_ERR((mcl_stderr, "rlc_ctl ERROR: RLC_OPT_PKT_TIMEOUT must be positive (got %d)\n",(*(int*)optvalue)))
			goto error;
		}
		rlccb->rlc_pkt_timeout = *(int*)optvalue;
		break;

	case RLC_OPT_DEAF_PERIOD:
		if (!optvalue || optlen != sizeof(int)) {
			PRINT_ERR((	stderr,	"rlc_ctl ERROR RLC_OPT_DEAF_PERIOD: null optvalue or bad optlen (got %d, expected %d)\n", optlen, sizeof(int)))
			goto error;
		}
		if ((*(int*)optvalue) < 0 ) {
			PRINT_ERR((mcl_stderr, "rlc_ctl ERROR: RLC_OPT_DEAF_PERIOD must be positive (got %d)\n",(*(int*)optvalue)))
			goto error;
		}
		rlccb->rlc_deaf_period = *(int*)optvalue;
		break;

	case RLC_OPT_LOSS_ACCEPTED:
		if (!optvalue || optlen != sizeof(int)) {
			PRINT_ERR((mcl_stderr, "rlc_ctl ERROR RLC_OPT_LOSS_ACCEPTED: null optvalue or bad optlen (got %d, expected %d)\n", optlen, sizeof(int)))
			goto error;
		}
		if ((*(int*)optvalue) < 0 ) {
			PRINT_ERR((mcl_stderr, "rlc_ctl ERROR: RLC_OPT_LOSS_ACCEPTED must be positive (got %d)\n",(*(int*)optvalue)))
			goto error;
		}
		rlccb->rlc_loss_accepted = *(int*)optvalue;
		break;

	case RLC_OPT_LATE_ACCEPTED:
		if (!optvalue || optlen != sizeof(int)) {
			PRINT_ERR((mcl_stderr, "rlc_ctl ERROR RLC_OPT_LATE_ACCEPTED: null optvalue or bad optlen (got %d, expected %d)\n", optlen, sizeof(int)))
			goto error;
		}
		if ((*(int*)optvalue) < 0 ) {
			PRINT_ERR((mcl_stderr, "rlc_ctl ERROR: RLC_OPT_LATE_ACCEPTED must be positive (got %d)\n",(*(int*)optvalue)))
			goto error;
		}
		rlccb->rlc_late_accepted = *(int*)optvalue;
		break;

	case RLC_OPT_LOSS_LIMIT:
		if (!optvalue || optlen != sizeof(int)) {
			PRINT_ERR((mcl_stderr, "rlc_ctl ERROR RLC_OPT_LOSS_LIMIT: null optvalue or bad optlen (got %d, expected %d)\n", optlen, sizeof(int)))
			goto error;
		}
		if ((*(int*)optvalue) < 0 ) {
			PRINT_ERR((mcl_stderr, "rlc_ctl ERROR: RLC_OPT_LOSS_LIMIT must be positive (got %d)\n",(*(int*)optvalue)))
			goto error;
		}
		rlccb->rlc_loss_limit = *(int*)optvalue;
		break;

	case RLC_OPT_LOSS_TIMEOUT:
		if (!optvalue || optlen != sizeof(int)) {
			PRINT_ERR((mcl_stderr, "rlc_ctl ERROR RLC_OPT_LOSS_TIMEOUT: null optvalue or bad optlen (got %d, expected %d)\n", optlen, sizeof(int)))
			goto error;
		}
		if ((*(int*)optvalue) < 0 ) {
			PRINT_ERR((mcl_stderr, "rlc_ctl ERROR: RLC_OPT_LOSS_TIMEOUT must be positive (got %d)\n",(*(int*)optvalue)))
			goto error;
		}
		rlccb->rlc_loss_timeout = *(int*)optvalue;
		break;

	case RLC_OPT_AGGRESSIVE_CC:
		if (!optvalue || optlen != sizeof(int)) {
			PRINT_ERR((mcl_stderr, "rlc_ctl ERROR RLC_OPT_AGGRESSIVE_CC: null optvalue or bad optlen (got %d, expected %d)\n", optlen, sizeof(int)))
			goto error;
		}
		if ((*(int*)optvalue) != 0 && (*(int*)optvalue) != 1) {
			PRINT_ERR((mcl_stderr, "rlc_ctl ERROR: RLC_OPT_AGGRESSIVE_CC must be 0 or 1 (got %d)\n",(*(int*)optvalue)))
			goto error;
		}
		rlccb->rlc_lan_cc = *(int*)optvalue;
		if (rlccb->rlc_lan_cc) {
			/*
			* if we indeed have a LAN congestion control profile,
			* then adapt various parameters: SP spacing, death
			* period...
			*/
			/* nb: SP spacing will be done in rlc_SP_spacing()
			 * function */
			rlccb->rlc_deaf_period = RLC_LAN_DEAF_PERIOD;
			rlccb->rlc_late_accepted = RLC_LAN_LATE_ACCEPTED;
			rlccb->rlc_loss_accepted = RLC_LAN_LOSS_ACCEPTED;
			rlccb->rlc_loss_limit = RLC_LAN_LOSS_LIMIT;
		}
		TRACELVL(5, (mcl_stdout, "   mcl_ctl: RLC_OPT_AGGRESSIVE_CC (%d)\n", rlccb->rlc_lan_cc))
		break;

	default:
		PRINT_ERR((mcl_stderr,
			"rlc_ctl ERROR: Unknown option %d\n", optname))
		goto error;
	}

	TRACELVL(5, (mcl_stdout, "<- RLC: rlc_ctl (ok)\n"))
	return MCL_OK;

error:
	TRACELVL(5, (mcl_stdout, "<- RLC: rlc_ctl (Failure)\n"))
	return MCL_ERROR;
}


/****** PRIVATE FUNCTIONS *****************************************************/


/**
 * Calculate the SP spacing.
 * @return 		The time (in the mcl_time_count timeline) when
 *			next SP will occur.
 */
static INT32
rlc_SP_spacing (rlccb_t	*rlccb,
		INT32	layer)
{
	INT32 spacing;

	if (rlccb->rlc_lan_cc) {
		/* aggressive RLC setup for LAN only tx */
		if (layer <= 1)
			spacing = (int)(rlccb->rlc_sp_cycle / MCL_TIMER_PERIOD) * (layer + 1);
		else 
			spacing = (int)(1000000 / MCL_TIMER_PERIOD); /* 1s */
	} else {
		/* use a linear spacing of SPs on the layers */
		/*spacing = (int)(rlccb->rlc_sp_cycle / MCL_TIMER_PERIOD)*/
		/*		* (layer + 1);*/
		/* use expon. spacing of SPs on the layers with upper bound */
		spacing = (int)(rlccb->rlc_sp_cycle / MCL_TIMER_PERIOD)
				* ((1 << (layer+1))-1);
	}
	return(mcl_time_count + spacing);
}


/**
 * Used by a receiver to detect packet losses/delays and take appropriate
 * measures.
 * @param mclcb		session Control Block
 * @return		Completion status (MCL_OK or MCL_ERROR)
 */
static INT32
CheckSequence (mcl_cb *mclcb, UINT8 layer, UINT16 seqid)
{
	rlccb_t		*rlccb;
	UINT16	Delta1, Delta2 = 0;
	UINT16	i, late_limit;

	ASSERT(mclcb);
	TRACELVL(5, (mcl_stdout, "-> RLC: CheckSequence\n"))
	rlccb = &(mclcb->rlccb);
	if( rlccb->rx_wait_for[layer] == seqid )
	{	/* This is the packet we're waiting for, let's go on! */
		(rlccb->rx_wait_for[layer])++;
		TRACELVL(5, (mcl_stdout, "<- RLC: CheckSequence (good seq)\n"))
		return MCL_OK;
	}

	/* This is not the one we're waiting for... */
	if( rlccb->rx_wait_for[layer] < seqid )
	{
		Delta1 = seqid - rlccb->rx_wait_for[layer];
		Delta2 = 65535 - Delta1;
		if ( Delta1 < Delta2 ) {
			/* Some packet(s) are missing */
			/* eg. wait seq 1512 and get 1513, so 1512 is missing */
			late_limit=RLC_MAX_LATES;
			for ( i = rlccb->rx_wait_for[layer]; i < seqid; i++)
			{
				AddLate(mclcb, layer, i);
				late_limit--;
				if(late_limit <= 0 )
				{
					TRACELVL(3, (mcl_stdout,
					"   RLC Warning*** Max number of LATE packets reached\n"))
					break;
				}
			}
			rlccb->rx_wait_for[layer] = seqid +1;
		}
		else {
			/* Late arrival packet (uint16 overflow) */
			/* eg. we're waiting seq 4 and we get seq 65532 */
			RemoveLate(mclcb, layer, seqid);
		}
	}
	else /* rx_wait_for > rlc_seqid */
	{
		Delta1 = rlccb->rx_wait_for[layer] - seqid;
		Delta2 = 65535 - Delta1;
		if ( Delta1 < Delta2 )
		{/* Late arrival packet */
		/* ie: we're waiting seq 501 and we get seq 498 */
			RemoveLate(mclcb, layer, seqid);
		}
		else {
		/* Some packet(s) are missing (uint16 overflow) */
		/* ie: waiting seq 65531 and get seq 3 */
			late_limit = RLC_MAX_LATES;
			for (i = rlccb->rx_wait_for[layer]; i != (seqid-1); i++)
			{
				AddLate(mclcb, layer, i);
				late_limit--;
				if(late_limit == 0 )
				{
					TRACELVL(3, (mcl_stdout,
					"   RLC Warning*** Max number of LATE packets reached\n"))
					break;
				}
			}
			rlccb->rx_wait_for[layer] = seqid +1;
		}
	}

	TRACELVL(5, (mcl_stdout, "<- RLC: CheckSequence (seq broken)\n"))
	return MCL_ERROR;
}



/**
 *
 * @param mclcb		session Control Block
 */
static void
RxProcessSP (mcl_cb *mclcb, rlccb_t *rlccb, UINT8 layer)
{
	TRACELVL(5, (mcl_stdout, "-> RLC: RxProcessSP\n"))
	TRACELVL(3, (mcl_stdout, "   RLC: Sync Point (toplevel=%d)\n", layer))
	if (mclcb->get_verbosity() == 2) {
		struct timeval	time;
		time = mcl_get_tvtime();
		PRINT_OUT((mcl_stdout, "\n%ld.%06ld\tSP_received %d\n",
			  time.tv_sec, time.tv_usec, layer))
	}

	if (rlccb->rx_nblost_since_sp <= rlccb->rlc_loss_accepted
	    && rlccb->rx_nblate_since_sp <= rlccb->rlc_late_accepted)
	{
		mcl_add_layer(mclcb, MCL_HIGHEST_LAYER);
		/*rlccb->rx_first_sp[mclcb->nb_layers-1] = 1;*/
		TRACELVL(2, (mcl_stdout, "   RLC: add a new layer, now receiving from %d layers\n", mclcb->nb_layers))
	}
	rlccb->rx_nblost_since_sp = 0;
	rlccb->rx_nblate_since_sp = 0;

	TRACELVL(5, (mcl_stdout, "<- RLC: RxProcessSP\n"))
}


/**
 * A packet is late, remember it.
 * @param mclcb		session Control Block
 * @return		Completion status (MCL_OK or MCL_ERROR)
 */
static INT32
AddLate (mcl_cb *mclcb, INT32 layer, INT32 nseq)
{
	rlccb_t		*rlccb;
	late_list_t	*new_missing;

	TRACELVL(5, (mcl_stdout, "-> RLC: AddLate\n"))
	ASSERT(mclcb && (layer <= mclcb->nb_layers));
	rlccb = &(mclcb->rlccb);

	if( !( new_missing = (late_list_t *)malloc(sizeof(late_list_t))) ) {
		PRINT_ERR((mcl_stderr, "AddLate: no memory"))
		mcl_exit(-1);
	}
	new_missing->seq_num = nseq;
	ASSERT(mcl_rlc_rx_period!=0)
	new_missing->ttw = rlccb->rlc_pkt_timeout / mcl_rlc_rx_period;
	new_missing->next = rlccb->rx_missing[layer].next;
	rlccb->rx_missing[layer].next = new_missing;
	(rlccb->rx_nblate)++;
	(rlccb->rx_nblate_since_sp)++;
	
	if (mclcb->get_verbosity() == 2) {
		struct timeval	time;
		time = mcl_get_tvtime();
		PRINT_OUT((mcl_stdout, "\n%ld.%06ld\tlate_detected %d %d %d\n",
		time.tv_sec, time.tv_usec, layer, nseq, rlccb->rx_nblate))
	}
	/* update stats: assume a late pkt is lost (corrected later if req.) */
	mclcb->stats.rx_lost_pkts++;
	mclcb->stats.rx_lost_per_lvl[layer]++;
	TRACELVL(3, (mcl_stdout, "   RLC: pkt %d of layer %d late, total %d late\n",
		nseq, layer, rlccb->rx_nblate))
	TRACELVL(5, (mcl_stdout, "<- RLC: AddLate: ok\n"))
	return MCL_OK;
}


/**
 * We finally received the delayed packet.
 * @param mclcb		session Control Block
 * @return		Completion status (MCL_OK or MCL_ERROR)
 */
static INT32
RemoveLate (mcl_cb *mclcb, INT32 layer, INT32 nseq)
{
	rlccb_t *rlccb;
	late_list_t *prev, *current;
	TRACELVL(5, (mcl_stdout, "-> RLC: RemoveLate seq=%d on layer %d\n", nseq, layer))

	ASSERT(mclcb && (layer<=mclcb->nb_layers));
	rlccb = &(mclcb->rlccb);
	prev = &(rlccb->rx_missing[layer]);
	current = prev->next;;
	
	while ( current != NULL && current->seq_num != nseq ) {
		prev = current;
		current = current->next;
	}
	if(current == NULL) {
		TRACELVL(5, (mcl_stdout, "<- RLC: RemoveLate (Missing)\n"))
		return MCL_ERROR;
	}
	else {
		prev->next = current->next;
		free(current);
		(rlccb->rx_nblate)--;
		(rlccb->rx_nblate_since_sp)--;
	}
	/* correct stats... */
	mclcb->stats.rx_lost_pkts--;
	mclcb->stats.rx_lost_per_lvl[layer]--;
	TRACELVL(5, (mcl_stdout, "<- RLC: RemoveLate (Removed)\n"))
	return MCL_OK;
}


/**
 *
 * @param mclcb		session Control Block
 * @return		Completion status (MCL_OK or MCL_ERROR)
 */
static INT32
UpdateLateList (mcl_cb *mclcb)
{
	INT32 layer;
	rlccb_t *rlccb;
	late_list_t *prev, *current;

	TRACELVL(5, (mcl_stdout, "-> RLC: UpdateLateList\n"))
	ASSERT(mclcb);
	rlccb = &(mclcb->rlccb);
	
#ifdef DEBUG
	if( rlccb->rx_nblate > 0 ) {
		TRACELVL(3, (mcl_stdout, "   RLC: packets late:\n"))
	}
#endif

	for ( layer = 0; layer < mclcb->nb_layers; layer++)
	{
		prev = &(rlccb->rx_missing[layer]);
		current = prev->next;
		
		while ( current != NULL)
		{
			ASSERT( current->ttw !=0 )
			TRACELVL(3, (mcl_stdout, "\t layer=%d nseq=%d ttw=%d\n",
				 layer, current->seq_num, current->ttw))

			if((--(current->ttw)) == 0) {
				/* this one is lost! */
				INT32 nseq = current->seq_num;
				prev->next = current->next;
				free(current);
				(rlccb->rx_nblate)--;
				(rlccb->rx_nblate_since_sp)--;
				
				UpdateLossList(mclcb);
				if (mclcb->get_verbosity() == 2) {
					struct timeval	time;
					time = mcl_get_tvtime();
					PRINT_OUT((mcl_stdout, "\n%ld.%06ld\tloss_detected %d %d %d\n",
						time.tv_sec, time.tv_usec,
						layer, nseq, rlccb->rx_nblost+1))
				}
				TRACELVL(3, (mcl_stdout, "   RLC: pkt %d of layer %d lost! total: %d lost\n",
					 nseq, layer, rlccb->rx_nblost+1))
				if( AddLost(mclcb) == TOO_MANY_LOSSES )
					goto end;
			}
			else prev = current;
			current = prev->next;
		}
	}

end:
	TRACELVL(5, (mcl_stdout, "<- RLC: UpdateLateList (ok)\n"))
	return MCL_OK;

}


/**
 *
 * @param mclcb		session Control Block
 */
static INT32
AddLost (mcl_cb *mclcb)
{
	rlccb_t		*rlccb;
	lost_list_t	*new_lost;

	TRACELVL(5, (mcl_stdout, "-> RLC: AddLost\n"))
	ASSERT(mclcb);
	rlccb = &(mclcb->rlccb);
	rlccb->rx_nblost ++;
	rlccb->rx_nblost_since_sp ++;
	
	if ( rlccb->rx_nblost >= rlccb->rlc_loss_limit ) {
		/* too many losses... drop a layer! */
		if (mcl_drop_layer(mclcb, MCL_HIGHEST_LAYER, MCL_DO_IT ) < 0) {
			/* error, cannot drop layer */
			TRACELVL(5, (mcl_stdout, "<- RLC: AddLost: cant drop layer, ignore\n"))
			return MCL_OK;
		}
		TRACELVL(2, (mcl_stdout, "    RLC: congestion, drop a layer, now receiving from %d layers\n",
			mclcb->nb_layers))

		/* initial state for futur potential use of this layer*/
		rlccb->rx_first_pkt[mclcb->nb_layers]=1;

		/* Clean up late and lost lists... */
		FreeLists(mclcb);		
		/* entering the deaf period... */
		ASSERT(mcl_rlc_rx_period!=0)
		rlccb->rx_deaf_wait = rlccb->rlc_deaf_period/mcl_rlc_rx_period;
		TRACELVL(3, (mcl_stdout, "AddLost: LOSS, rlccb->rx_deaf_wait=%d\n",
			rlccb->rx_deaf_wait))
		TRACELVL(5, (mcl_stdout, "<- RLC: AddLost: too many losses, layer dropped\n"))
		return TOO_MANY_LOSSES;
	}

	if( !( new_lost = (lost_list_t *)malloc(sizeof(lost_list_t)))) {
		PRINT_ERR((mcl_stderr, "AddLost: no memory"))
		mcl_exit(-1);
	}
	new_lost->pkt_remaining = rlccb->rlc_loss_timeout;
	new_lost->next = rlccb->rx_lost.next;
	rlccb->rx_lost.next = new_lost;

	TRACELVL(5, (mcl_stdout, "<- RLC:AddLost (ok)\n"))
	return MCL_OK;
}


/**
 *
 * @param mclcb		session Control Block
 */
static INT32
UpdateLossList (mcl_cb *mclcb)
{
	rlccb_t *rlccb;
	lost_list_t *prev, *current;
	TRACELVL(5, (mcl_stdout, "-> RLC: UpdateLossList\n"))

	ASSERT(mclcb);
	rlccb = &(mclcb->rlccb);
	prev = &(rlccb->rx_lost);
	current = prev->next;

	while (current != NULL) {	
		if( --(current->pkt_remaining) == 0) {
			/* this loss is too old : removing */		
			prev->next = current->next;
			free(current);
			(rlccb->rx_nblost)--;
		} else
			prev = current;
		current = prev->next;
	}

	TRACELVL(5, (mcl_stdout, "<- RLC: UpdateLossList (ok)\n"))
	return MCL_OK;
}


/**
 * Frees all the lists and re-initializes the corresponding variables.
 * @param mclcb		session Control Block
 */
static void
FreeLists (mcl_cb	*mclcb)
{
	INT32 i;
	late_list_t *current_late;
	lost_list_t *current_lost;
	rlccb_t *rlccb;

	TRACELVL(5, (mcl_stdout, "-> RLC: FreeLists\n"))
	ASSERT(mclcb);
	rlccb = &(mclcb->rlccb);
	
	for (i = 0; i<MAX_NB_TX_LAYERS; i++) {
		current_late = rlccb->rx_missing[i].next;
		while (current_late != NULL) {
			rlccb->rx_missing[i].next = current_late->next;
			free(current_late);
			current_late = rlccb->rx_missing[i].next;
		}
		rlccb->rx_missing[i].next = NULL; /* redundant, isn't it? */
	}

	current_lost = rlccb->rx_lost.next;
	while ( current_lost != NULL ) {
		rlccb->rx_lost.next = current_lost->next;
		free(current_lost);
		current_lost = rlccb->rx_lost.next;
	}
	rlccb->rx_lost.next = NULL; /* redundant, isn't it? */
	rlccb->rx_nblost = 0;
	rlccb->rx_nblate = 0;
	rlccb->rx_nblost_since_sp = 0;
	rlccb->rx_nblate_since_sp = 0;
}


#endif /* RLC */
