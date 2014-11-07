/* $Id: mcl_lib.cpp,v 1.10 2004/09/22 14:10:22 chneuman Exp $ */
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

/*
 *	This file contains the entry functions of the MCL library.
 */

#include "mcl_includes.h"

int mcl_ctl2 (mcl_cb *mclcb, int optname, void *optvalue, int optlen);

/****** external functions of the library *************************************/

/**
 * Open an MCL-NORM session.
 * @param  mode	defines the kind of session ("r", "w")
 * @return returns to appli a unique identifier if ok, or < 0 in case of error
 */
int
mcl_open (const char*	mode)
{
	mcl_cb		*mclcb;
	mcl_tx_rx_mode	mcl_mode;

	// check mode and translate to internal mcl mode
	if (!strcmp("w", mode)) {
		mcl_mode = MCL_IS_A_SENDER_ONLY;
	} else if (!strcmp("r", mode)) {
		mcl_mode = MCL_IS_A_RECEIVER_ONLY;
	} else {
		PRINT_ERR((mcl_stderr, "mcl_open: ERROR unknown mode %s\n", mode))
		goto bad;
	}
	mclcb = new mcl_cb(mcl_mode); // to do immediately before anything else
	if (!mclcb) {
		PRINT_ERR((mcl_stderr, "mcl_open: ERROR, mclcb constructor failed\n"))
		goto bad;
	}
	TRACELVL(5, (mcl_stdout, "<- mcl_open: \"%s\" return %d\n",
				mode, (int)mclcb->get_id()))
	return (int)(mclcb->get_id());

bad:
	PRINT_ERR((mcl_stderr, "mcl_open: ERROR\n"))
	return -1;
}


/**
 * Close the MCL session.
 * @param  id	MCL session identifier
 * @return returns 0 if ok, or < 0 in case of error
 */
int
mcl_close (int	id)
{
	mcl_cb	*mclcb;

	if (id >= MAX_NB_MCLCB || (mclcb = mclcb_tab[id]) == NULL) {
		PRINT_ERR((mcl_stderr, "mcl_close: ERROR, bad id %d\n", id))
		return -1;
	}
	mclcb->lock();
	TRACELVL(5, (mcl_stdout, "-> mcl_close: id=%d\n", id))

	if (mclcb->is_a_sender() && mclcb->fsm.can_send_data(mclcb)) {
		/*
		 * this is a sender
		 * data has already been submitted, so remember there
		 * will be no more ADUs but wait before sending a
		 * NORM_CMD_NO_NEW_OBJECTS packet (will be done at
		 * end of data tx and all pending control operations
		 * performed)
		 */
		mclcb->fsm.update_tx_state(mclcb, TEVENT_CLOSE_CALLED);
		mclcb->tx.set_no_new_adu(mclcb);
		// wait till everything has been sent...
		while (!mclcb->fsm.close_can_return(mclcb)) {
			TRACELVL(5, (mcl_stdout, "   mcl_close: (sender) wait a bit...\n"))
			mclcb->unlock();
			mcl_usleep(DFLT_POLLING_PERIOD);
			mclcb->lock();
			TRACELVL(5, (mcl_stdout, "   mcl_close: (sender) end of sleep...\n"))
		}
		//mclcb->tx_ctrl.send_close_pkt(mclcb);
		//mclcb->fsm.update_tx_state(mclcb, TEVENT_CLOSE_SENT);
		mclcb->fsm.update_tx_state(mclcb, TEVENT_CLOSE_RETURN);
	} else if (mclcb->is_a_sender()) {
		/*
		 * this is a sender
		 * data has never been submitted => send a CLOSE and return
		 */
		mclcb->finish_session_init_if_needed(); // yes, really needed!
		mclcb->fsm.update_tx_state(mclcb, TEVENT_CLOSE_CALLED);
		mclcb->tx.set_no_new_adu(mclcb);
		mclcb->tx_ctrl.send_close_pkt(mclcb);
		mclcb->fsm.update_tx_state(mclcb, TEVENT_CLOSE_SENT);
		mclcb->fsm.update_tx_state(mclcb, TEVENT_CLOSE_RETURN);
	} else {
		/*
		 * this is a receiver
		 */
		mclcb->fsm.update_rx_state(mclcb, REVENT_CLOSE_CALLED);
		while (!mclcb->fsm.close_can_return(mclcb)) {
			TRACELVL(5, (mcl_stdout, "   mcl_close: (receiver) wait a bit...\n"))
			mclcb->unlock();
			mcl_usleep(DFLT_POLLING_PERIOD);
			mclcb->lock();
		};
		mclcb->fsm.update_rx_state(mclcb, REVENT_CLOSE_RETURN);
	}
	/*
	 * de-register the session (e.g. used by the periodic_timer_thread)
	 */
	mclcb_tab[id] = NULL;
	/*
	 * destroy the rx thread now...
	 */
//#ifndef WIN32
//	if (mclcb->rx_thread.get_rx_thread_id() != 0) {
//		pthread_cancel(mclcb->rx_thread.get_rx_thread_id());
//	}
//#endif
	mclcb->unlock();	/* tx and rx threads might be blocked */
#ifndef WIN32
	if (mclcb->rx_thread.get_rx_thread_id() != 0) {
		pthread_join(mclcb->rx_thread.get_rx_thread_id(), NULL);
	}
#endif
	mclcb->free_everything();		/* free everything... */
	TRACELVL(5, (mcl_stdout, "<- mcl_close:\n"))
	delete mclcb;				/* ...including mclcb itself */
	return (close(id));
}


