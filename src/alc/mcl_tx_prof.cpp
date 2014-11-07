/* $Id: mcl_tx_prof.cpp,v 1.18 2005/05/18 14:37:56 roca Exp $ */
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


/* Local private function prototypes */
static void mcl_print_tx_rates (mcl_cb	*mclcb);


/*
 * if compiled with IPv6 support, then be conservative: reserve room
 * for IPv6 headers...
 */
#ifdef INET6
#define UDP_IP_HDR_SZ	UDP_IPv6_HEADER_SIZE
#else
#define UDP_IP_HDR_SZ	UDP_IPv4_HEADER_SIZE
#endif



/** transmission profile parameters structure. */
typedef struct {
	INT32	layer_nb;	/* max number of layers */
	INT32	datagram_sz;	/* datagram size; the higher, the better */
	double	tx_rate;	/* in packets per tick on the base layer */
	INT32	rlc_aggressive_cc;/* aggressive congestion ctrl setup for LAN tx*/
} tx_prof_t;

/*
 * WARNING1: keep synchronized with mcl_lib.h
 * WARNING2: changing datagram_size must be done both at source AND receivers
 *	by using the appropriate tx profile argument.
 */
static tx_prof_t	tx_prof_tab[] = {
		      /* lay	datagram_size	     tx_rate  aggr_cc*/
/* LOW_RATE_INTERNET */{10,	576 - UDP_IP_HDR_SZ,	3.0,	0},
/* MID_RATE_INTERNET */{10,	576 - UDP_IP_HDR_SZ,	20.0,	0},
/* HIGH_SPEED_INTERNET */{10,	1024 - UDP_IP_HDR_SZ,	40.0,	0},

/* HIGH_SPEED_LAN */	{1,	1400 - UDP_IP_HDR_SZ,	800.0,	0},
};

/*
 * Additional rules:
 *
 * *_INTERNET:
 * 	CC: can be either RLC or FLID-SL. If none, try FLID-SL, otherwise
 * 		RLC, otherwise error
 * 	Single layer mode: set to false since it is incompatible with CC
 *
 * HIGH_SPEED_LAN:
 * 	CC: set to NO_CC. If another one is choosen, change it to NO_CC.
 *	Single layer mode: set to true
 */


/*
 * set a pre-defined profile, modifying some MCL parameters as required
 */
int
mcl_set_tx_profile (mcl_cb	*mclcb,
		    int		profile)
{
	INT32	val;
	double	dval;	// same as val but with double format
	int	err;

	TRACELVL(5, (mcl_stdout, "-> mcl_set_tx_profile: prof=%d\n", profile))
	switch (profile) {
	case MCL_TX_PROFILE_LOW_RATE_INTERNET:
	case MCL_TX_PROFILE_MID_RATE_INTERNET:
	case MCL_TX_PROFILE_HIGH_SPEED_INTERNET:
		/* NB: call mcl_ctl2 internal func as we are already locked! */
		mclcb->single_layer_mode = false;
		/* maximum number of layers */
		val = tx_prof_tab[profile].layer_nb;
		err = mcl_ctl2(mclcb, MCL_OPT_LAYER_NB, (void*)&val,
				sizeof(val));
		if (err) goto error;
		/* datagram size */
		val = tx_prof_tab[profile].datagram_sz;
		err = mcl_ctl2(mclcb, MCL_OPT_DATAGRAM_SIZE, (void*)&val,
				sizeof(val));
		if (err) goto error;
		/* transmission rate */
		dval = tx_prof_tab[profile].tx_rate;
		err = mcl_ctl2(mclcb, MCL_OPT_TX_RATE, (void*)&dval,
				sizeof(dval));
		if (err) goto error;
		/*
		 * if a congestion control of type RLC or FLID-SL is already
		 * specified, that's ok. Otherwise choose one, by default
		 * FLID-SL, if not available RLC, otherwise return an error.
		 */
		if (mclcb->congestion_control != RLC_CC &&
		    mclcb->congestion_control != FLID_SL_CC) {
#ifdef FLIDS
			val = FLID_SL_CC; /* default */
#elif defined(RLC)
			val = RLC_CC;
#else
			ERROR, specifying a non null congestion control
			protocol is mandatory for an Internet profile.
			Add RLC or FLID-SL support in mcl_profile.h and
			recompile MCL...
#endif
			err =  mcl_ctl2(mclcb, MCL_OPT_SET_CC_SCHEME,
					(void*)&val,  sizeof(val));
			if (err) goto error;
		}
		break;

	case MCL_TX_PROFILE_HIGH_SPEED_LAN:
		/*
		 * LAN case (no multicast routing), so switch to single
		 * layer mode and remove congestion control.
		 */
		mclcb->single_layer_mode = true;
		mclcb->nb_layers = 1;	/* 1 layer only! */
		mclcb->max_nb_layers = 1;	/* 1 layer only! */
		mclcb->scheduler = MCL_SCHED_LCT1; /* mix everything */
		/* 
		 * Required for FLUTE interoperability tests, but a
		 * limitation is that no loss statistics are possible
		 */
		mclcb->congestion_control = NO_CC; /* no CC header */

		val = tx_prof_tab[profile].datagram_sz;
		err = mcl_ctl2(mclcb, MCL_OPT_DATAGRAM_SIZE, (void*)&val, sizeof(val));
		if (err) goto error;
		dval = tx_prof_tab[profile].tx_rate;
		err = mcl_ctl2(mclcb, MCL_OPT_TX_RATE, (void*)&dval, sizeof(dval));
		if (err) goto error;
		/* above tx rate specification is only valid if a single
		 * layer is specified in tx_prof_tab[], otherwise should
		 * consider the aggregated tx rate... */
		ASSERT(tx_prof_tab[profile].layer_nb == 1);
		break;

	default:
		PRINT_ERR((mcl_stderr,
		"mcl_set_tx_profile: profile %d not recognized\n", profile))
		goto error;
	}
	TRACELVL(5, (mcl_stdout, "<- mcl_set_tx_profile:\n"))
	return 0;

error:
	TRACELVL(5, (mcl_stdout, "<- mcl_set_tx_profile: error\n"))
	return -1;
}