/*
 * close the session immediately and send CLOSE messages if we
 * are a source.
 */
int
mcl_abort (int	id)
{
	mcl_cb	*mclcb;
	int	err;		/* return code from close */
	int	trylock_err;	/* return code from trylock: 0 if ok, or EBUSY*/
	static bool in_abort = false;/* return if true (someone else in abort)*/

	if (in_abort) {
		//TRACE((mcl_stdout, "<- mcl_abort: someone else in abort\n"))
		return 0;	/* don't try to call close in abort mode */
	}
	in_abort = true;		/* take "lock" */
	if (id >= MAX_NB_MCLCB || (mclcb = mclcb_tab[id]) == NULL) {
		PRINT_ERR((mcl_stderr, "mcl_abort: ERROR, bad id %d\n", id))
		in_abort = false;	/* release "lock" */
		return 0;	/* don't try to call close in abort mode */
	}
	/*
	 * do not call lock in an async signal environment...
	 * this is a problem as mcl_abort is usually called after a CTRL-C
	 * so, use trylock instead and take lock only if available.
	 */
	trylock_err = mclcb->trylock();
	TRACELVL(5, (mcl_stdout, "-> mcl_abort: id=%d\n", id))
	if (mclcb->is_a_sender()) {
		/* send a CLOSE and return, no matter which is current state */
		mclcb->finish_session_init_if_needed(); /* yes really needed! */
		mclcb->fsm.update_tx_state(mclcb, TEVENT_ABORT);
		mclcb->tx_ctrl.send_close_pkt(mclcb);
		mclcb->fsm.update_tx_state(mclcb, TEVENT_CLOSE_SENT);
		mclcb->fsm.update_tx_state(mclcb, TEVENT_CLOSE_RETURN);
	} else {
		if (mclcb->fsm.is_opened(mclcb)) {
			mclcb->fsm.update_rx_state(mclcb, REVENT_CLOSE_CALLED);
			mclcb->fsm.update_rx_state(mclcb, REVENT_CLOSE_RETURN);
		}
		/* else already in RSTATE_NIL, do nothing */
	}
	/*
	 * de-register the session (e.g. used by the periodic_timer_thread)
	 */
	mclcb_tab[id] = NULL;
	/*
	 * destroy the rx thread now...
	 */
//#ifndef WIN32
//	if (mclcb->rx_thread.get_rx_thread_id() != 0) {
//		pthread_cancel(mclcb->rx_thread.get_rx_thread_id());
//	}
//#endif
	if (trylock_err == 0) {
		mclcb->unlock();	/* tx and rx threads might be blocked */
	}
#ifndef WIN32
	if (mclcb->rx_thread.get_rx_thread_id() != 0) {
		// XXX: sometimes blocks... TODO fix it!
		// pthread_join(mclcb->rx_thread.get_rx_thread_id(), NULL);
	}
#endif
	mclcb->free_everything();		/* free everything... */
	TRACELVL(5, (mcl_stdout, "<- mcl_abort:\n"))
	delete mclcb;			/* ...including mclcb itself */
	err = close(id);
	in_abort = false;		/* release "lock" */
	return err;
}


/**
 * Get or set control parameters for the session.
 * Follows the Unix ioctl() syscall interface.
 * Warning: usually the tx and rx contexts are still not fully initialized!
 * This is only done during the first call to mcl_send or mcl_recv...
 *
 * @param id	MCL session identifier
 * @param optname	the option to set/get/change
 * @param optvalue	the associated value set or read
 * @return returns 0 if ok, or < 0 in case of error
 */
int
mcl_ctl (int	id,
	 int	optname,
	 void	*optvalue,
	 int	optlen)
{
	mcl_cb	*mclcb;
	int	err;

	if (id >= MAX_NB_MCLCB || (mclcb = mclcb_tab[id]) == NULL) {
		PRINT_ERR((mcl_stderr, "mcl_ctl: ERROR, bad id %d\n", id))
		return -1;
	}
	mclcb->lock();
	if (!mclcb->fsm.is_opened(mclcb)) {
		PRINT_ERR((mcl_stderr, "mcl_ctl: ERROR not opened\n"))
		mclcb->unlock();
		return -1;
	}
	TRACELVL(5, (mcl_stdout, "-> mcl_ctl: id=%d\n", id))
	// call the internal ctl function where work will be done
	err = mcl_ctl2(mclcb, optname, optvalue, optlen);
	TRACELVL(5, (mcl_stdout, "<- mcl_ctl: return %d\n", err))
	mclcb->unlock();
	return err;
}


/**
 * private internal version of the MCL control function
 */
int
mcl_ctl2 (mcl_cb	*mclcb,
	  int		optname,
	  void		*optvalue,
	  int		optlen)
{
	TRACELVL(5, (mcl_stdout, "-> mcl_ctl2: optname=%d, optvalue=x%x, optlen=%d\n",
		optname, (int)optvalue, optlen))
	/* 
	 * called first (before mcl_open) to set the mcl options
	 * or call the internal application
	 */
	switch (optname) {

	case MCL_OPT_VERBOSITY:
		if (!optvalue || optlen != sizeof(int)) {
			PRINT_ERR((mcl_stderr,
			"mcl_ctl: MCL_OPT_VERBOSITY ERROR: null optvalue or bad optlen (got %d, expected %d)\n",
			optlen, sizeof(int)))
			goto error;
		}
		if (mclcb->set_verbosity(*(INT32*)optvalue) == MCL_ERROR) {
			PRINT_ERR((mcl_stderr,
			"mcl_ctl: MCL_OPT_VERBOSITY ERROR: verbose arg %d out of range\n", *(int*)optvalue))
			mclcb->set_verbosity(0); // set a conservative value
			goto error;
		}
		TRACELVL(5, (mcl_stdout, "   mcl_ctl: MCL_OPT_VERBOSITY (%d)\n", mclcb->get_verbosity()))
		break;

	case MCL_OPT_STATS: 
		if (!optvalue || optlen != sizeof(int)) {
			PRINT_ERR((mcl_stderr,
			"mcl_ctl: MCL_OPT_STATS ERROR: null optvalue or bad optlen (got %d, expected %d)\n", optlen, sizeof(int)))
			goto error;
		}
		if (mclcb->set_stats_level(*(INT32*)optvalue) == MCL_ERROR) {
			PRINT_ERR((mcl_stderr,
			"mcl_ctl: MCL_OPT_STATS ERROR: statistics arg %d out of range\n", *(int*)optvalue))
			mclcb->set_stats_level(0); // set a conservative value
			goto error;
		}
		TRACELVL(5, (mcl_stdout, "   mcl_ctl: MCL_OPT_STATS (%d)\n", mclcb->get_stats_level()))
		break;

#if 0
	case MCL_OPT_NETIF:
		if (!optvalue || optlen != sizeof(int)) {
			PRINT_ERR((mcl_stderr, "mcl_ctl: MCL_OPT_NETIF ERROR: null optvalue or bad optlen (got %d, expected %d)\n", optlen, sizeof(int)))
			goto error;
		}
		mclcb->ses_channel.mcast_if.set_addr(*(UINT32*)optvalue);
		TRACELVL(5, (mcl_stdout, "   mcl_ctl: MCL_OPT_NETIF (%d)\n", mclcb->ses_channel.mcast_if.get_addr()))
		break;
#endif
	case MCL_OPT_SET_NETIF_NAME:
		if (!optvalue || optlen <= 0 || optlen > MAX_NAME_LEN) {
			PRINT_ERR((mcl_stderr,
			"mcl_ctl: MCL_OPT_SET_NETIF_NAME ERROR: null optvalue or out of range optlen (got %d, expected ]0; %d])\n",
			optlen, MAX_NAME_LEN))
			goto error;
		}
		if ((mclcb->ses_channel.mcast_if_name = (char*)calloc(1, optlen + 1)) == NULL) {
			PRINT_ERR((mcl_stderr,
			"mcl_ctl: MCL_OPT_SET_NETIF_NAME ERROR: calloc failed.\n"))
			goto error;
		}
		strncpy(mclcb->ses_channel.mcast_if_name, (char*)optvalue, optlen);
		TRACELVL(5, (mcl_stdout, "   mcl_ctl: MCL_OPT_SET_NETIF_NAME (%s)\n", mclcb->ses_channel.mcast_if_name))
		break;

	case MCL_OPT_SET_NETIF_ADDR:
		/* specify sockaddr of the network interface to use */
		if (!optvalue) {
			PRINT_ERR((mcl_stderr,
				"mcl_ctl: MCL_OPT_SET_NETIF_ADDR ERROR: null optvalue\n"))
			goto error;
		}
		switch (optlen) {
		case (sizeof(struct sockaddr_in)): {
			struct sockaddr_in	sin;

			sin = *(struct sockaddr_in*)optvalue;
			/* assumes addr/port in network order */
	    		mclcb->ses_channel.mcast_if_addr = new mcl_addr();
			mclcb->ses_channel.mcast_if_addr->set_addr_struct(&sin);
			break;
			}
#ifdef INET6
		case (sizeof(struct sockaddr_in6)): {
			struct sockaddr_in6	sin6;

			sin6 = *(struct sockaddr_in6*)optvalue;
			/* assumes addr/port in network order */
	    		mclcb->ses_channel.mcast_if_addr = new mcl_addr();
			mclcb->ses_channel.mcast_if_addr->set_addr_struct(&sin6);
			break;
			}
#endif // INET6
		default:
			PRINT_ERR((mcl_stderr,
			"mcl_ctl: MCL_OPT_SET_NETIF_ADDR ERROR: bad optlen (got %d, expected %d or %d)\n",
				optlen, sizeof(struct sockaddr_in),
				sizeof(struct sockaddr_in6)))
			goto error;
		}
		TRACELVL(5, (mcl_stdout,
			"   mcl_ctl: MCL_OPT_SET_NETIF_ADDR (%s)\n",
			mclcb->ses_channel.mcast_if_addr->get_addr_string()))
		break;

	case MCL_OPT_PORT: {
		UINT16	p;	// port
		if (!optvalue || optlen != sizeof(int)) {
			PRINT_ERR((mcl_stderr,
			"mcl_ctl: MCL_OPT_PORT ERROR: null optvalue or bad optlen (got %d, expected %d)\n",
			optlen, sizeof(int)))
			goto error;
		}
		/* port in host byte order */
		p = *(int*)optvalue;
		mclcb->ses_channel.ses_addr.set_port(p);
		TRACELVL(5, (mcl_stdout, "   mcl_ctl: MCL_OPT_PORT (%d)\n", p))
		break;
		}

	case MCL_OPT_ADDR:
		if (!optvalue || optlen != sizeof(UINT32)) {
			PRINT_ERR((mcl_stderr,
			"mcl_ctl: MCL_OPT_ADDR ERROR: null optvalue or bad optlen (got %d, expected %d)\n",
			optlen, sizeof(UINT32)))
			goto error;
		}
		/* addr in host byte order */
		mclcb->ses_channel.ses_addr.set_addr(*(UINT32*)optvalue);
		TRACELVL(5, (mcl_stdout, "   mcl_ctl: MCL_OPT_ADDR (%s addr %s)\n", 
			(mclcb->ses_channel.is_mcast_session() ? "multicast" : "unicast"), mclcb->ses_channel.ses_addr.get_addr_string()))
		break;

#if 0
	case MCL_OPT_SRC_ADDR:
		if (!optvalue || optlen != sizeof(uint32_t)) {
			PRINT_ERR((mcl_stderr,
			"mcl_ctl: MCL_OPT_SRC_ADDR ERROR: null optvalue or bad optlen (got %d, expected %d)\n",
			optlen, sizeof(uint32_t)))
			goto error;
		}
		/* addr in host byte order */
		mclcb->src_addr = *(uint32_t*)optvalue;
		TRACELVL(5, (mcl_stdout, "   mcl_ctl: MCL_OPT_SRC_ADDR\n"))
		break;

	case MCL_OPT_BIND: {
		struct sockaddr_in	sin;
		/* specify sockaddr to use like a bind syscall */
		if (!optvalue || optlen != sizeof(struct sockaddr_in)) {
			PRINT_ERR((mcl_stderr,
			"mcl_ctl: MCL_OPT_BIND ERROR: null optvalue or bad optlen (got %d, expected %d)\n",
			optlen, sizeof(struct sockaddr_in)))
			goto error;
		}
		/* addr/port in network order => change to host order */
		sin = *(struct sockaddr_in*)optvalue;
		mclcb->addr = ntohl(sin.sin_addr.s_addr);
		mclcb->myport = ntohs(sin.sin_port);
		if (IN_MULTICAST(mclcb->addr))
			mclcb->is_multicast = 1;
		else
			mclcb->is_multicast = 0;
		TRACELVL(5, (mcl_stdout, "   mcl_ctl: MCL_OPT_BIND (%s addr)\n",
			(mclcb->is_multicast ? "multicast" : "unicast")))
		break;
		}
#endif // 0

	case MCL_OPT_BIND:
		/* specify sockaddr to use like a bind syscall */
		if (!optvalue) {
			PRINT_ERR((mcl_stderr,
				"mcl_ctl: MCL_OPT_BIND ERROR: null optvalue\n"))
			goto error;
		}
		switch (optlen) {
		case (sizeof(struct sockaddr_in)): {
			struct sockaddr_in	sin;

			sin = *(struct sockaddr_in*)optvalue;
			/* assumes addr/port in network order */
			mclcb->ses_channel.ses_addr.set_addr_struct(&sin);
			break;
			}
#ifdef INET6
		case (sizeof(struct sockaddr_in6)): {
			struct sockaddr_in6	sin6;

			sin6 = *(struct sockaddr_in6*)optvalue;
			/* assumes addr/port in network order */
			mclcb->ses_channel.ses_addr.set_addr_struct(&sin6);
			break;
			}
#endif // INET6
		default:
			PRINT_ERR((mcl_stderr,
			"mcl_ctl: MCL_OPT_BIND ERROR: bad optlen (got %d, expected %d or %d)\n",
				optlen, sizeof(struct sockaddr_in),
				sizeof(struct sockaddr_in6)))
			goto error;
		}
		TRACELVL(5, (mcl_stdout,
			"   mcl_ctl: MCL_OPT_BIND (%s IPv6 addr %s)\n",
			(mclcb->ses_channel.ses_addr.is_multicast_addr() ? "multicast"
			 				 : "unicast"),
			mclcb->ses_channel.ses_addr.get_addr_string()))
		break;

	case MCL_OPT_TTL:
		if (!optvalue || optlen != sizeof(int)) {
			PRINT_ERR((mcl_stderr,
			"mcl_ctl: MCL_OPT_TTL ERROR: null optvalue or bad optlen (got %d, expected %d)\n",
			optlen, sizeof(int)))
			goto error;
		}
		mclcb->ses_channel.ttl = *(UINT32*)optvalue;
		TRACELVL(5, (mcl_stdout, "   mcl_ctl: MCL_OPT_TTL (%d)\n",
			mclcb->ses_channel.ttl))
		if (mclcb->ses_channel.ttl > 127) {
			/* ttl 0 means do not leave host which is valid */
			PRINT_ERR((mcl_stderr,
			"mcl_ctl: MCL_OPT_TTL ERROR: TTL %d out of range ([0; %d]), reset to 1.\nNB: TTL == 0 means that packets won't leave the host\n",
			mclcb->ses_channel.ttl, 127))
			mclcb->ses_channel.ttl = 1;
			goto error;
		}
		TRACELVL(5, (mcl_stdout, "   mcl_ctl: ttl=%d\n", mclcb->ses_channel.ttl))
		break;

	case MCL_OPT_FEC_RATIO:
		if (!optvalue || optlen != sizeof(float)) {
			PRINT_ERR((mcl_stderr,
			"mcl_ctl: MCL_OPT_FEC_RATIO ERROR: null optvalue or bad optlen (got %d, expected %d)\n",
			optlen, sizeof(float)))
			goto error;
		}
		if (mclcb->fec.set_fec_ratio(mclcb, *(float*)optvalue) == MCL_ERROR) {
			PRINT_ERR((mcl_stderr,
			"mcl_ctl: MCL_OPT_FEC_RATIO failed for fec_ratio %f\n",
			*(float*)optvalue))
			goto error;
		}
		TRACELVL(5, (mcl_stdout, "   mcl_ctl: MCL_OPT_FEC_RATIO (%f)\n", *(float*)optvalue))
		break;

	case MCL_OPT_TX_PROFILE:	/* select a pre-defined tx profile */
		mcl_tx_profile	prof;
		if (!optvalue || optlen != sizeof(int)) {
			PRINT_ERR((mcl_stderr,
			"mcl_ctl: MCL_OPT_TX_PROFILE ERROR: null optvalue or bad optlen (got %d, expected %d)\n", optlen, sizeof(int)))
			goto error;
		}
		prof = (mcl_tx_profile)(*(int*)optvalue);
		if (mclcb->tx.set_tx_profile(mclcb, prof) < 0) {
			PRINT_ERR((mcl_stderr,
			"mcl_ctl: MCL_OPT_TX_PROFILE: mcl_set_tx_profile() failed for profile %d\n", (int)prof))
			goto error;
		}
		TRACELVL(5, (mcl_stdout, "   mcl_ctl: MCL_OPT_TX_PROFILE (%d)\n", (int)prof))
		break;

	case MCL_OPT_DATAGRAM_SIZE:
		if (!optvalue || optlen != sizeof(int)) {
			PRINT_ERR((mcl_stderr,
			"mcl_ctl: MCL_OPT_DATAGRAM_SIZE ERROR: null optvalue or bad optlen (got %d, expected %d)\n", optlen, sizeof(int)))
			goto error;
		}
		if (mclcb->set_max_datagram_size(*(int*)optvalue) == MCL_ERROR) {
			PRINT_ERR((mcl_stderr,
			"mcl_ctl: MCL_OPT_DATAGRAM_SIZE ERROR: size %d out of range\n",
			*(int*)optvalue))
			goto error;
		}
		TRACELVL(5, (mcl_stdout, "   mcl_ctl: MCL_OPT_DATAGRAM_SIZE (%d)\n", mclcb->get_max_datagram_size()))
		break;

	case MCL_OPT_TX_RATE: {
		int	rate;
		if (!optvalue || optlen != sizeof(int)) {
			PRINT_ERR((mcl_stderr, "mcl_ctl: MCL_OPT_TX_RATE ERROR: null optvalue or bad optlen (got %d, expected %d)\n", optlen, sizeof(int)))
			goto error;
		}
		rate = *(int*)optvalue;		/* in packets/s on base layer */
		TRACELVL(5, (mcl_stdout, "   mcl_ctl: MCL_OPT_TX_RATE (%d)\n", rate))
#define MCL_MAX_TX_RATE		4000
		if (rate <= 0 || rate > MCL_MAX_TX_RATE) {
			PRINT_ERR((mcl_stderr,
			"mcl_ctl: MCL_OPT_TX_RATE ERROR: rate %d out of range ([1; %d] pkts/s)\n",
			rate, MCL_MAX_TX_RATE))
			goto error;
		}
		mclcb->tx.set_bps_tx_rate(mclcb, rate);
		break;
		}

	case MCL_OPT_SET_FEC_CODE: /* set the FEC codec to be used */
		if (!optvalue || optlen != sizeof(int)) {
			PRINT_ERR((mcl_stderr,
			"mcl_ctl: MCL_OPT_SET_FEC_CODE ERROR: null optvalue or bad optlen (got %d, expected %d)\n",
			optlen, sizeof(int)))
			goto error;
		}
		if ((*(int*)optvalue) < MCL_FEC_SCHEME_NULL ||
		    (*(int*)optvalue) >= MCL_FEC_SCHEME_NB) {
			PRINT_ERR((mcl_stderr,
			"mcl_ctl: MCL_OPT_SET_FEC_CODE ERROR: value out or range (got %d, expected [%d; %d])\n",
			(*(int*)optvalue),
			MCL_FEC_SCHEME_NULL, MCL_FEC_SCHEME_NB - 1))
			goto error;
		}
		if (mclcb->fec.set_fec_code(mclcb, (*(int*)optvalue)) == MCL_ERROR) {
			PRINT_ERR((mcl_stderr,
			"mcl_ctl: MCL_OPT_SET_FEC_CODE WARNING: fec.set_fec_code(%d) failed\n",
			(*(int*)optvalue)))
			goto error;
		}
		TRACELVL(5, (mcl_stdout,
			"   mcl_ctl: MCL_OPT_SET_FEC_CODE (%d)\n",
			(*(int*)optvalue)))
		break;

	case MCL_OPT_GET_MAX_BLOCK_SIZE_FOR_CURRENT_FEC:
		if (!optvalue || optlen != sizeof(int)) {
			PRINT_ERR((mcl_stderr,
			"mcl_ctl: MCL_OPT_GET_MAX_BLOCK_SIZE_FOR_CURRENT_FEC ERROR: null optvalue or bad optlen (got %d, expected %d)\n",
			optlen, sizeof(UINT32)))
			goto error;
		}
		*(int*)optvalue = mclcb->fec.get_k() * mclcb->get_payload_size();
		TRACELVL(5, (mcl_stdout,
		"   mcl_ctl: MCL_OPT_GET_MAX_BLOCK_SIZE_FOR_CURRENT_FEC (%d)\n",
			(*(int*)optvalue)))
		break;

	case MCL_OPT_MOREABOUT:
		TRACELVL(5, (mcl_stdout, "   mcl_ctl: MCL_OPT_MOREABOUT\n"))
		mcl_moreabout();
		break;

	case MCL_OPT_TMP_DIR:	/* name of directory to use for temp files */
		if (!optvalue || optlen <= 0 || optlen >= MAX_FILE_NAME_LEN) {
			PRINT_ERR((mcl_stderr,
			"mcl_ctl: MCL_OPT_TMP_DIR ERROR: null optvalue or bad optlen (got %d, expected in ]0; %d[)\n",
			optlen, MAX_FILE_NAME_LEN))
			goto error;
		}
		strncpy(mcl_tmp_dir_name, (char*)optvalue, optlen);
		mcl_tmp_dir_name[MAX_FILE_NAME_LEN-1] = '\0';	/* security */
		/* update the mcl_stdout/err outputs */
		mcl_stdout_stderr_init();
		TRACELVL(5, (mcl_stdout,
		"   mcl_ctl: MCL_OPT_TMP_DIR %s\n", mcl_tmp_dir_name))
		break;

	default:
		bool opt_found = false;
#ifdef RLC
		if (rlc_ctl(mclcb, optname, optvalue, optlen) == MCL_OK) {
			opt_found = true;
		}
#endif /* RLC */		
		if (opt_found == false) {
			PRINT_ERR((mcl_stderr, "mcl_ctl: ERROR: unknown optname %d\n", optname))
			PRINT_ERR((mcl_stderr, "Check that the MCL lib has been compiled with the appropriate flags...\n"))
			goto error;
		}
		break;

	}
	TRACELVL(5, (mcl_stdout, "<- mcl_ctl2: ok\n"))
	return 0;

error:
	TRACELVL(5, (mcl_stdout, "<- mcl_ctl2: error\n"))
	return -1;
}