/**
 * Print the current transmission profile.
 */
mcl_error_status
mcl_print_tx_profile (mcl_cb	*mclcb)
{
	if (!mclcb->is_a_sender()) {
		PRINT_ERR((mcl_stderr,
			"<- mcl_set_tx_profile: error, only for senders\n"))
		return MCL_ERROR;
	}
	PRINT_OUT((mcl_stdout, "TRANSMISSION PROFILE:\n"))
	PRINT_OUT((mcl_stdout, "\tCongestion control protocol: %s\n",
		(mclcb->congestion_control == NO_CC ? "NONE" :
			(mclcb->congestion_control == RLC_CC ? "RLC" :
				"FLID-SL"))))
	PRINT_OUT((mcl_stdout, "\tPacket payload size:         %d\n",
				mclcb->get_payload_size()))
	PRINT_OUT((mcl_stdout, "\tCurrent number of layers:    %d\n",
				mclcb->nb_layers))
	PRINT_OUT((mcl_stdout, "\tMaximum number of layers:    %d\n",
				mclcb->max_nb_layers))
	PRINT_OUT((mcl_stdout, "\tSingle Layer mode:           %s\n",
				(mclcb->single_layer_mode) ? "true" : "false"))
	PRINT_OUT((mcl_stdout, "\tTransmission rates (data only, not considering ALC/UDP/IPv4/v6 headers):\n"))
	mcl_print_tx_rates(mclcb);
	PRINT_OUT((mcl_stdout, "\tFEC code of last object:     %s, k=%d, n=%d\n",
				mclcb->fec.get_fec_code_string(),
				mclcb->fec.get_k(), mclcb->fec.get_n()))
	PRINT_OUT((mcl_stdout, "\tFEC expansion ratio:         %f\n",
				mclcb->fec.get_fec_ratio()))
	PRINT_OUT((mcl_stdout, "\n"))
	return MCL_OK;
}


/**
 * Prints the transmission rates for all layers.
 */
static void
mcl_print_tx_rates (mcl_cb	*mclcb)
{
	INT32		i;
	txlay_t		*tl;
	double		tps;		// ticks per second.
	double		kbits_per_du;	// kilobits per DU.

	tps = mcl_periodic_timer::get_frequency();
	kbits_per_du = (double)mclcb->get_payload_size() * 8.0 / 1000.0;
	for (i = 0, tl = mclcb->txlay_tab; i < mclcb->max_nb_layers; i++, tl++) {
		/* ticks per second (this is an int!) */
		PRINT_OUT((mcl_stdout, "Layer %d:    rate=%.2f kbps (%.2f pkts/s) \tcumul_rate=%.2f kbps\n",
			i, tl->du_per_tick * kbits_per_du * tps,
			tl->du_per_tick * tps,
			tl->cumul_du_per_tick * kbits_per_du * tps))
		PRINT_OUT((mcl_stdout, "tps=%f, kbits_per_du=%f, du_per_tick=%f\n",
			tps, kbits_per_du, tl->du_per_tick))
		if (mclcb->single_layer_mode) {
			break;
		}
	}
}