/**
 * mcl_send data sending function.
 * @param id	MCL session identifier
 * @param data	data block; in MCL_OPT_REUSE_APPLI_TX_BUFFER mode, the
 * 		data pointer must be the result of a standard
 * 		malloc/calloc/realloc function call.
 * @param len	data block length in bytes
 * @return	number of bytes sent or -1 if error
 */
int
mcl_send (int		id,
	  const void 	*data,
	  int		len)
{
	return mcl_sendto(id, data, len, NULL, 0);
}


/**
 * mcl_sendto data sending function.
 * @param id	MCL session identifier
 * @param data	data block; in MCL_OPT_REUSE_APPLI_TX_BUFFER mode, the
 * 		data pointer must be the result of a standard
 * 		malloc/calloc/realloc function call.
 * @param len	data block length in bytes
 * @param saddr	destination address (network byte order)
 * @param saddr_len	address length (the sockaddr struct is generic!)
 * @return	number of bytes sent or -1 if error
 */
int
mcl_sendto (int			id,
	    const void 		*data,
	    int			len,
	    const struct sockaddr *saddr,
	    int 		saddr_len)
{
	mcl_cb		*mclcb;
	mcl_adu		*adu;		/* ADU descriptor of the data buffer */
	mcl_addr	addr;
	//INT32		padded_len;	/* size with padding to 0 for FEC */
#ifdef MCL_USE_PROACTIVE_FEC
	int		i;
	mcl_block	*blk;
	INT32		fec_desired;
#endif

	if (id >= MAX_NB_MCLCB || (mclcb = mclcb_tab[id]) == NULL) {
		PRINT_ERR((mcl_stderr, "mcl_sendto: ERROR, bad id %d\n", id))
		return -1;
	}
	mclcb->lock();
	TRACELVL(5, (mcl_stdout, "-> mcl_sendto: len=%d, saddr_len=%d\n",
		len, saddr_len))
	mclcb->finish_session_init_if_needed();
	if (data == NULL && len == 0) {
		/*
		 * the appli issued this call just to finish the init...
		 * nothing else to do, return.
		 */
		TRACELVL(5, (mcl_stdout, "<- mcl_sendto: no data, return\n"))
		goto end;
	}
	if (!data || len <= 0) {
		PRINT_ERR((mcl_stderr,
			"mcl_sendto: ERROR: bad data or len argument\n"))
		goto error;
	}
	if ((saddr_len == 0 && saddr != NULL) ||
	    (saddr == NULL && saddr_len > 0)) {
		PRINT_ERR((mcl_stderr,
			"mcl_sendto: ERROR: bad address or address len argument\n"))
		goto error;
	}
	// TO DO: accept sockaddr_in6 for IPv6...
	if (saddr && saddr_len != sizeof(struct sockaddr_in)) {
		PRINT_ERR((mcl_stderr,
			"mcl_sendto: ERROR: bad addr or addr_len argument\n"))
		goto error;
	}
	mclcb->fsm.update_tx_state(mclcb, TEVENT_NEW_ADU);
	/*
	 * is there any room available in the tx window?
	 */
	while (!mclcb->tx_window.can_register_new_adu(mclcb)) {
		TRACELVL(5, (mcl_stdout, "   mcl_sendto: wait a bit for some room...\n"))
		mclcb->unlock();
		mcl_usleep(DFLT_POLLING_PERIOD);
		mclcb->lock();
	}
	/*
	 * and can we actually send data?
	 */
	if (!mclcb->fsm.can_send_data(mclcb)) {
		TRACELVL(5, (mcl_stdout, "<- mcl_sendto: ERROR, cannot send\n"))
		goto error;
	}
	/*
	 * initialize the dest addr.
	 */
	if (saddr != NULL) {
		// use the provided address
		addr.set_addr_struct((struct sockaddr_in*)saddr);
	} else {
		// ...else use the default session addr
		addr = mclcb->ses_channel.ses_addr;
	}
	/*
	 * create the adu descriptor and initialize its attributes...
	 * do not store data buffer yet...
	 */
	adu = new mcl_adu (mclcb, len, &addr);
	/*
	 * insert the adu in the tx list...
	 */
	mclcb->tx.insert_adu(mclcb, adu);
	/*
	 * store the application data...
	 */
	//padded_len = adu->get_padded_len();
	if (mclcb->tx_storage.store_adu_data (mclcb, adu, (char*) data) == MCL_ERROR) {
		PRINT_ERR((mcl_stderr,
			"mcl_sendto: ERROR: data buffer storage failed\n"))
		goto error;
	}
	/*
	 * segment the adu...
	 */
	adu->segment_for_tx(mclcb);
	/*
	 * store the adu (and all its dus) in the tx_window...
	 */
	mclcb->tx_window.register_new_adu(mclcb, adu);
#ifdef MCL_USE_PROACTIVE_FEC
	/*
	 * create additional FEC DUs for each block for proactive FEC tx...
	 * these FEC DUs will automatically be scheduled for transmission
	 * once created by the encoding thread.
	 */
	for (i = adu->get_block_nb(), blk = adu->get_block_head();
	     i > 0; i--, blk++) {
#ifdef LINUX
		fec_desired = (int)(ceilf(MCL_PROACTIVE_FEC_RATIO * (float)(blk->du_nb)));
#else		// ceilf does not exist...
		fec_desired = (int)(ceil((double)MCL_PROACTIVE_FEC_RATIO * (double)(blk->du_nb)));
#endif // OS
		if (mclcb->fec.encode(mclcb, blk, fec_desired) <= 0) {
			PRINT_ERR((mcl_stderr,
				"mcl_sendto: ERROR: FEC encoding failed\n"))
			goto error;
		}
	}
#endif
	/*
	 * update stats
	 */
	mclcb->stats.adus_announced++;
	//mclcb->stats.buf_space += adu->get_padded_len();
	mclcb->stats.buf_space += adu->get_len();	// ignore padding
	if (mclcb->stats.buf_space > mclcb->stats.max_buf_space)
		mclcb->stats.max_buf_space = mclcb->stats.buf_space;
	if (mclcb->get_stats_level() > 0)
		mclcb->stats.print_tx_stats(mclcb);

	TRACELVL(5, (mcl_stdout, "<- mcl_sendto: %d bytes sent\n", len))
end:
	mclcb->unlock();
	return len;

error:
	TRACELVL(5, (mcl_stdout, "<- mcl_sendto: ERROR, return -1\n"))
	mclcb->unlock();
	return -1;
}


/**
 * mcl_recv data sending function.
 * @param id	MCL session identifier
 * @param buf	pointer to data buffer
 * @param len	data buffer length in bytes
 * @return	number of bytes received or -1 if error
 */
int
mcl_recv (int		id,
	  void		*buf,
	  int		len)
{
	int	saddr_len = 0;
	return mcl_recvfrom(id, buf, len, (struct sockaddr*)NULL, (int*)&saddr_len);
}


/**
 * mcl_recvfrom data sending function.
 * @param id	MCL session identifier
 * @param data	pointer to data buffer
 * @param len	data buffer length in bytes
 * @param saddr	source address from which data was received (network byte order)
 * @param saddr_len	pointer to address length (sockaddr struct is generic!)
 * @return	number of bytes received or -1 if error
 */
int
mcl_recvfrom (int		id,
	      void		*buf,
	      int		len,
	      struct sockaddr	*saddr,
	      int 		*saddr_len)
{
	INT32		rlen = 0;
	struct sockaddr	tmp_saddr;
	INT32 		tmp_saddr_len = sizeof(tmp_saddr);
	mcl_cb		*mclcb;

	if (id >= MAX_NB_MCLCB || (mclcb = mclcb_tab[id]) == NULL) {
		PRINT_ERR((mcl_stderr, "mcl_recvfrom: ERROR, bad id %d\n", id))
		return -1;
	}
	mclcb->lock();
	TRACELVL(5, (mcl_stdout, "-> mcl_recvfrom: len=%d, saddr_len ptr=x%x\n",
		len, (UINT32)saddr_len))
	mclcb->finish_session_init_if_needed();
	if (buf == NULL || len == 0) {
		/*
		 * the appli issued this call just to finish the init...
		 * nothing else to do, return.
		 */
		TRACELVL(5, (mcl_stdout,
			"<- mcl_recvfrom: NULL reception buffer, return\n"))
		rlen = 0;
		goto end;
	}
	/*
	 * check if an adu is available and copy it to
	 * the application buffer
	 * XXX : use polling for the present... change it!
	 */
	while ((rlen = mclcb->rx.return_adu_to_appli(mclcb, (char*)buf,
					(INT32)len, &tmp_saddr,
					&tmp_saddr_len)) < 0) {
		if (mclcb->fsm.no_new_undup_du(mclcb)) {
			TRACELVL(5, (mcl_stdout,
				"<- mcl_recvfrom: closed, return -1\n"))
			goto error;
		}
		/*
		 * nothing received yet so wait a little bit (polling).
		 * the waiting time is a compromize... what should we use?
		 */
		mclcb->unlock();
		mcl_usleep(DFLT_POLLING_PERIOD);
		mclcb->lock();
	};

	/*
	 * copy back remote addr if appli is interested
	 */
	if (saddr) {
		ASSERT(saddr_len > 0);
		/*memcpy(saddr, &tmp_saddr, tmp_saddr_len);*/
		*saddr = tmp_saddr;
		*saddr_len = tmp_saddr_len;
	}
	TRACELVL(5, (mcl_stdout, "<- mcl_recvfrom: %d bytes recvd from %s\n",
		rlen, inet_ntoa(((struct sockaddr_in*)&tmp_saddr)->sin_addr)))
end:
	mclcb->unlock();
	return rlen;

error:
	mclcb->unlock();
	return -1;
}


