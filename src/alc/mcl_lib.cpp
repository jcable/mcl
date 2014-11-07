/* $Id: mcl_lib.cpp,v 1.72 2005/05/23 14:55:51 roca Exp $ */
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

/*
 *	This file contains the entry functions of the MCL library.
 */

#include "mcl_includes.h"


/****** external functions of the library *************************************/

/**
 * Open an MCL_ALC session.
 * => See header file for more informations.
 */
INT32
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
 * Close an MCL_ALC session.
 * => See header file for more informations.
 */
int
mcl_close (INT32 id)
{
	int	max_adu;	/* highest ADU seq number */
	mcl_cb	*mclcb;

	if (id >= MAX_NB_MCLCB || (mclcb = mclcb_tab[id]) == NULL) {
		PRINT_ERR((mcl_stderr, "mcl_close: ERROR, bad id %d\n", id))
		return -1;
	}
	mclcb->lock();
	TRACELVL(5, (mcl_stdout, "-> mcl_close: id=%d\n", id))

	if (mclcb->is_a_sender() && mclcb->fsm.can_send_data(mclcb)) {
		/* data has already been submitted => send NO_NEW_ADU first
		 * unless we are in FLUTE mode */
		mclcb->fsm.update_tx_state(mclcb, TEVENT_CLOSE_CALLED);
		max_adu = mclcb->tx.get_highest_toi(mclcb);
		if (mclcb->is_flute_compatible() == false) {
			SendNONEWADU(mclcb, max_adu);
		}
		/* wait till everything has been sent... */
		while (!mclcb->fsm.close_can_return(mclcb)) {
			TRACELVL(5, (mcl_stdout, "   mcl_close: (sender) wait a bit...\n"))
			mclcb->unlock();
			mcl_usleep(DFLT_POLLING_PERIOD);
			mclcb->lock();
		};
		mclcb->fsm.update_tx_state(mclcb, TEVENT_CLOSE_RETURN);
	} else if (mclcb->is_a_sender()) {
		/* data has never been submitted => send a CLOSE and return */
		/* yes, we really need to finish initialization! */
#ifdef SVSOA_RECV
		mclcb->finish_session_init_if_needed(1);
#else
		mclcb->finish_session_init_if_needed();
#endif

		mclcb->fsm.update_tx_state(mclcb, TEVENT_CLOSE_CALLED);
		SendCLOSE(mclcb);
		mclcb->fsm.update_tx_state(mclcb, TEVENT_CLOSE_SENT);
		mclcb->fsm.update_tx_state(mclcb, TEVENT_CLOSE_RETURN);
	} else {
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
	 * destroy the rx thread now...
	 */
#ifdef WIN32
	if (mclcb->rx_thread) mclcb->test_cancel = TRUE;
	mclcb->unlock();		/* tx and rx threads might be blocked */
	if (mclcb->rx_thread) {
		DWORD res = WaitForSingleObject((HANDLE)mclcb->rx_thread, 5000);
		if (res == WAIT_FAILED) {
			if (GetLastError()==ERROR_INVALID_HANDLE) {
				Sleep(100);
			} else if (GetLastError()== ERROR_ACCESS_DENIED) {
			/* by Florian Geyer <F.Geyer@science-computing.de>*/
				Sleep(1);
			} else {
				PRINT_ERR((mcl_stderr,
				"mcl_close: ERROR, WaitForSingleObject() failed (%d)\n",
					GetLastError()))
				mcl_exit(-1);
			}
		}
	}
#else  // UNIX
	if (mclcb->rx_thread) pthread_cancel(mclcb->rx_thread);
	mclcb->unlock();	/* tx and rx threads might be blocked */
	if (mclcb->rx_thread) pthread_join(mclcb->rx_thread,NULL);
#endif // OS
	mclcb->rx_thread = 0;
	/* free everything */
	mclcb->free_everything();
	TRACELVL(5, (mcl_stdout, "<- mcl_close:\n"))
	delete mclcb;
	return (close(id));
}


/**
 * Close the session immediately and send CLOSE messages if we
 * are a source.
 * => See header file for more informations.
 */
int
mcl_abort (INT32	id)
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
		in_abort = 0;	/* release "lock" */
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
		/* send a CLOSE and return, no matter what is current state */
		/* yes, we really need to finish initialization! */
#ifdef SVSOA_RECV
		mclcb->finish_session_init_if_needed(1);
#else		
		mclcb->finish_session_init_if_needed();
#endif
		mclcb->fsm.update_tx_state(mclcb, TEVENT_ABORT);
		SendCLOSE(mclcb);
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
	 * de-register the session (e.g. used by the periodic timer thread)
	 */
	mclcb_tab[id] = NULL;
	/*
	 * destroy the rx thread now...
	 */
#ifdef WIN32
	if (mclcb->rx_thread) mclcb->test_cancel = TRUE;
	if (trylock_err == 0)
		mclcb->unlock();	/* tx and rx threads might be blocked */
	if (mclcb->rx_thread) {
		DWORD res = WaitForSingleObject((HANDLE)mclcb->rx_thread, 5000);
		if (res == WAIT_FAILED) {
			if (GetLastError()==ERROR_INVALID_HANDLE) {
				Sleep(100);
			} else {
				PRINT_ERR((mcl_stderr, "mcl_abort ERROR: WaitForSingleObject() failed (%d)\n", GetLastError()))
				mcl_exit(-1);
			}
		}
	}
#else  // UNIX
	if (mclcb->rx_thread) pthread_cancel(mclcb->rx_thread);
	if (trylock_err == 0)
		mclcb->unlock();	/* tx and rx threads might be blocked */
	if (mclcb->rx_thread) pthread_join(mclcb->rx_thread, NULL);
#endif // OS

	mclcb->rx_thread = 0;
	/* free everything */
	mclcb->free_everything();
	TRACELVL(5, (mcl_stdout, "<- mcl_abort:\n"))
	delete mclcb;
	err = close(id);
	in_abort = false;			/* release "lock" */
	return err;
}


/**
 * Get or set control parameters for the session.
 * WARNING: usually the tx and rx contexts are still not created!
 * This is only done during the first call to mcl_send or mcl_recv...
 * => See header file for more informations.
 */
int
mcl_ctl (INT32	id,
	 INT32	optname,
	 void	*optvalue,
	 INT32	optlen)
{
	mcl_cb	*mclcb;
	int	err;

	if (id >= MAX_NB_MCLCB || (mclcb = mclcb_tab[id]) == NULL) {
		PRINT_ERR((mcl_stderr, "mcl_ctl: ERROR, bad id %d\n", id))
		return -1;
	}
	mclcb->lock();
	if (mclcb->fsm.is_opened(mclcb) == false) {
		/* not yet opened */
		PRINT_ERR((mcl_stderr, "mcl_ctl: ERROR, not opened\n"))
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
 * Private version of the MCL control function.
 */
int
mcl_ctl2 (mcl_cb	*const mclcb,
	  INT32		optname,
	  void		*optvalue,
	  INT32		optlen)
{
	char	opt_found;

	TRACELVL(5, (mcl_stdout, "-> mcl_ctl2: optname=%d, optvalue=x%x, optlen=%d\n",
		optname, (int)optvalue, optlen))
	/* 
	 * called first (before mcl_open) to set the mcl options
	 * or call the internal application
	 */
	switch (optname) {

	case MCL_OPT_LAYER_NB:		/* set the number of tx layers */
		if (!optvalue || optlen != sizeof(int)) {
			PRINT_ERR((mcl_stderr,
			"mcl_ctl: MCL_OPT_LAYER_NB ERROR: null optvalue or bad optlen (got %d, expected %d)\n", optlen, sizeof(int)))
			goto error;
		}
		mclcb->max_nb_layers = *(int*)optvalue;
		TRACELVL(5, (mcl_stdout, "   mcl_ctl: MCL_OPT_LAYER_NB (%d)\n",  mclcb->max_nb_layers))
		if (mclcb->max_nb_layers < 1 || mclcb->max_nb_layers > MAX_NB_TX_LAYERS) {
			PRINT_ERR((mcl_stderr,
			"mcl_ctl: MCL_OPT_LAYER_NB ERROR: layer %d out of range ([1; %d])\n",
			mclcb->max_nb_layers, MAX_NB_TX_LAYERS))
			mclcb->max_nb_layers = 1;
			goto error;
		}
		break;

	case MCL_OPT_TX_PROFILE:	/* select a pre-defined tx profile */
		if (!optvalue || optlen != sizeof(int)) {
			PRINT_ERR((mcl_stderr,
			"mcl_ctl: MCL_OPT_TX_PROFILE ERROR: null optvalue or bad optlen (got %d, expected %d)\n", optlen, sizeof(int)))
			goto error;
		}
		TRACELVL(5, (mcl_stdout, "   mcl_ctl: MCL_OPT_TX_PROFILE (%d)\n", *(int*)optvalue))
		if (mcl_set_tx_profile(mclcb, *(int*)optvalue) < 0) {
			PRINT_ERR((mcl_stderr,
			"mcl_ctl: MCL_OPT_TX_PROFILE: mcl_set_tx_profile() failed\n"))
			goto error;
		}
		break;

	case MCL_OPT_PRINT_TX_PROFILE:	/* print the current tx profile */
		if (optvalue || optlen != 0) {
			PRINT_ERR((mcl_stderr,
			"mcl_ctl: MCL_OPT_PRINT_TX_PROFILE ERROR: non null optvalue or optlen > 0\n"))
			goto error;
		}
		TRACELVL(5, (mcl_stdout, "   mcl_ctl: MCL_OPT_PRINT_TX_PROFILE\n"))
		if (mcl_print_tx_profile(mclcb) != MCL_OK) {
			PRINT_ERR((mcl_stderr,
			"mcl_ctl: MCL_OPT_PRINT_TX_PROFILE: mcl_print_tx_profile() failed\n"))
			goto error;
		}
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
		double	rate;
		if (!optvalue || optlen != sizeof(double)) {
			PRINT_ERR((mcl_stderr, "mcl_ctl: MCL_OPT_TX_RATE ERROR: null optvalue or bad optlen (got %d, expected %d)\n", optlen, sizeof(double)))
			goto error;
		}
		rate = *(double*)optvalue;		/* in packets/s on base layer */
#if 0
#define MCL_MAX_TX_RATE		4000.0
		if (rate <= 0.0 || rate > MCL_MAX_TX_RATE) {
			PRINT_ERR((mcl_stderr,
			"mcl_ctl: MCL_OPT_TX_RATE ERROR: rate %f out of range ([1; %f] pkts/s)\n",
			rate, MCL_MAX_TX_RATE))
			goto error;
		}
#else
		/* no upper bound */
		if (rate <= 0.0) {
			PRINT_ERR((mcl_stderr,
			"mcl_ctl: MCL_OPT_TX_RATE ERROR: rate %f out of range (must be > 0.0)\n",
			rate))
			goto error;
		}
#endif
		mclcb->rate_on_base_layer = rate;
		TRACELVL(5, (mcl_stdout, "   mcl_ctl: MCL_OPT_TX_RATE (%f)\n", rate))
		break;
		}

	case MCL_OPT_SET_NETIF_NAME:
		if (!optvalue || optlen <= 0 || optlen > MAX_NAME_LEN) {
			PRINT_ERR((mcl_stderr,
			"mcl_ctl: MCL_OPT_SET_NETIF_NAME ERROR: null optvalue or out of range optlen (got %d, expected ]0; %d])\n",
			optlen, MAX_NAME_LEN))
			goto error;
		}
		if (optlen != (int)strlen((char*)optvalue)) {
			PRINT_ERR((mcl_stderr,
			"mcl_ctl: MCL_OPT_SET_NETIF_NAME ERROR: optlen (%d) different from string optvalue len (%d)\n",
			optlen, MAX_NAME_LEN))
			goto error;
		}
		if ((mclcb->mcast_if_name = (char*)calloc(1, optlen + 1)) == NULL) {
			PRINT_ERR((mcl_stderr,
			"mcl_ctl: MCL_OPT_SET_NETIF_NAME ERROR: calloc failed.\n"))
			goto error;
		}
		strncpy(mclcb->mcast_if_name, (char*)optvalue, optlen);
		TRACELVL(5, (mcl_stdout, "   mcl_ctl: MCL_OPT_SET_NETIF_NAME (%s)\n", mclcb->mcast_if_name))
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
	    		mclcb->mcast_if_addr = new mcl_addr();
			mclcb->mcast_if_addr->set_addr_struct(&sin);
			break;
			}
#ifdef INET6
		case (sizeof(struct sockaddr_in6)): {
			struct sockaddr_in6	sin6;

			sin6 = *(struct sockaddr_in6*)optvalue;
			/* assumes addr/port in network order */
	    		mclcb->mcast_if_addr = new mcl_addr();
			mclcb->mcast_if_addr->set_addr_struct(&sin6);
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
			mclcb->mcast_if_addr->get_addr_string()))
		break;

	case MCL_OPT_PORT: {
		int	port;
		if (!optvalue || optlen != sizeof(int)) {
			PRINT_ERR((mcl_stderr,
			"mcl_ctl: MCL_OPT_PORT ERROR: null optvalue or bad optlen (got %d, expected %d)\n",
			optlen, sizeof(int)))
			goto error;
		}
		/* port in host byte order */
		//mclcb->myport = *(int*)optvalue;
		port = *(int*)optvalue;
		mclcb->addr.set_port((u_short)port);
		TRACELVL(5, (mcl_stdout, "   mcl_ctl: MCL_OPT_PORT (%d)\n", port))
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
		mclcb->addr.set_addr(*(UINT32*)optvalue);
		TRACELVL(5, (mcl_stdout, "   mcl_ctl: MCL_OPT_ADDR (%s addr %s)\n", 
			(mclcb->addr.is_multicast_addr() ? "multicast" : "unicast"),
			mclcb->addr.get_addr_string()))
		break;

	case MCL_OPT_SET_SSM: {
		/* set the SSM (Source Specific Multicast) */
		bool	val;
		if (!optvalue || optlen != sizeof(int)) {
			PRINT_ERR((mcl_stderr,
			"mcl_ctl: MCL_OPT_SET_SSM ERROR: null optvalue or bad optlen (got %d, expected %d)\n",
			optlen, sizeof(int)))
			goto error;
		}
#ifdef SSM
		if (*(int*)optvalue == 0) {
			val = false;
		} else if (*(int*)optvalue == 1) {
			val = true;
		} else {
			PRINT_ERR((mcl_stderr,
			"mcl_ctl: MCL_OPT_SET_SSM ERROR: arg %d out of range (0/1)\n",
			*(int*)optvalue))
			goto error;
		}
		mclcb->rx.ssm = val;
		TRACELVL(5, (mcl_stdout,
		"   mcl_ctl: MCL_OPT_SET_SSM (%d)\n", val))
		break;
#else  /* SSM */
		if (*(int*)optvalue != 0) {
			PRINT_ERR((mcl_stderr, "mcl_ctl: MCL_OPT_SET_SSM ERROR: SSM (Source Specific Multicast) not supported. Recompile with the SSM define set.\n"))
			goto error;
		}
#endif /* SSM */
		}

	case MCL_OPT_SRC_ADDR:
		/* specify sockaddr to use as source address */
		if (!optvalue) {
			PRINT_ERR((mcl_stderr,
				"mcl_ctl: MCL_OPT_SRC_ADDR ERROR: null optvalue\n"))
			goto error;
		}
		switch (optlen) {
		case (sizeof(struct sockaddr_in)): {
			struct sockaddr_in	sin;

			sin = *(struct sockaddr_in*)optvalue;
			/* assumes addr/port in network order */
			mclcb->rx.src_addr.set_addr_struct(&sin);
			mclcb->rx.check_src_addr = true;
			break;
			}
#ifdef INET6
		case (sizeof(struct sockaddr_in6)): {
			struct sockaddr_in6	sin6;

			sin6 = *(struct sockaddr_in6*)optvalue;
			/* assumes addr/port in network order */
			mclcb->rx.src_addr.set_addr_struct(&sin6);
			mclcb->rx.check_src_addr = true;
			break;
			}
#endif // INET6
		default:
			PRINT_ERR((mcl_stderr,
			"mcl_ctl: MCL_OPT_SRC_ADDR ERROR: bad optlen (got %d, expected %d or %d)\n",
				optlen, sizeof(struct sockaddr_in),
				sizeof(struct sockaddr_in6)))
			goto error;
		}
		TRACELVL(5, (mcl_stdout,
			"   mcl_ctl: MCL_OPT_SRC_ADDR (%s %s addr %s)\n",
			(mclcb->rx.src_addr.is_ipv4_addr() ? "IPv4" : "IPv6"),
			(mclcb->rx.src_addr.is_multicast_addr() ? "multicast"
			 				 : "unicast"),
			mclcb->rx.src_addr.get_addr_string()))
		break;

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
			mclcb->addr.set_addr_struct(&sin);
			break;
			}
#ifdef INET6
		case (sizeof(struct sockaddr_in6)): {
			struct sockaddr_in6	sin6;

			sin6 = *(struct sockaddr_in6*)optvalue;
			/* assumes addr/port in network order */
			mclcb->addr.set_addr_struct(&sin6);
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
			"   mcl_ctl: MCL_OPT_BIND (%s %s addr %s)\n",
			(mclcb->addr.is_ipv4_addr() ? "IPv4" : "IPv6"),
			(mclcb->addr.is_multicast_addr() ? "multicast"
			 				 : "unicast"),
			mclcb->addr.get_addr_string()))
		break;

	case MCL_OPT_TTL:
		if (!optvalue || optlen != sizeof(int)) {
			PRINT_ERR((mcl_stderr,
			"mcl_ctl: MCL_OPT_TTL ERROR: null optvalue or bad optlen (got %d, expected %d)\n",
			optlen, sizeof(int)))
			goto error;
		}
		mclcb->ttl = (UINT16)(*(int*)optvalue);
		TRACELVL(5, (mcl_stdout, "   mcl_ctl: MCL_OPT_TTL (%d)\n", mclcb->ttl))
		if (mclcb->ttl > 255) {
			/* ttl 0 means do not leave host which is valid */
			PRINT_ERR((mcl_stderr,
			"mcl_ctl: MCL_OPT_TTL ERROR: TTL %d out of range ([0; %d])\nNB: TTL == 0 means do not leave host\n",
			mclcb->ttl, 127))
			mclcb->ttl = 1;
			goto error;
		}
		TRACELVL(5, (mcl_stdout, "   mcl_ctl: ttl=%d\n", mclcb->ttl))
		break;

	case MCL_OPT_DEMUX_LABEL:
		if (!optvalue || optlen != sizeof(int)) {
			PRINT_ERR((mcl_stderr,
			"mcl_ctl: MCL_OPT_DEMUX_LABEL ERROR: null optvalue or bad optlen (got %d, expected %d)\n",
			optlen, sizeof(int)))
			goto error;
		}
		mclcb->demux_label = (*(int*)optvalue);
		TRACELVL(5, (mcl_stdout, "   mcl_ctl: MCL_OPT_DEMUX_LABEL (%d)\n",
			mclcb->demux_label))
		break;

	case MCL_OPT_VERBOSITY:
		if (!optvalue || optlen != sizeof(int)) {
			PRINT_ERR((mcl_stderr,
			"mcl_ctl: MCL_OPT_VERBOSITY ERROR: null optvalue or bad optlen (got %d, expected %d)\n",
			optlen, sizeof(int)))
			goto error;
		}
		if (mclcb->set_verbosity(*(int*)optvalue) == MCL_ERROR) {
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
		if (mclcb->set_stats_level(*(int*)optvalue) == MCL_ERROR) {
			PRINT_ERR((mcl_stderr,
			"mcl_ctl: MCL_OPT_STATS ERROR: statistics arg %d out of range\n", *(int*)optvalue))
			mclcb->set_stats_level(0); // set a conservative value
			goto error;
		}
		TRACELVL(5, (mcl_stdout, "   mcl_ctl: MCL_OPT_STATS (%d)\n", mclcb->get_stats_level()))
		break;

#if 0
	case MCL_OPT_DEBUG:	/* debug mode : no tx or rx thread */
		TRACELVL(5, (mcl_stdout, "   mcl_ctl: MCL_OPT_DEBUG\n"))
		mclcb->debug = 1;
		break;
#endif

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
		TRACELVL(5, (mcl_stdout, "   mcl_ctl: MCL_OPT_TMP_DIR %s\n", mcl_tmp_dir_name))
		break;

	case MCL_OPT_SET_FLUTE_MODE:
		if (optvalue || optlen != 0) {
			PRINT_ERR((mcl_stderr,
			"mcl_ctl: MCL_OPT_SET_FLUTE_MODE ERROR: no argument needed)\n"))
			goto error;
		}
		mclcb->set_flute_mode(true);
		TRACELVL(5, (mcl_stdout,"   mcl_ctl: MCL_OPT_SET_FLUTE_MODE\n"))
		break;

	case MCL_OPT_SET_NEXT_TOI: {
		UINT32	toi;
		if (!optvalue || optlen != sizeof(UINT32)) {
			PRINT_ERR((mcl_stderr,
			"mcl_ctl: MCL_OPT_SET_NEXT_TOI ERROR: null optvalue or bad optlen (got %d, expected %d)\n",
			optlen, sizeof(UINT32)))
			goto error;
		}
		toi = (*(UINT32*)optvalue);
		if (mclcb->tx.set_next_toi(toi) == MCL_ERROR) {
			PRINT_ERR((mcl_stderr,
			"mcl_ctl: MCL_OPT_SET_NEXT_TOI ERROR: mcl_tx::set_next_toi() failed\n"))
			goto error;
		}
		TRACELVL(5, (mcl_stdout,
			"   mcl_ctl: MCL_OPT_SET_NEXT_TOI (%d)\n", toi))
		break;
		}

	case MCL_OPT_SET_NEXT_ADU_HIGH_IMPORTANCE:
			/* set the next ADU of beeing of HIGH importance */
			/* (e.g. for control data) */
		TRACELVL(5, (mcl_stdout,
			"   mcl_ctl: MCL_OPT_SET_NEXT_ADU_HIGH_IMPORTANCE\n"))
		mclcb->tx.next_adu_is_of_high_prio = true;
		break;

	case MCL_OPT_FLUSH_HIGH_IMPORTANCE_OBJECTS: {
			/* flush all high priority data */
		INT32	lay;
		txlay_t	*tl;
		
		TRACELVL(5, (mcl_stdout,
			"   mcl_ctl: MCL_OPT_FLUSH_HIGH_IMPORTANCE_OBJECTS\n"))
		for (lay = 0, tl = mclcb->txlay_tab; lay < mclcb->max_nb_layers; lay++, tl++) 
		{
			tl->tx_high_flush = true;
		}
		break;
		}

#ifdef METAOBJECT_USED
	case MCL_OPT_SET_META_OBJECT_ONLY_MODE: 
		TRACELVL(5, (mcl_stdout,
			"   mcl_ctl: MCL_OPT_SET_META_OBJECT_ONLY_MODE\n"))
		mclcb->meta_obj_layer->metaobject_only_mode = true;
		break;

	case MCL_OPT_KEEP_META_OBJECT: 
		TRACELVL(5, (mcl_stdout,
			"   mcl_ctl: MCL_OPT_KEEP_META_OBJECT\n"))
		if (!mclcb->is_a_sender() ||
		    (mclcb->meta_obj_layer->completed == false && mclcb->meta_obj_layer->in_use() == true)) {
			PRINT_ERR((mcl_stderr,
			"mcl_ctl: MCL_OPT_KEEP_META_OBJECT ERROR: not a sender or KEEP_META_OBJECT mode already set\n"))
			goto error;
		}
		mclcb->meta_obj_layer->create_meta_object();
		break;

	case MCL_OPT_PUSH_META_OBJECT: 
		TRACELVL(5, (mcl_stdout,
			"   mcl_ctl: MCL_OPT_PUSH_META_OBJECT\n"))
		if (!mclcb->is_a_sender() ||
		    !(mclcb->meta_obj_layer->completed == false && mclcb->meta_obj_layer->in_use() == true)) {
			PRINT_ERR((mcl_stderr,
			"mcl_ctl: MCL_OPT_PUSH_META_OBJECT ERROR: not a sender or KEEP_META_OBJECT mode is not set\n"))
			goto error;
		}
		//mclcb->meta_obj_layer->close_meta_object(mclcb->next_adu_seq);
		mclcb->meta_obj_layer->close_meta_object(mclcb->tx.get_next_toi());
		// mclcb->tx.incr_next_toi() // XXX: VR: is it needed ???
		break;
#endif /* METAOBJECT_USED */

	case MCL_OPT_KEEP_DATA: /* step1: wait before doing ADU scheduling */
		TRACELVL(5, (mcl_stdout, "   mcl_ctl: MCL_OPT_KEEP_DATA\n"))
		if (mclcb->tx.keep_data(mclcb) == MCL_ERROR) {
			PRINT_ERR((mcl_stderr,
			"mcl_ctl: MCL_OPT_KEEP_DATA ERROR: mcl_tx::keep_data() failed\n"))
			goto error;
		}
		break;

	case MCL_OPT_PUSH_DATA:	/* step2: now perform ADU scheduling and send */
		TRACELVL(5, (mcl_stdout, "   mcl_ctl: MCL_OPT_PUSH_DATA\n"))
		if (mclcb->tx.push_data(mclcb) == MCL_ERROR) {
			PRINT_ERR((mcl_stderr,
			"mcl_ctl: MCL_OPT_PUSH_DATA ERROR: mcl_tx::push_data() failed\n"))
			goto error;
		}
		break;
	
	case MCL_OPT_STOP_TRANSMITTING_ADU: {
		TRACELVL(5, (mcl_stdout,
			"->  mcl_ctl: MCL_OPT_STOP_TRANSMITTING_ADU\n"))

		UINT64		toi;
		INT32		lay;
		txlay_t		*tl;
		adu_t		*adu = NULL;
		
		if (mclcb->is_a_sender() == false) {
			PRINT_ERR((mcl_stderr,
			"mcl_ctl: MCL_OPT_STOP_TRANSMITTING_ADU ERROR: only possible in SENDER mode\n"))	
		}
		
		if (!optvalue || optlen != sizeof(UINT64)) {
			PRINT_ERR((mcl_stderr,
			"mcl_ctl: MCL_OPT_STOP_TRANSMITTING_ADU ERROR: null optvalue or bad optlen (got %d, expected %d)\n",
			optlen, sizeof(UINT64)))
			goto error;
		}

		if (mclcb->tx.is_in_keep_data_mode() == true){
			PRINT_ERR((mcl_stderr,
				"mcl_ctl: MCL_OPT_STOP_TRANSMITTING_ADU ERROR: not supported in KEEP_DATA mode\n"))
				goto error;	
		}

		toi = (*(UINT64*)optvalue);
		
		if (toi<=0) {
			PRINT_ERR((mcl_stderr,
			"mcl_ctl: MCL_OPT_STOP_TRANSMITTING_ADU ERROR: toi %d, not a valid value\n",
			toi))
			goto error;
		}

		if ((adu = mclcb->tx.find_adu(mclcb,toi,-1)) == NULL) {
			TRACELVL(5, (mcl_stdout,
			"<-  mcl_ctl: MCL_OPT_STOP_TRANSMITTING_ADU: ADU already removed\n"))
			break;

		}
		
		/* step O: TODO: transmit B-flag to signal end of object tx */
		
		/* first reset the transmission tables,
		 * then remove the ADU from the tx list and free the ADU,
		 * and update tx schedule */
		
		/* step 1: reset the transmission tables */			
		for (lay = 0, tl = mclcb->txlay_tab;
		     lay < mclcb->max_nb_layers; lay++, tl++) {

			/* delete the old tx tabs, and create new ones */
			if (adu->priority == ADU_NORMAL_PRIO) {
				delete tl->tx_tab;
				tl->tx_tab = new mcl_tx_tab();
			}
			else if (adu->priority == ADU_HIGH_PRIO){
				delete tl->tx_tab_high;
				tl->tx_tab_high = new mcl_tx_tab();				
			}
		}
		
		 /* step 2: remove the ADU from the tx list and free the ADU */
		mclcb->tx.remove_adu(mclcb, adu);
		mcl_tx_free_this_adu(mclcb, adu);
		free(adu);
		
		/* step 3: update tx schedule */
		if (mclcb->tx.is_in_keep_data_mode() == false) {
			UpdateTxPlanning(mclcb,mclcb->tx.get_first_adu(mclcb),mclcb->tx.get_last_adu(mclcb));
		}
#if 0
		else
		{		
			/* As it is implemented at the moment it is as if we are making a PUSH, this is not
			 * the expected behavior.*/
			/* TODO: Normally we should send everything except all ADUs after the last KEEP */
			UpdateTxPlanning(mclcb,mclcb->tx.get_first_adu(mclcb),mclcb->tx.get_last_adu(mclcb));
		
		}
#endif				


		TRACELVL(5, (mcl_stdout,
			"<-  mcl_ctl: MCL_OPT_STOP_TRANSMITTING_ADU\n"))
		
		break;
		}
		
	case MCL_OPT_RESET_TRANSMISSIONS: {
		/* Delete and free all transmission tables and their ADUs */
		TRACELVL(5, (mcl_stdout,
			"->  mcl_ctl: MCL_OPT_RESET_TRANSMISSIONS\n"))
		
		INT32	lay;
		txlay_t	*tl;

		if (mclcb->is_a_sender() == false) {
			PRINT_ERR((mcl_stderr,
			"mcl_ctl: MCL_OPT_RESET_TRANSMISSIONS ERROR: only possible in SENDER mode\n"))
			goto error;
		}

		adu_t		*new_adu = NULL;
		adu_t		*adu_list = NULL;

		
		/* Go through all layers, remove the adu from their tx_tab,
		 * and keep in mind (in the adu_list) the removed adus 
		 * (they will all be freed in the next step) */
			 
		for (lay = 0, tl = mclcb->txlay_tab;
		     lay < mclcb->max_nb_layers; lay++, tl++) {
			tl->tx_tab->remove_all_adu_from_txtab(mclcb, &adu_list);
			tl->tx_tab_high->remove_all_adu_from_txtab(mclcb, &adu_list);

			/* delete the old tx tabs, and create new ones */
			delete tl->tx_tab;
			delete tl->tx_tab_high;
			tl->tx_tab = new mcl_tx_tab();
			tl->tx_tab_high = new mcl_tx_tab();				
		}

		/* Now free the adus */
		while(adu_list != NULL)
		{
			new_adu = adu_list;
			mcl_remove_adu (mclcb, new_adu, &adu_list);
			mcl_tx_free_this_adu(mclcb, new_adu);	
			free(new_adu);	
		}

		mclcb->fsm.update_tx_state(mclcb, TEVENT_RESET);
		TRACELVL(5, (mcl_stdout,
			"<-  mcl_ctl: MCL_OPT_RESET_TRANSMISSIONS\n"))
		break;
		}

#ifdef SVSOA_RECV
	case MCL_OPT_SET_RX_LEVEL:	
		if (!optvalue || optlen != sizeof(int)) {
			PRINT_ERR((mcl_stderr,
			"mcl_ctl: MCL_OPT_SET_RX_LEVEL ERROR: null optvalue or bad optlen (got %d, expected %d)\n",
			optlen, sizeof(UINT32)))
			goto error;
		}
		if (*(int*)optvalue > MAX_NB_TX_LAYERS){
			PRINT_ERR((mcl_stderr,
			"mcl_ctl: MCL_OPT_SET_RX_LEVEL ERROR: nb of layer specified to high (got %d, max %d)\n",
			*(int*)optvalue, MAX_NB_TX_LAYERS))
			goto error;
		}
		if (mclcb->is_a_sender())  {
			PRINT_ERR((mcl_stderr,
			"mcl_ctl: MCL_OPT_SET_RX_LEVEL ERROR: only possible in RECEIVER mode\n"))
			goto error;
		}
		
		mclcb->nb_layers = *(int*)optvalue;

		TRACELVL(5, (mcl_stdout,
		"   mcl_ctl: MCL_OPT_SET_RX_LEVEL (%d)\n", *(int*)optvalue))
		break;

	case MCL_OPT_GET_RX_LEVEL:	
		if (!optvalue || optlen != sizeof(int)) {
			PRINT_ERR((mcl_stderr,
			"mcl_ctl: MCL_OPT_GET_RX_LEVEL ERROR: null optvalue or bad optlen (got %d, expected %d)\n",
			optlen, sizeof(UINT32)))
			goto error;
		}
		if (mclcb->is_a_sender())  {
			PRINT_ERR((mcl_stderr,
			"mcl_ctl: MCL_OPT_GET_RX_LEVEL ERROR: only possible in RECEIVER mode\n"))
			goto error;
		}
		*(int*)optvalue = mclcb->nb_layers;
		TRACELVL(5, (mcl_stdout,
		"   mcl_ctl: MCL_OPT_GET_RX_LEVEL (%d)\n", *(int*)optvalue))
		break;

	case MCL_USED_FOR_BASE_VIDEO_LAYER: 
		TRACELVL(5, (mcl_stdout, "   mcl_ctl: MCL_BASE_VIDEO_LAYER\n"))
#ifdef ANTICIPATED_TX_FOR_PUSH
		mclcb->anticipated_tx_for_push = 1; 
#endif			
		break;
#endif // SVSOA_RECV

	case MCL_OPT_REUSE_APPLI_TX_BUFFER: {
		bool  val;
		if (!optvalue || optlen != sizeof(int)) {
			PRINT_ERR((mcl_stderr,
			"mcl_ctl: MCL_OPT_REUSE_APPLI_TX_BUFFER ERROR: null optvalue or bad optlen (got %d, expected %d)\n",
			optlen, sizeof(int)))
			goto error;
		}
		if (*(int*)optvalue == 0) {
			val = false;
		} else if (*(int*)optvalue == 1) {
			val = true;
		} else {
			PRINT_ERR((mcl_stderr,
			"mcl_ctl: MCL_OPT_REUSE_APPLI_TX_BUFFER ERROR: arg %d out of range (0/1)\n",
			*(int*)optvalue))
			goto error;
		}
		mclcb->tx.set_reuse_appli_buf_bool(val);
		TRACELVL(5, (mcl_stdout,
		"   mcl_ctl: MCL_OPT_REUSE_APPLI_TX_BUFFER (%d)\n", val))
		break;
		}

#ifdef VIRTUAL_TX_MEM
	case MCL_OPT_VIRTUAL_TX_MEMORY: 
		if (!optvalue || optlen != sizeof(int)) {
			PRINT_ERR((mcl_stderr,
			"mcl_ctl: MCL_OPT_VIRTUAL_TX_MEMORY ERROR: null optvalue or bad optlen (got %d, expected %d)\n", optlen, sizeof(int)))
			goto error;
		}
		mclcb->vtm_used = *(int*)optvalue;
		TRACELVL(5, (mcl_stdout,
			"   mcl_ctl: MCL_OPT_VIRTUAL_TX_MEMORY (%d)\n",
			mclcb->vtm_used))
		if (mclcb->vtm_used != 0 && mclcb->vtm_used != 1) {
			PRINT_ERR((mcl_stderr,
			"mcl_ctl: MCL_OPT_VIRTUAL_TX_MEMORY ERROR: %d invalid (expected 0 or 1)\n", mclcb->vtm_used))
			mclcb->vtm_used = 0;
			goto error;
		}
		break;
#endif /* VIRTUAL_TX_MEM */

	case MCL_OPT_VIRTUAL_RX_MEMORY: 
		bool	set;
#ifdef VIRTUAL_RX_MEM
		if (!optvalue || optlen != sizeof(int)) {
			PRINT_ERR((mcl_stderr,
			"mcl_ctl: MCL_OPT_VIRTUAL_RX_MEMORY ERROR: null optvalue or bad optlen (got %d, expected %d)\n", optlen, sizeof(int)))
			goto error;
		}
		set = *(int*)optvalue;
		if (set != 0 && set != 1) {
			PRINT_ERR((mcl_stderr,
			"mcl_ctl: MCL_OPT_VIRTUAL_RX_MEMORY ERROR: %d invalid (expected 0 or 1)\n", set))
			goto error;
		}
		mclcb->rx_storage.set_vrmem(mclcb, (set ? true : false));
		TRACELVL(5, (mcl_stdout,
			"   mcl_ctl: MCL_OPT_VIRTUAL_RX_MEMORY (%d)\n",
			(INT32)mclcb->rx_storage.is_vrmem_used()))
		break;
#else  /* !VIRTUAL_RX_MEM */
		TRACELVL(5, (mcl_stdout,
			"   mcl_ctl: MCL_OPT_VIRTUAL_RX_MEMORY (%d) ignored\n"))
		break;
#endif /* VIRTUAL_RX_MEM */

	case MCL_OPT_DELIVERY_MODE:
		if (!optvalue || optlen != sizeof(int)) {
			PRINT_ERR((mcl_stderr,
			"mcl_ctl: MCL_OPT_DELIVERY_MODE ERROR: null optvalue or bad optlen (got %d, expected %d)\n",
			optlen, sizeof(int)))
			goto error;
		}
		mclcb->delivery_mode = *(int*)optvalue;
		TRACELVL(5, (mcl_stdout, "   mcl_ctl: MCL_OPT_DELIVERY_MODE (%d)\n", mclcb->delivery_mode))
		if (mclcb->delivery_mode < DEL_MODE_PUSH ||
		    mclcb->delivery_mode > DEL_MODE_STREAMING) {
			PRINT_ERR((mcl_stderr,
			"mcl_ctl: MCL_OPT_DELIVERY_MODE ERROR: delivery_mode arg %d out of range ([0; 2])\n",
			mclcb->delivery_mode))
			mclcb->delivery_mode = DEL_MODE_PUSH;
			goto error;
		}
#ifdef ANTICIPATED_TX_FOR_PUSH
		if (mclcb->delivery_mode == DEL_MODE_PUSH) {
			mclcb->anticipated_tx_for_push = 1; /* usefull here */
		} else {
			mclcb->anticipated_tx_for_push = 0; /* useless otherw */
		}
#endif /* ANTICIPATED_TX_FOR_PUSH */
		break;

	case MCL_OPT_IMMEDIATE_DELIVERY_TO_APPLI:
		if (!optvalue || optlen != sizeof(int)) {
			PRINT_ERR((mcl_stderr,
			"mcl_ctl: MCL_OPT_IMMEDIATE_DELIVERY_TO_APPLI ERROR: null optvalue or bad optlen (got %d, expected %d)\n",
			optlen, sizeof(int)))
			goto error;
		}
		if (*(int*)optvalue == 1) {	/* true */
			if (mclcb->rx_window.set_delivery_mode_to_appli(mclcb,
					MCL_IMMEDIATE_DELIVERY) == MCL_ERROR) {
				PRINT_ERR((mcl_stderr,
				"mcl_ctl: MCL_OPT_IMMEDIATE_DELIVERY_TO_APPLI ERROR: set_delivery_mode_to_appli() failed\n"))
				goto error;
			}
		} else {			/* false */
			if (mclcb->rx_window.set_delivery_mode_to_appli(mclcb,
					MCL_ORDERED_DELIVERY) == MCL_ERROR) {
				PRINT_ERR((mcl_stderr,
				"mcl_ctl: MCL_OPT_IMMEDIATE_DELIVERY_TO_APPLI ERROR: set_delivery_mode_to_appli() failed\n"))
				goto error;
			}
		}
		TRACELVL(5, (mcl_stdout,
		"   mcl_ctl: MCL_OPT_IMMEDIATE_DELIVERY_TO_APPLI (%d)\n",
			mclcb->rx_window.get_delivery_mode_to_appli(mclcb)))
		break;

	case MCL_OPT_GET_ADU_RX_INFO: {
		mcl_adu_rx_info_t	*infop;	/* in/out: info structure */
		UINT32			toi;
		adu_t			*adu;
		if (!optvalue || optlen != sizeof(mcl_adu_rx_info_t)) {
			PRINT_ERR((mcl_stderr,
			"mcl_ctl: MCL_OPT_GET_ADU_RX_INFO ERROR: null optvalue or bad optlen (got %d, expected %d)\n",
			optlen, sizeof(int)))
			goto error;
		}
		infop = (mcl_adu_rx_info_t*)optvalue;
		toi = (UINT32)infop->toi;
		adu = mclcb->rx_window.find_adu(mclcb, toi, 0);
		if (adu == NULL) {
			TRACELVL(1, (mcl_stdout,
			"mcl_ctl: MCL_OPT_GET_ADU_RX_INFO Warning: ADU (toi=%lu) not found\n", toi))
			goto error;
		} else {			/* false */
			infop->len = adu->len;
			infop->recvd_src_data = adu->recvd_src_data;
			infop->rx_status = adu->rx_status;
		}
		TRACELVL(5, (mcl_stdout,
		"   mcl_ctl: MCL_OPT_GET_ADU_RX_INFO (%lu)\n", toi))
		break;
		}
	case MCL_OPT_SET_FTI_INFO:
		hdr_infos_t	hdr_infos;
		FTI_infos_t	*fti_infop;
		if (!optvalue || optlen != sizeof(FTI_infos_t)) {
			PRINT_ERR((mcl_stderr,
			"mcl_ctl: MCL_OPT_SET_FTI_INFO ERROR: null optvalue or bad optlen (got %d, expected %d)\n",
			optlen, sizeof(int)))
			goto error;
		}
		fti_infop = (FTI_infos_t*)optvalue;
		
		memset(&hdr_infos, 0, sizeof(hdr_infos));
		hdr_infos.idf_adu = fti_infop->toi;
		hdr_infos.fec_encoding_id = fti_infop->fec_encoding_id;
		hdr_infos.fec_instance_id = fti_infop->fec_instance_id;
#ifdef LDPC_FEC
		hdr_infos.fec_key = fti_infop->fec_key;
#endif
		hdr_infos.adu_len = fti_infop->adu_len;
		hdr_infos.max_k = fti_infop->max_k;
		hdr_infos.max_n = fti_infop->max_n;
		hdr_infos.symbol_len = fti_infop->symbol_len;
		
		if (mcl_process_sig (mclcb,EXT_FTI,&hdr_infos,NULL)<0)
		 {			
		 	PRINT_ERR((mcl_stderr,
			"mcl_ctl: MCL_OPT_SET_FTI_INFO ERROR: mcl_process_sig failed\n"))
		 }
		TRACELVL(5, (mcl_stdout,
		"   mcl_ctl: MCL_OPT_SET_FTI_INFO (%lu)\n", fti_infop->toi))


		break;

	case MCL_OPT_SET_FLUTE_STORE_ALL_ADUS_BY_DEFAULT:
		if (!optvalue || optlen != sizeof(int)) {
			PRINT_ERR((mcl_stderr,
			"mcl_ctl: MCL_OPT_SET_FLUTE_STORE_ALL_ADUS_BY_DEFAULT ERROR: null optvalue or bad optlen (got %d, expected %d)\n",
			optlen, sizeof(int)))
			goto error;
		}
		if (mclcb->rx_flute.set_flute_store_all_adus_by_default(mclcb, (bool) optvalue)
				== MCL_ERROR) {
			PRINT_ERR((mcl_stderr,
			"mcl_ctl: MCL_OPT_SET_FLUTE_ALL_ADUS_DELIVERY_TO_APPLI ERROR: set_flute_delivery_mode_to_appli() failed\n"))
			goto error;
		}
		TRACELVL(5, (mcl_stdout,
		"   mcl_ctl: MCL_OPT_SET_FLUTE_ALL_ADUS_DELIVERY_TO_APPLI\n"))
		break;

#if 0
	case MCL_OPT_SET_FLUTE_SELECTED_ADUS_DELIVERY_TO_APPLI:
		if (optvalue || optlen != 0) {
			PRINT_ERR((mcl_stderr,
			"mcl_ctl: MCL_OPT_SET_FLUTE_SELECTED_ADUS_DELIVERY_TO_APPLI ERROR: no argument needed)\n"))
			goto error;
		}
		if (mclcb->rx_flute.set_flute_delivery_mode_to_appli
				(mclcb, MCL_FLUTE_SELECTED_ADUS_DELIVERY)
				== MCL_ERROR) {
			PRINT_ERR((mcl_stderr,
			"mcl_ctl: MCL_OPT_SET_FLUTE_SELECTED_ADUS_DELIVERY_TO_APPLI ERROR: set_flute_delivery_mode_to_appli() failed\n"))
			goto error;
		}
		TRACELVL(5, (mcl_stdout, "   mcl_ctl: MCL_OPT_SET_FLUTE_SELECTED_ADUS_DELIVERY_TO_APPLI set\n"))
		break;
#endif
	case MCL_OPT_FLUTE_DELIVER_THIS_ADU_TO_APPLI:
		if (!optvalue || optlen != sizeof(UINT32)) {
			PRINT_ERR((mcl_stderr,
			"mcl_ctl: MCL_OPT_DELIVER_THIS_ADU_TO_APPLI ERROR: null optvalue or bad optlen (got %d, expected %d)\n",
			optlen, sizeof(UINT32)))
			goto error;
		}
		if (mclcb->rx_flute.add_requested_toi(mclcb,*(UINT32*)optvalue)
				== MCL_ERROR) {
			PRINT_ERR((mcl_stderr,
			"mcl_ctl: MCL_OPT_DELIVER_THIS_ADU_TO_APPLI ERROR: add_requested_toi() failed\n"))
			goto error;
		}
		TRACELVL(5, (mcl_stdout,
			"   mcl_ctl: MCL_OPT_DELIVER_THIS_ADU_TO_APPLI (%d)\n",
			*(UINT32*)optvalue))
		break;	

	case MCL_OPT_POSTPONE_FEC_DECODING:
#ifdef RSE_FEC
		if (!optvalue || optlen != sizeof(int)) {
			PRINT_ERR((mcl_stderr,
			"mcl_ctl: MCL_OPT_POSTPONE_FEC_DECODING ERROR: null optvalue or bad optlen (got %d, expected %d)\n",
			optlen, sizeof(int)))
			goto error;
		}
		if (*(int*)optvalue < 0 || *(int*)optvalue > 1) {
			PRINT_ERR((mcl_stderr,
			"mcl_ctl: MCL_OPT_POSTPONE_FEC_DECODING ERROR: arg %d out of range (0/1)\n",
			*(int*)optvalue))
			goto error;
		}
		mclcb->postpone_fec_decoding = *(int*)optvalue;
		TRACELVL(5, (mcl_stdout,
			"   mcl_ctl: MCL_OPT_POSTPONE_FEC_DECODING (%d)\n",
			mclcb->postpone_fec_decoding))
#else  /* !RSE_FEC */
		/* not critical, avoid a warning if DEBUG not set */
		TRACELVL(1, (mcl_stderr,
			"mcl_ctl: MCL_OPT_POSTPONE_FEC_DECODING ERROR: only valid with RSE, doesn't apply to LDPC\n"))
		goto error;
#endif /* RSE_FEC */
		break;

	case MCL_OPT_NEVER_LEAVE_BASE_LAYER:
		if (!optvalue || optlen != sizeof(int)) {
			PRINT_ERR((mcl_stderr,
			"mcl_ctl: MCL_OPT_NEVER_LEAVE_BASE_LAYER ERROR: null optvalue or bad optlen (got %d, expected %d)\n",
			optlen, sizeof(int)))
			goto error;
		}
		if (*(int*)optvalue < 0 || *(int*)optvalue > 1) {
			PRINT_ERR((mcl_stderr,
			"mcl_ctl: MCL_OPT_NEVER_LEAVE_BASE_LAYER ERROR: arg %d out of range (0/1)\n",
			mclcb->rx.never_leave_base_layer))
			mclcb->rx.never_leave_base_layer = false;
			goto error;
		}
		mclcb->rx.never_leave_base_layer = *(bool*)optvalue;
		TRACELVL(5, (mcl_stdout,
			"   mcl_ctl: MCL_OPT_NEVER_LEAVE_BASE_LAYER (%d)\n",
			mclcb->rx.never_leave_base_layer))
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
		TRACELVL(5, (mcl_stdout, "   mcl_ctl: MCL_OPT_FEC_RATIO (%f)\n",
			mclcb->fec.get_fec_ratio()))
		break;

	case MCL_OPT_NB_OF_TX:	/* send DUs that number of times on base layer*/
		if (!optvalue || optlen != sizeof(int)) {
			PRINT_ERR((mcl_stderr,
			"mcl_ctl: MCL_OPT_NB_OF_TX ERROR: null optvalue or bad optlen (got %d, expected %d)\n", optlen, sizeof(int)))
			goto error;
		}
		mclcb->nb_tx = *(int*)optvalue;
		TRACELVL(5, (mcl_stdout, "   mcl_ctl: MCL_OPT_NB_OF_TX (%d)\n", mclcb->nb_tx))
		if (mclcb->nb_tx <= 0) {
			PRINT_ERR((mcl_stderr,
			"mcl_ctl: MCL_OPT_NB_OF_TX ERROR: %d invalid (expected > 0)\n", mclcb->nb_tx))
			mclcb->nb_tx = 1;
			goto error;
		}
		break;

	case MCL_OPT_ADD_NB_OF_TX: {	/* send DUs that number of times on base layer*/
	
		int	lay;		/* layer nb */
		txlay_t	*tl;

		
		if (!optvalue || optlen != sizeof(int)) {
			PRINT_ERR((mcl_stderr,
			"mcl_ctl: MCL_OPT_ADD_NB_OF_TX ERROR: null optvalue or bad optlen (got %d, expected %d)\n", optlen, sizeof(int)))
			goto error;
		}

		if (*(int*)optvalue <= 0) {
			PRINT_ERR((mcl_stderr,
			"mcl_ctl: MCL_OPT_ADD_NB_OF_TX ERROR: %d invalid (expected > 0)\n", mclcb->nb_tx))
			goto error;
		}

		mclcb->nb_tx += *(int*)optvalue;
		TRACELVL(5, (mcl_stdout, "   mcl_ctl: MCL_OPT_ADD_NB_OF_TX (%d)\n", *(int*)optvalue))

		for (lay = 0, tl = mclcb->txlay_tab; lay < mclcb->max_nb_layers;
			     lay++, tl++) {
			
			tl->tx_tab->add_tx_cycles(mclcb, *(int*)optvalue);	     
			tl->tx_tab_high->add_tx_cycles(mclcb, *(int*)optvalue);			     
		
		}
		
		break;
		}

	case MCL_OPT_SCHED:
		if (!optvalue || optlen != sizeof(int)) {
			PRINT_ERR((mcl_stderr,
			"mcl_ctl: MCL_OPT_SCHED ERROR: null optvalue or bad optlen (got %d, expected %d)\n",
			optlen, sizeof(int)))
			goto error;
		}
#ifdef LCT_SCHED1
		if (*(int*)optvalue == MCL_SCHED_LCT1) {
			mclcb->scheduler = MCL_SCHED_LCT1;
			TRACELVL(5, (mcl_stdout, "   mcl_ctl: MCL_OPT_SCHED (sched1)\n"))
			break;
		}
#endif
#ifdef LCT_SCHEDSTREAM
		if (*(int*)optvalue == MCL_SCHED_LCTSTREAM) {
			mclcb->scheduler = MCL_SCHED_LCTSTREAM;
			TRACELVL(5, (mcl_stdout, "   mcl_ctl: MCL_OPT_SCHED (sched_stream)\n"))
			break;
		}
#endif
		PRINT_ERR((mcl_stderr,
			"mcl_ctl: MCL_OPT_SCHED ERROR: arg %d unsupported\n",
			*(int*)optvalue))
		goto error;

	case MCL_OPT_OBJ_SCHED:
		if (!optvalue || optlen != sizeof(int)) {
			PRINT_ERR((mcl_stderr,
			"mcl_ctl: MCL_OPT_OBJ_SCHED ERROR: null optvalue or bad optlen (got %d, expected %d)\n",
			optlen, sizeof(int)))
			goto error;
		}
		mclcb->adu_scheduling = *(int*)optvalue;
		TRACELVL(5, (mcl_stdout, "   mcl_ctl: MCL_OPT_OBJ_SCHED: %d\n",
			mclcb->adu_scheduling))
		if (mclcb->adu_scheduling < 0 ||
		    mclcb->adu_scheduling > MCL_SCHED_MIXED_ORDER) {
			TRACELVL(5, (mcl_stdout, "   mcl_ctl: MCL_OPT_SCHED ERROR: bad value, must be in range {0..%d}\n", MCL_SCHED_MIXED_ORDER))
			mclcb->adu_scheduling = 0;
			goto error;
		}
		break;

	case MCL_OPT_SET_CC_SCHEME: /* set the CC scheme to be used */
		if (!optvalue || optlen != sizeof(int)) {
			PRINT_ERR((mcl_stderr,
			"mcl_ctl: MCL_OPT_SET_CC_SCHEME ERROR: null optvalue or bad optlen (got %d, expected %d)\n",
			optlen, sizeof(int)))
			goto error;
		}

		mclcb->congestion_control = (* (mcl_congestion_control_scheme*)optvalue);

		if (mclcb->congestion_control == NO_CC)
		{
			mclcb->nb_layers = 1;	/* 1 layer only! */
			mclcb->max_nb_layers = 1;	/* 1 layer only! */
			mclcb->scheduler = MCL_SCHED_LCT1; /* mix everything */
			mclcb->single_layer_mode = true;
		}
#ifdef RLC
		if (mclcb->congestion_control == RLC_CC)
		{
			mclcb->scheduler = MCL_SCHED_LCT1; /* mix everything */
			mclcb->single_layer_mode = false;
			rlc_init_session(mclcb);

		}
#endif
#ifdef FLIDS
		if (mclcb->congestion_control == FLID_SL_CC)
		{
			mclcb->scheduler = MCL_SCHED_LCT1; /* mix everything */
			mclcb->single_layer_mode = false;
			FLIDs_InitSession(mclcb);
		}
#endif

		TRACELVL(5, (mcl_stdout, "   mcl_ctl: MCL_OPT_SET_CC_SCHEME: %d\n", *(int*)optvalue))

		break;

	case MCL_OPT_SET_FEC_CODE: /* set the FEC codec to be used */
		if (!optvalue || optlen != sizeof(int)) {
			PRINT_ERR((mcl_stderr,
			"mcl_ctl: MCL_OPT_SET_FEC_CODE ERROR: null optvalue or bad optlen (got %d, expected %d)\n",
			optlen, sizeof(int)))
			goto error;
		}
		if (mclcb->fec.set_fec_code(mclcb, (*(int*)optvalue)) == MCL_ERROR) {
			PRINT_ERR((mcl_stderr,
			"mcl_ctl: MCL_OPT_SET_FEC_CODE WARNING: fec.set_fec_code(%d) failed\n",
			(*(int*)optvalue)))
			goto error;
		}
		TRACELVL(5, (mcl_stdout,
		"   mcl_ctl: MCL_OPT_SET_FEC_CODE (%d)\n", (*(int*)optvalue)))
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

	default:
		opt_found = 0;
#ifdef RLC
		if (mclcb->congestion_control == RLC_CC)
		{
			if (rlc_ctl( mclcb, optname, optvalue, optlen) == MCL_OK) {
				opt_found = 1;
			}
		}
#endif
#ifdef FLIDS
		if (mclcb->congestion_control == FLID_SL_CC)
		{
			if (FLIDs_ctl( mclcb, optname, optvalue, optlen) == MCL_OK) {
				opt_found = 1;
			} 
		}
#endif
		if (!opt_found) {
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
 * mcl_send.
 * => See header file for more informations.
 */
INT32
mcl_send (INT32		id,
	  const void	*data,
	  INT32		len)
{
	INT32			result;
	mcl_cb			*mclcb;
	struct mcl_iovec	iov;
	struct mcl_msghdr	msg;

	if (id >= MAX_NB_MCLCB || (mclcb = mclcb_tab[id]) == NULL) {
		PRINT_ERR((mcl_stderr, "mcl_send: ERROR, bad id %d\n", id))
		return -1;
	}
	mclcb->lock();
	/*
	 * create a corresponding msg descriptor.
	 */
	memset((void*)&iov, 0, sizeof(iov));
	memset((void*)&msg, 0, sizeof(msg));
	msg.msg_iov = &iov;
	msg.msg_iovlen = 1;
	iov.iov_type = MCL_IOV_TYPE_BUFFER;
	iov.iov_base = (void*)data;
	iov.iov_len = len;
	result = (INT32)mcl_sendmsg2(mclcb, &msg, MCL_MSG_DEFAULT);
	mclcb->unlock();
	return result;
}


/**
 * mcl_sendto.
 * => See header file for more informations.
 */
INT32
mcl_sendto (INT32		id,
	    const void		*data,
	    INT32		len,
	    const struct sockaddr *saddr,
	    INT32 		saddr_len)
{
	INT32 			result;
	mcl_cb			*mclcb;
	struct mcl_iovec	iov;
	struct mcl_msghdr	msg;

	if (id >= MAX_NB_MCLCB || (mclcb = mclcb_tab[id]) == NULL) {
		PRINT_ERR((mcl_stderr, "mcl_sendto: ERROR, bad id %d\n", id))
		return -1;
	}
	mclcb->lock();
	/*
	 * create a corresponding msg descriptor.
	 */
	memset((void*)&iov, 0, sizeof(iov));
	memset((void*)&msg, 0, sizeof(msg));
	if (saddr) {
		msg.msg_name = (void*)saddr;
		msg.msg_namelen = saddr_len;	/* will be checked later */
	} else {
		msg.msg_name = NULL;
		msg.msg_namelen = 0;
	}
	msg.msg_iov = &iov;
	msg.msg_iovlen = 1;
	iov.iov_type = MCL_IOV_TYPE_BUFFER;
	iov.iov_base = (void*)data;
	iov.iov_len = len;
	result = (INT32)mcl_sendmsg2(mclcb, &msg, MCL_MSG_DEFAULT);
	mclcb->unlock();
	return result;
}

#if 0

/**
 * mcl_sendto2.
 * => See header file for more informations.
 */
int
mcl_sendto2 (mcl_cb		*const mclcb,
	    const void		*data,
	    int			len,
	    const struct sockaddr *saddr,
	    int 		saddr_len)
{
	INT32 			result;
	struct mcl_iovec	iov;
	struct mcl_msghdr	msg;

	/*
	 * create a corresponding msg descriptor.
	 */
	memset((void*)&iov, 0, sizeof(iov));
	memset((void*)&msg, 0, sizeof(msg));
	if (saddr) {
		msg.msg_name = (void*)saddr;
		msg.msg_namelen = saddr_len;	/* will be checked later */
	} else {
		msg.msg_name = NULL;
		msg.msg_namelen = 0;
	}
	msg.msg_iov = &iov;
	msg.msg_iovlen = 1;
	iov.iov_type = MCL_IOV_TYPE_BUFFER;
	iov.iov_base = (void*)data;
	iov.iov_len = len;
	result = (INT32)mcl_sendmsg2(mclcb, &msg, MCL_MSG_DEFAULT);
	return result;

#if 0
	adu_t		*adu;		/* DU descriptor of the data buffer */
#ifdef FEC
	block_t		*blk;
	int		i;
#endif /* FEC */
	UINT32		padded_len;	/* size with padding to 0 for FEC */
	int		tot_fec_nb = 0;	/* fec DUs created by mcl_fec_encode */
#ifdef ANTICIPATED_TX_FOR_PUSH
	int		try_2_send_count = 4;	/* period between 2 attempts */
#endif

	TRACELVL(5, (mcl_stdout, "-> mcl_sendto: len=%d, saddr_len=%d\n",
		len, saddr_len))
#ifdef SVSOA_RECV
	mclcb->finish_session_init_if_needed(1);
#else
	mclcb->finish_session_init_if_needed();
#endif
	if (data == NULL && len == 0) {
		/*
		 * the appli issued this call just to finish the init...
		 * nothing else to do, return.
		 */
		TRACELVL(5, (mcl_stdout, "<- mcl_sendto: no data, return\n"))
		goto end;
	}

#ifdef METAOBJECT_USED	
	/* check if we are sending a meta object */
	if (mclcb->meta_obj_layer->in_use() == true &&
	    mclcb->meta_obj_layer->completed == false) {
		if (mclcb->meta_obj_layer->add_object((char *)data, len) >= 0) {
			goto end;
		} else {
			goto error;
		}
	}
#endif

	mclcb->fsm.update_tx_state(mclcb, TEVENT_NEW_ADU);
	if (mclcb->fsm.can_send_data(mclcb) == false) {
		TRACELVL(5, (mcl_stdout, "<- mcl_sendto: ERROR, cannot send\n"))
		goto error;
	}
	if (!data || len <= 0) {
		PRINT_ERR((mcl_stderr,
			"mcl_sendto: ERROR: data arg NULL or len arg <= 0\n"))
		goto error;
	}
	if (saddr && (saddr_len != sizeof(struct sockaddr_in)
		      saddr_len != sizeof(struct sockaddr_in6))) {
		PRINT_ERR((mcl_stderr,
			"mcl_sendto: ERROR: saddr != NULL and bad saddr_len arg (only %d/%d possible, got %d)\n",
			sizeof(struct sockaddr_in),
			sizeof(struct sockaddr_in6), saddr_len))
		goto error;
	}

	/*
	 * Create the ADU descriptor...
	 */
	adu = mcl_create_adu(mclcb);
	adu->fec_scheme = mclcb->fec.get_fec_code();
	if (mclcb->fec.scheme_to_enc_inst(mclcb, adu->fec_scheme,
					&(adu->fec_encoding_id),
					&(adu->fec_instance_id)) == MCL_ERROR) {
		PRINT_ERR((mcl_stderr,
			"mcl_sendto: ERROR: unsupported FEC scheme %d\n",
			adu->fec_scheme))
		goto error;
	}
	adu->len = len;
	if (saddr_len > 0) {
		/* remember the dest addr to use (rather than default dest) */
		if (saddr_len == sizeof(struct sockaddr_in)) {
			adu->addr.set_addr_struct((struct sockaddr_in*)saddr);
		} else {
			adu->addr.set_addr_struct((struct sockaddr_in6*)saddr);
		}
		adu->addr_valid = true;
	}
	adu->seq = mclcb->tx.get_next_toi();
	/* by default, unless the appli specifies a new TOI, next ADU will
	 * use this one... */
	mclcb->tx.incr_next_toi();
	/* set the importance of the ADU */
	if (mclcb->tx.next_adu_is_of_high_prio == true) {
		adu->priority = ADU_HIGH_PRIO;
		/* reset for next ADU */
		mclcb->tx.next_adu_is_of_high_prio = false;
	} else {
		adu->priority = ADU_NORMAL_PRIO;
	}

	/* all DUs for this ADU have this size */
	adu->symbol_len = mclcb->get_payload_size();
	if (mclcb->is_flute_compatible() 
#ifdef METAOBJECT_USED	
	    || (mclcb->meta_obj_layer->in_use() == true)
#endif
	) {
		if (adu->seq == 0) {
			/* FDT */
			adu->FDT_instanceid =
				mclcb->tx_flute.get_next_FDT_instanceID();
			mclcb->tx_flute.incr_next_FDT_instanceID();
		} else {
			adu->FDT_instanceid = 0;	// XXX: TODO...
		}
	}
	/*
	 * insert it in the transmission ADU list...
	 */
	mclcb->tx.insert_adu(mclcb, adu);
	/*
	 * store the application data...
	 */
	padded_len = (UINT32)(ceil((double)len / (double)adu->symbol_len)
			   * (double)adu->symbol_len);
	adu->padded_len = padded_len;
#ifdef VIRTUAL_TX_MEM
	if (mcl_vtm_can_store_in_vtm(mclcb, padded_len)) {
		/*
		 * use the VTMEM service to register data
		 */
		if (mcl_vtm_store_data(mclcb, adu, NULL, (char*)data, len, padded_len) ) {
			PRINT_ERR((mcl_stderr,
			"mcl_sendto: ERROR: Virtual Tx Memory service failed\n"))
			goto error;
		}
		if (mclcb->tx.can_reuse_appli_buf()) {
			/*
			 * free the data buffer.
			 * In fact using reuse_appli is here useless!
			 */
			free((void*)data);
			PRINT_ERR((mcl_stderr, "mcl_sendto: WARNING: using reuse_appli_tx_buffer is useless in Virtual Tx Memory mode\n"))
		}
	} else {
		/*
		 * store in physical tx memory (ptm)
		 */
#endif /* VIRTUAL_TX_MEM */
		if (mclcb->tx.can_reuse_appli_buf()) {
			/* take control of buf and avoid an extra data copy */
			if (padded_len != len) {
				/* add a null padding to end of block */
				if (!(adu->data = (char*)realloc((void*)data, padded_len))) {
					PRINT_ERR((mcl_stderr,
					"mcl_sendto: ERROR: out of memory\n"))
					goto error;
				}
				memset(adu->data + len, 0, padded_len - len);
			}
		} else {
			if (!(adu->data = (char*)malloc(padded_len))) {
				PRINT_ERR((mcl_stderr,
				"mcl_sendto: ERROR: out of memory\n"))
				goto error;
			}
			memcpy(adu->data, data, len);
			memset(adu->data + len, 0, padded_len - len);/*padding*/
			/*PRINT_OUT((mcl_stdout,
				"copied data to x%x, len=%d, padded_len=%d\n",
				(int)adu->data, len, padded_len))*/
		}
#ifdef VIRTUAL_TX_MEM
		if (mclcb->vtm_used) {
			/* remember it kept in physical memory */
			mcl_vtm_register_in_ptm(mclcb, adu, NULL, padded_len);
		}
	}
#endif /* VIRTUAL_TX_MEM */
	/*
	 * segment it...
	 */
	mcl_tx_segment_adu(mclcb, adu);
#ifdef FEC
	/*
	 * create additional FEC DUs for each block...
	 */
	for (i = adu->blocking_struct.block_nb, blk = adu->block_head; i > 0; i--, blk++) {
		tot_fec_nb += mclcb->fec.encode(mclcb, blk);
		/*PRINT_OUT((mcl_stdout,
			"tot_fec_nb=%d, blk->k=%d, blk->fec_nb=%d \n",
			tot_fec_nb, blk->k, blk->fec_du_nb_in_list))*/
#ifdef ANTICIPATED_TX_FOR_PUSH
		if (mclcb->anticipated_tx_for_push) {
			if (--try_2_send_count <= 0) {
				try_2_send_count = 4;
				/* now try to send */
				mclcb->periodic_proc.do_periodic_proc(mclcb);
			}
		}
#endif
	}
#endif /* FEC */
	/*
	 * planify transmissions...
	 */
	if (mclcb->tx.is_in_keep_data_mode()) {
		/*
		 * If we are in KEEP_DATA mode (in that case scheduling
		 * will be done later when the application issues a PUSH),
		 * then remember it for a future PUSH...
		 */
		mclcb->tx.register_adu_in_keep_data_mode(mclcb, adu);
#ifdef ANTICIPATED_TX_FOR_PUSH 
		if (mclcb->anticipated_tx_for_push &&
		    (mclcb->is_flute_compatible() == false)) {
			/* update tx planning for anticipated tx in push mode
			 * unless we are in FLUTE mode */
			TRACELVL(5, (mcl_stdout,
				"   mcl_sendto: anticipated tx of adu"))
			AnticipTx_UpdateTxPlanning(mclcb, adu);
			/* and now try to send */
			mclcb->periodic_proc.do_periodic_proc(mclcb);
		}
#endif /* ANTICIPATED_TX_FOR_PUSH */		
	} else {
		/*
		 * We are not in KEEP_DATA mode.
		 */
		UpdateTxPlanning(mclcb, adu, adu);
	}
	mclcb->tx.there_is_more_to_tx = true;
	/*
	 * update stats
	 */
	mclcb->stats.adus_announced++;
	mclcb->stats.buf_space += padded_len + tot_fec_nb * adu->symbol_len;
	if (mclcb->stats.buf_space > mclcb->stats.max_buf_space)
		mclcb->stats.max_buf_space = mclcb->stats.buf_space;
	if (mclcb->get_stats_level() > 0)
		mcl_print_tx_stats(mclcb);

	TRACELVL(5, (mcl_stdout, "<- mcl_sendto: return %d\n", len))
end:
	return len;

error:
	return -1;
#endif
}

#endif

/**
 * mcl_sendmsg.
 * => See header file for more informations.
 */
INT64
mcl_sendmsg (int		id,
	     struct mcl_msghdr	*msg,
	     mcl_msgflag	flags)
{
	INT64 		result;
	mcl_cb		*mclcb;

	if (id >= MAX_NB_MCLCB || (mclcb = mclcb_tab[id]) == NULL) {
		PRINT_ERR((mcl_stderr, "mcl_sendmsg: ERROR, bad id %d\n", id))
		return -1;
	}
	mclcb->lock();
	result = mcl_sendmsg2(mclcb, msg, flags);
	mclcb->unlock();
	return result;
}


/**
 * mcl_sendmsg2.
 * => See header file for more informations.
 */
INT64
mcl_sendmsg2(mcl_cb		*const mclcb,
	     struct mcl_msghdr	*msg,
	     mcl_msgflag	flags)
{
	adu_t		*adu;		/* DU descriptor of the data buffer */
#ifdef FEC
	block_t		*blk;
	INT32		i;
#endif /* FEC */
	INT32		tot_fec_nb = 0;	/* fec DUs created by mcl_fec_encode */
	struct sockaddr *saddr;		/* optional dest addr struct */
	int 		saddr_len;	/* optional dest addr struct length */
#ifdef ANTICIPATED_TX_FOR_PUSH
	int		try_2_send_count = 4;	/* period between 2 attempts */
#endif

	TRACELVL(5, (mcl_stdout, "-> mcl_sendmsg2:\n"))
#ifdef SVSOA_RECV
	mclcb->finish_session_init_if_needed(1);
#else
	mclcb->finish_session_init_if_needed();
#endif
	if (msg == NULL) {
		/*
		 * the appli issued this call just to finish the init...
		 * nothing else to do, return.
		 */
		TRACELVL(5, (mcl_stdout, "<- mcl_sendmsg2: no data, return\n"))
		goto end;
	}
	if (flags != MCL_MSG_DEFAULT) {
		PRINT_ERR((mcl_stderr,
		"mcl_sendmsg2: ERROR: invalid flags (expected %d, got %d)\n",
			MCL_MSG_DEFAULT, flags))
		goto error;
	}

#ifdef METAOBJECT_USED	
	/* check if we are sending a meta object */
	if (mclcb->meta_obj_layer->in_use() == true &&
	    mclcb->meta_obj_layer->completed == false) {
		if (mclcb->meta_obj_layer->add_object(msg) >= 0) {
			return msg->msg_iov->iov_len;
		} else {
			goto error;
		}
	}
#endif

	mclcb->fsm.update_tx_state(mclcb, TEVENT_NEW_ADU);
	if (mclcb->fsm.can_send_data(mclcb) == false) {
		TRACELVL(5, (mcl_stdout, "<- mcl_sendmsg2: ERROR, cannot send\n"))
		goto error;
	}
	//if (!data || len <= 0)
	if ((msg->msg_iov == NULL) || (msg->msg_iovlen <= 0)) {
		PRINT_ERR((mcl_stderr,
			"mcl_sendmsg2: ERROR: msg->msg_iov NULL or msg->msg_iovlen <= 0\n"))
		goto error;
	}
	saddr = (struct sockaddr*)(msg->msg_name);
	saddr_len = msg->msg_namelen;
	if (saddr && (saddr_len != sizeof(struct sockaddr_in))
		  &&  (saddr_len != sizeof(struct sockaddr_in6))) {
		PRINT_ERR((mcl_stderr,
			"mcl_sendmsg2: ERROR: msg->msg_name != NULL and bad msg->msg_namelen arg (only %d/%d possible, got %d)\n",
			sizeof(struct sockaddr_in),
			sizeof(struct sockaddr_in6), saddr_len))
		goto error;
	}

	/*
	 * Create the ADU descriptor...
	 */
	adu = mcl_create_adu(mclcb);
	adu->fec_scheme = mclcb->fec.get_fec_code();
	if (mclcb->fec.scheme_to_enc_inst(mclcb, adu->fec_scheme,
					&(adu->fec_encoding_id),
					&(adu->fec_instance_id)) == MCL_ERROR) {
		PRINT_ERR((mcl_stderr,
			"mcl_sendmsg2: ERROR: unsupported FEC scheme %d\n",
			adu->fec_scheme))
		goto error;
	}
	//adu->len = len;
	if (saddr_len > 0) {
		/* remember the dest addr to use (rather than default dest) */
		if (saddr_len == sizeof(struct sockaddr_in)) {
			adu->addr.set_addr_struct((struct sockaddr_in*)saddr);
		} else {
			adu->addr.set_addr_struct((struct sockaddr_in6*)saddr);
		}
		adu->addr_valid = true;
	}
	adu->seq = mclcb->tx.get_next_toi();
	/* by default, unless the appli specifies a new TOI, next ADU will
	 * use this one... */
	mclcb->tx.incr_next_toi();
	/* set the importance of the ADU */
	if (mclcb->tx.next_adu_is_of_high_prio == true) {
		adu->priority = ADU_HIGH_PRIO;
		/* reset for next ADU */
		mclcb->tx.next_adu_is_of_high_prio = false;
	} else {
		adu->priority = ADU_NORMAL_PRIO;
	}

	/* all DUs for this ADU have this size */
	adu->symbol_len = mclcb->get_payload_size();
	if (mclcb->is_flute_compatible() 
#ifdef METAOBJECT_USED	
	    || (mclcb->meta_obj_layer->in_use() == true)
#endif
	) {
		if (adu->seq == 0) {
			/* FDT */
			adu->FDT_instanceid =
				mclcb->tx_flute.get_next_FDT_instanceID();
			mclcb->tx_flute.incr_next_FDT_instanceID();
			if (mclcb->tx.is_in_keep_data_mode() == false) 
			/* TODO: remove the 'if' when MCL_OPT_STOP_TRANSMITTING_ADU is supported in KEEP mode */
			{
				/* remove previous ADU with toi == 0 */
				UINT64 toi = 0;
				mcl_ctl2(mclcb, MCL_OPT_STOP_TRANSMITTING_ADU, (void*)&toi, sizeof(toi));
			}
			
		} else {
			adu->FDT_instanceid = 0;
		}
	}
	/*
	 * insert it in the transmission ADU list...
	 */
	mclcb->tx.insert_adu(mclcb, adu);
	/*
	 * process the iovec structure, calculate the ADU length, and
	 * store the application data...
	 */
	if (mclcb->tx.copy_from_iovec_to_adu(mclcb, msg, adu) == MCL_ERROR) {
		goto error;
	}
	/*
	 * segment it...
	 */
	mcl_tx_segment_adu(mclcb, adu);
#ifdef FEC
	/*
	 * create additional FEC DUs for each block...
	 */
	for (i = adu->blocking_struct.block_nb, blk = adu->block_head;
	     i > 0; i--, blk++) {
		tot_fec_nb += mclcb->fec.encode(mclcb, blk);
		/*PRINT_OUT((mcl_stdout,
			"tot_fec_nb=%d, blk->k=%d, blk->fec_nb=%d \n",
			tot_fec_nb, blk->k, blk->fec_du_nb_in_list))*/
#ifdef ANTICIPATED_TX_FOR_PUSH
		if (mclcb->anticipated_tx_for_push) {
			if (--try_2_send_count <= 0) {
				try_2_send_count = 4;
				/* now try to send */
				mclcb->periodic_proc.do_periodic_proc(mclcb);
			}
		}
#endif
	}
#endif /* FEC */
	/*
	 * planify transmissions...
	 */
	if (mclcb->tx.is_in_keep_data_mode()) {
		/*
		 * If we are in KEEP_DATA mode, then remember the ADU
		 * for a future PUSH (here scheduling will be done later
		 * when the application issues a PUSH).
		 */
		mclcb->tx.register_adu_in_keep_data_mode(mclcb, adu);
#ifdef ANTICIPATED_TX_FOR_PUSH 
		if (mclcb->anticipated_tx_for_push &&
		    (mclcb->is_flute_compatible() == false)) {
			/* update tx planning for anticipated tx in push mode
			 * unless we are in FLUTE mode */
			TRACELVL(5, (mcl_stdout,
				"   mcl_sendmsg2: anticipated tx of adu"))
			AnticipTx_UpdateTxPlanning(mclcb, adu);
			/* and now try to send */
			mclcb->periodic_proc.do_periodic_proc(mclcb);
		}
#endif /* ANTICIPATED_TX_FOR_PUSH */		
	} else {
		/*
		 * We are not in KEEP_DATA mode.
		 */
		UpdateTxPlanning(mclcb, adu, adu);
	}
	mclcb->tx.there_is_more_to_tx = true;
	/*
	 * update stats
	 */
	mclcb->stats.adus_announced++;
	mclcb->stats.buf_space += adu->padded_len + tot_fec_nb * adu->symbol_len;
	if (mclcb->stats.buf_space > mclcb->stats.max_buf_space)
		mclcb->stats.max_buf_space = mclcb->stats.buf_space;
	if (mclcb->get_stats_level() > 0)
		mcl_print_tx_stats(mclcb);

	TRACELVL(5, (mcl_stdout, "<- mcl_sendmsg2: return %d\n", adu->len))
end:
	return adu->len;

error:
	return -1;
}


#if 0
#ifndef SVSOA_RECV	//Flute will never be used with SVSOA
/**
 * mcl_recv_flute.
 * => See header file for more informations.
 */
int
mcl_recv_flute (int		id,
		void		*buf,
		int		len,
		unsigned int	*toi)
{
	int	saddr_len = 0;
	return mcl_recvfrom_flute(id, buf, len, NULL, &saddr_len, toi);
}


/**
 * mcl_recvfrom_flute.
 * => See header file for more informations.
 */
int
mcl_recvfrom_flute (int		id,
	      void		*buf,
	      int		len,
	      struct sockaddr	*saddr,
	      int 		*saddr_len,
	      unsigned int	*toi)
{	
	int		rlen;
	mcl_cb		*mclcb;
	struct mcl_iovec	iov;
	struct mcl_msghdr	msg;

	if (id >= MAX_NB_MCLCB || (mclcb = mclcb_tab[id]) == NULL) {
		PRINT_ERR((mcl_stderr, "mcl_recvfrom_flute: ERROR, bad id %d\n", id))
		return -1;
	}
	mclcb->lock();
	TRACELVL(5, (mcl_stdout, "-> mcl_recvfrom: len=%d, saddr_len ptr=x%x\n",
		len, (int)saddr_len))
#ifdef SVSOA_RECV
	mclcb->finish_session_init_if_needed(1);
#else		
	mclcb->finish_session_init_if_needed();
#endif
	if (buf == NULL || len == 0) {
		/*
		 * the appli issued this call just to finish the init...
		 * nothing else to do, return.
		 */
		TRACELVL(5, (mcl_stdout, "<- mcl_recvfrom: NULL reception buffer, return\n"))
		rlen = 0;
		goto end;
	}
#if 0	/* can't check because the tx may be finished and the appli */
		/* only retrieves data available... */
	if (mclcb->fsm.can_recv_data(mclcb) == false) {
		PRINT_ERR((mcl_stderr, "mcl_recvfrom: ERROR: cannot recv\n"))
		goto error;
	}
#endif
	/*
	 * create a corresponding msg descriptor.
	 */
	memset((void*)&iov, 0, sizeof(iov));
	memset((void*)&msg, 0, sizeof(msg));
	if (saddr) {
		if (saddr_len == NULL) {
			PRINT_ERR((mcl_stderr,
			"mcl_recvfrom: ERROR: bad saddr_len arg (NULL pointer)\n"))
			goto error;
		}
		msg.msg_name = saddr;
		msg.msg_namelen = *saddr_len;
	} else {
		msg.msg_name = NULL;
		msg.msg_namelen = 0;
	}
	msg.msg_iov = &iov;
	msg.msg_iovlen = 1;
	iov.iov_type = MCL_IOV_TYPE_BUFFER;
	iov.iov_base = buf;
	iov.iov_len = len;
	/*
	 * check if an adu is available and copy it to the application buffer.
	 * Use polling for the present...
	 */
	while ((rlen = mclcb->rx_window.return_adu_to_appli(mclcb, &msg,
							    MCL_MSG_DEFAULT,
							    toi)) < 0) {
		if (mclcb->fsm.no_new_undup_du(mclcb)) {
			TRACELVL(5, (mcl_stdout, "<- mcl_recvfrom: closed, return -1\n"))
			goto error;
		}
		/*
		 * nothing received yet so wait a little bit (polling).
		 * the waiting time is a compromize... what should we use?
		 */
		mclcb->unlock();
		mcl_usleep(DFLT_POLLING_PERIOD);
		mclcb->lock();
		if (mclcb->rx_thread == 0)
			goto error;

	}
	/*
	 * copy back remote addr if appli is interested
	 */
	if (saddr) {
		*saddr_len = msg.msg_namelen;
	}
	TRACELVL(5, (mcl_stdout, "<- mcl_recvfrom: return %d\n", rlen))
end:
	mclcb->unlock();
	return rlen;

error:
	mclcb->unlock();
	return -1;
}
#endif /* !SVSOA_RECV */
#endif


/**
 * mcl_recv.
 * => See header file for more informations.
 */
INT32
mcl_recv (INT32	id,
	  void	*buf,
      INT32	len)
{
    INT32	saddr_len = 0;
	return mcl_recvfrom(id, buf, len, NULL, &saddr_len);
}

/**
 * mcl_recvfrom.
 * => See header file for more informations.
 */
INT32
mcl_recvfrom (INT32		id,
	      void		*buf,
	      INT32		len,
	      struct sockaddr	*saddr,
	  INT32 		*saddr_len)
{	
	int			rlen = 0;
	mcl_cb			*mclcb;
	struct mcl_iovec	iov;
	struct mcl_msghdr	msg;
    UINT32          	toi = 0;

	if (id >= MAX_NB_MCLCB || (mclcb = mclcb_tab[id]) == NULL) {
		PRINT_ERR((mcl_stderr, "mcl_recvfrom: ERROR, bad id %d\n", id))
		return -1;
	}
	mclcb->lock();
	TRACELVL(5, (mcl_stdout, "-> mcl_recvfrom: len=%d, saddr_len ptr=x%x\n",
		len, (int)saddr_len))
#ifdef SVSOA_RECV
	mclcb->finish_session_init_if_needed(mclcb->nb_layers);
#else		
	mclcb->finish_session_init_if_needed();
#endif
	if (buf == NULL || len == 0) {
		/*
		 * the appli issued this call just to finish the init...
		 * nothing else to do, return.
		 */
		TRACELVL(5, (mcl_stdout, "<- mcl_recvfrom: NULL reception buffer, return\n"))
		rlen = 0;
		goto end;
	}
#if 0	/* can't check because the tx may be finished and the appli */
		/* only retrieves data available... */
	if (mclcb->fsm.can_recv_data(mclcb) == false) {  
		PRINT_ERR((mcl_stderr, "mcl_recvfrom: ERROR: cannot recv\n"))
		goto error;
	}
#endif
	/*
	 * create a corresponding msg descriptor.
	 */
	memset((void*)&iov, 0, sizeof(iov));
	memset((void*)&msg, 0, sizeof(msg));
	if (saddr) {
		if (saddr_len == NULL) {
			PRINT_ERR((mcl_stderr,
			"mcl_recvfrom: ERROR: bad saddr_len arg (NULL pointer)\n"))
			goto error;
		}
		msg.msg_name = saddr;
		msg.msg_namelen = *saddr_len;
	} else {
		msg.msg_name = NULL;
		msg.msg_namelen = 0;
	}
	msg.msg_iov = &iov;
	msg.msg_iovlen = 1;
	iov.iov_type = MCL_IOV_TYPE_BUFFER;
	iov.iov_base = buf;
	iov.iov_len = len;
	/*
	 * check if an adu is available and copy it to the application buffer.
	 * Use polling for the present...
	 */
#ifdef METAOBJECT_USED	 
	if (mclcb->meta_obj_layer->in_use() == false)
	{
#endif
		while ((rlen = mclcb->rx_window.return_adu_to_appli(mclcb, &msg,
							MCL_MSG_DEFAULT, &toi)) < 0)
		{
			if (mclcb->fsm.no_new_undup_du(mclcb)) {
				TRACELVL(5, (mcl_stdout,
					"<- mcl_recvfrom: closed, return -1\n"))
				goto error;
			}
			/*
			 * nothing received yet so wait a little bit (polling).
			 * the waiting time is a compromize...
			 */
			mclcb->unlock();
			mcl_usleep(DFLT_POLLING_PERIOD);
			mclcb->lock();
			if (mclcb->rx_thread == 0)
				goto error;
			
#ifdef METAOBJECT_USED
			if (mclcb->meta_obj_layer->in_use() == true) 
			{ 	/* 
			 	 * We received a packet that indicated the use 
				 * of metaobjects. jump to appropriate loop.
			 	 */
				goto meta_object_used;
			}
#endif
		}

#ifdef METAOBJECT_USED		
		if (mclcb->meta_obj_layer->in_use() == true && toi == 0) 
		{ 	/* 
		 	 * We received a MODT and a packet that indicated
			 * the use of metaobjects.
			 * Pass MODT to the meta object mgmt and jump to
			 * appropriate loop.
		 	 */	 
			mclcb->meta_obj_layer->add_object_description_table((char*)buf, (u_int)rlen);
			goto meta_object_used;
		}
	} else {

meta_object_used:

		while (((rlen = mclcb->rx_window.return_adu_to_appli
				(mclcb, &msg, MCL_MSG_DEFAULT, &toi)) < 0)
	      		|| (toi == 0 && rlen > 0))
		{
			if (toi == 0 && rlen > 0) {
				/*
				 * we got a MODT... Pass it to the meta object
				 * mgmt layer only, but not to the application.
				 */
				mclcb->meta_obj_layer->add_object_description_table((char*)buf, (u_int)rlen);
			}
			else if ((rlen = mclcb->meta_obj_layer->check_for_decoded_objects(&msg, MCL_MSG_DEFAULT)) > 0) {
				break;
			}
			else if (mclcb->fsm.no_new_undup_du(mclcb)) {
				TRACELVL(5, (mcl_stdout,
					"<- mcl_recvfrom: closed, return -1\n"))
				goto error;
			}
			/*
			 * nothing received yet so wait a little bit (polling).
			 * the waiting time is a compromize... what should we use?
			 */
			mclcb->unlock();
			mcl_usleep(MODT_POLLING_PERIOD);
			mclcb->lock();
			if (mclcb->rx_thread == 0)
				goto error;
		}
	}
#endif /* METAOBJECT_USED */

	/*
	 * copy back remote addr len if appli is interested.
	 */
	if (saddr) {
		*saddr_len = msg.msg_namelen;
	}
	TRACELVL(5, (mcl_stdout, "<- mcl_recvfrom: return %d\n", rlen))
end:
	mclcb->unlock();
	return rlen;

error:
	mclcb->unlock();
	return -1;
}


/**
 * mcl_recvmsg extended data reception function.
 * => See header file for more informations.
 */
INT64
mcl_recvmsg (INT32 id,
	     struct mcl_msghdr	*msg,
	     enum mcl_msgflag	flags)
{
	int		rlen;
	mcl_cb		*mclcb;
    UINT32	toi = 0;

	if (id >= MAX_NB_MCLCB || (mclcb = mclcb_tab[id]) == NULL) {
		PRINT_ERR((mcl_stderr, "mcl_recvmsg: ERROR, bad id %d\n", id))
		return -1;
	}
	mclcb->lock();
	TRACELVL(5, (mcl_stdout, "-> mcl_recvmsg: \n"))
#ifdef SVSOA_RECV
	mclcb->finish_session_init_if_needed(mclcb->nb_layers);
#else		
	mclcb->finish_session_init_if_needed();
#endif
	if (msg == NULL) {
		/*
		 * the appli issued this call just to finish the init...
		 * nothing else to do, return.
		 */
		TRACELVL(5, (mcl_stdout, "<- mcl_recvmsg: NULL reception buffer, return\n"))
		rlen = 0;
		goto end;
	}
	/*
	 * check if an adu is available and copy it to the application buffer.
	 * Use polling for the present...
	 */
#ifdef METAOBJECT_USED
	if (mclcb->meta_obj_layer->in_use() == false) {
#endif	 
		while ((rlen = mclcb->rx_window.return_adu_to_appli(mclcb, msg,
							flags, &toi)) < 0)
		{
			if (mclcb->fsm.no_new_undup_du(mclcb)) {
				TRACELVL(5, (mcl_stdout,
					"<- mcl_recvfrom: closed, return -1\n"))
				goto error;
			}
			/*
			 * nothing received yet so wait a little bit (polling).
			 * the waiting time is a compromize...
			 */
			mclcb->unlock();
			mcl_usleep(DFLT_POLLING_PERIOD);
			mclcb->lock();
			if (mclcb->rx_thread == 0)
				goto error;
#ifdef METAOBJECT_USED			
			if (mclcb->meta_obj_layer->in_use() == true) 
			{
				/* 
			 	 * We received a packet that indicated the use
				 * of metaobjects. jump to appropriate loop.
			 	 */
				goto meta_object_used;
			}
#endif
		}
#ifdef METAOBJECT_USED		
		if (mclcb->meta_obj_layer->in_use() == true && toi == 0) {
			/* 
		 	 * We received a MODT and a packet that indicated the
			 * use of metaobjects.
			 * Pass MODT to the meta object mgmt and jump to
			 * appropriate loop.
		 	 * If we are not in MCL_MSG_DEFAULT, the next
			 * return_adu_to_appli will return the same ADU
			 * again, and will then be passed to the meta object
			 * mgmt.
			 */
			if (flags == MCL_MSG_DEFAULT)
				mclcb->meta_obj_layer->add_object_description_table((char*)msg->msg_iov->iov_base, (u_int)rlen);
			goto meta_object_used;
		}
	} else {

meta_object_used:
		while (((rlen = mclcb->rx_window.return_adu_to_appli(mclcb, msg,
						flags, &toi)) < 0)
	      					|| (toi == 0 && rlen > 0))
		{
			if (toi == 0 && rlen > 0)
			{
				/*
				 * we got a MODT... Pass it to the meta object 
				 * mgmt layer only, but not to the application.
				 */
				if (flags != MCL_MSG_DEFAULT)
				{
					/* object has not been copied, it was only
					 * a check. So call return_adu_to_appli and
					 * now copy it */
					char * buf = (char *) malloc(rlen);
					struct mcl_msghdr	msg2;
					struct mcl_iovec	iov2;	

					/*
					 * create a corresponding msg descriptor.
					 */
					msg2.msg_iov = &iov2;
					msg2.msg_iovlen = 1;
					msg2.msg_name = NULL;
					msg2.msg_namelen = 0;
					iov2.iov_type = MCL_IOV_TYPE_BUFFER;
					iov2.iov_base = buf;
					iov2.iov_len = rlen;
					iov2.iov_filename = NULL;
					iov2.iov_offset = 0;
					/* get the MODT now and pass it to the
					 * meta object mgmt layer */
					mclcb->rx_window.return_adu_to_appli(mclcb,
							&msg2, MCL_MSG_DEFAULT, NULL);
					mclcb->meta_obj_layer->add_object_description_table((char*)buf, (u_int)rlen);
					free(buf);
				}
				else
				{
					mclcb->meta_obj_layer->add_object_description_table((char*)msg->msg_iov->iov_base, (u_int)rlen);
				}
			}
			else if ((rlen = mclcb->meta_obj_layer->check_for_decoded_objects(msg, flags)) > 0) {
				break;
			}
			else if (mclcb->fsm.no_new_undup_du(mclcb)) {
				TRACELVL(5, (mcl_stdout,
					"<- mcl_recvmsg: closed, return -1\n"))
				goto error;
			}
			/*
			 * nothing received yet so wait a little bit (polling).
			 * the waiting time is a compromize...
			 * what should we use?
			 */
			mclcb->unlock();
			mcl_usleep(MODT_POLLING_PERIOD);
			mclcb->lock();
			if (mclcb->rx_thread == 0)
				goto error;
		}
	}
#endif /* METAOBJECT_USED */


	TRACELVL(5, (mcl_stdout, "<- mcl_recvmsg: return %d\n", rlen))
end:
	mclcb->unlock();
	return rlen;

error:
	mclcb->unlock();
	return -1;
}


/**
 * mcl_wait_event waits (i.e. blocks) until a given event takes
 * place and then returns.
 * => See header file for more informations.
 */
int
mcl_wait_event (int	id,
		int	event)
{
	mcl_cb	*mclcb;

	if (id >= MAX_NB_MCLCB || (mclcb = mclcb_tab[id]) == NULL) {
		PRINT_ERR((mcl_stderr, "mcl_wait_event: ERROR, bad id %d\n", id))
		return -1;
	}
	mclcb->lock();
	switch(event) {
	case MCL_WAIT_EVENT_END_TX:
		/*
		 * wait untill all DUs have been sent
		 */
		if (mclcb->is_a_sender() == false) {
			PRINT_ERR((mcl_stderr,
			"mcl_wait_event: MCL_WAIT_EVENT_END_TX, ERROR, id %d isn't sender\n", id))
			goto error;
		}
		while (mclcb->tx.there_is_more_to_tx) {
			TRACELVL(5, (mcl_stdout, "   mcl_wait_event: MCL_WAIT_EVENT_END_TX, wait a bit...\n"))
			mclcb->unlock();
			mcl_usleep(DFLT_POLLING_PERIOD);
			mclcb->lock();
		}
		break;

	case MCL_WAIT_EVENT_END_RX:
		/*
		 * wait untill a sufficient number of DUs have been received
		 * to complete all ADUs
		 */
		if (mclcb->is_a_receiver() == false) {
			PRINT_ERR((mcl_stderr,
			"mcl_wait_event: MCL_WAIT_EVENT_END_RX, ERROR, id %d isn't receiver\n", id))
			goto error;
		}
		while (!mclcb->fsm.no_new_undup_du(mclcb)) {
			TRACELVL(5, (mcl_stdout, "   mcl_wait_event: MCL_WAIT_EVENT_END_RX, wait a bit...\n"))
			mclcb->unlock();
			mcl_usleep(DFLT_POLLING_PERIOD);
			mclcb->lock();
		}
		break;

	case MCL_WAIT_EVENT_CLOSED:
		/*
		 * wait untill we receive a CLOSE message from source,
		 * indicating that all DUs have been sent...
		 */
		if (mclcb->is_a_receiver() == false) {
			PRINT_ERR((mcl_stderr,
			"mcl_wait_event: MCL_WAIT_EVENT_CLOSED, ERROR, id %d isn't receiver\n", id))
			goto error;
		}
		/*
		 * NB: it requires that the receiver continues to receive
		 * DUs, even after the completion of all the ADUs, so don't
		 * leave layer 0 otherwise the CLOSE messages may be lost...
		 */
		if (!mclcb->rx.never_leave_base_layer) {
			PRINT_ERR((mcl_stderr,
			"mcl_wait_event: MCL_WAIT_EVENT_CLOSED, ERROR, use NEVER_LEAVE_BASE_LAYER option first\n"))
			goto error;
		}
		while (!mclcb->fsm.close_already_rx(mclcb)) {
			TRACELVL(5, (mcl_stdout, "   mcl_wait_event: MCL_WAIT_EVENT_CLOSED, wait a bit...\n"))
			mclcb->unlock();
			mcl_usleep(DFLT_POLLING_PERIOD);
			mclcb->lock();
		}
		break;

	default:
		PRINT_ERR((mcl_stderr,
			"mcl_wait_event: ERROR, unknown event %d\n", event))
		goto error;
	}
	mclcb->unlock();
	return 0;

error:
	mclcb->unlock();
	return -1;
}


/*
 * mcl_select
 *
 * WARNING: this is a limited version of mcl_select that only accepts
 * a single MCL fd of read type, with a timeout.
 * All uses with several readfds, a non null writefds or exceptfds
 * will fail.
 *
 * params : same at the Socket select() syscall
 * 
 * return : same at the Socket select() syscall
 * 	    WARNING: the timeout parameter is undefined after calling
 * 	    mcl_select, do not assume linux' select(2) like behaviour
 * 	    concerning timeout...
 */
int
mcl_select (int			n,
	    fd_set		*readfds,
	    fd_set		*writefds,
	    fd_set		*exceptfds,
	    struct timeval	*timeout)
{
	mcl_cb		*mclcb;
	int		id;	/* readfd of interest (there should be only 1)*/
	int		rlen;
	struct timeval	begin;	/* time starts here... */
	struct timeval	end;	/* ...and stops here */
	struct timeval	cur;	/* current time to compare with end */
	struct mcl_msghdr msg;	/* needed by mcl_recvmsg */

	if (writefds != NULL || exceptfds != NULL) {
		PRINT_ERR((mcl_stderr,
		"mcl_select: ERROR, non NULL writefds or exceptfds not supported\n"))
		return -1;
	}
	if (readfds == NULL) {
		PRINT_ERR((mcl_stderr,
		"mcl_select: ERROR, NULL readfs not supported\n"))
		return -1;
	}
	if (timeout) {
		/* sanity checks */
		if (timeout->tv_sec < 0 || timeout->tv_usec < 0 ||
		    timeout->tv_usec >= 1000000) {
			PRINT_ERR((mcl_stderr,
			"mcl_select: ERROR, invalid timeout value\n"))
			return -1;
		}
		/* calculate timeout date */
		mcl_gettimeofday(&begin);
		end.tv_usec = (begin.tv_usec + timeout->tv_usec) % 1000000;
		end.tv_sec = begin.tv_sec + timeout->tv_sec +
			(begin.tv_usec + timeout-> tv_usec) / 1000000;
	}
#ifdef DEBUG
	/* check there is a single fd set in readfds */
#endif
	id = n - 1;
	if (id >= MAX_NB_MCLCB || (mclcb = mclcb_tab[id]) == NULL) {
		PRINT_ERR((mcl_stderr, "mcl_select: ERROR, bad id %d\n", id))
		return -1;
	}
	mclcb->lock();
	/*
	 * since the problem is to wait at most timeout until a new ADU
	 * is available on this session, we can reuse the mcl_recvfrom
	 * code, with the only exception that we don't read data from
	 * the incoming list, just check if there is some available.
	 */
	TRACELVL(5, (mcl_stdout, "-> mcl_select: watch mclcb=x%x\n", (int)mclcb))
#ifdef SVSOA_RECV
	mclcb->finish_session_init_if_needed(1);
#else
	mclcb->finish_session_init_if_needed();
#endif

#if 0
	if (!mclcb->initialized) {
#ifdef SVSOA_RECV
		mcl_end_init_mclcb(mclcb, 1);
#else
		mcl_end_init_mclcb(mclcb);
#endif
	}
#endif
	/*
	 * check if an adu is available without removing it from MCL's
	 * incoming list (use NULL buffer/ 0 len for that).
	 * XXX : use polling for the present... change it!
	 */
	/* create a corresponding msg descriptor */
	memset((void*)&msg, 0, sizeof(msg));
	while ((rlen = mclcb->rx_window.return_adu_to_appli(mclcb, &msg,
					MCL_MSG_GET_INFO_FOR_AVAILABLE_OBJECT,
					0)) < 0) {
		if (mclcb->fsm.no_new_undup_du(mclcb)) {
			TRACELVL(5, (mcl_stdout, "<- mcl_recvfrom: closed, return -1\n"))
			return 0;
		}
		/*
		 * check if we can afford to wait some more...
		 */
		if (timeout) {
			mcl_gettimeofday(&cur);
			if (cur.tv_sec > end.tv_sec ||
			    (cur.tv_sec == end.tv_sec &&
			     cur.tv_usec > end.tv_usec)) {
				TRACELVL(5, (mcl_stdout, "<- mcl_select: timeout\n"))
				mclcb->unlock();
				return 0;
			}
		} else {
			TRACELVL(5, (mcl_stdout, "<- mcl_select: nothing available\n"))
			mclcb->unlock();
			return 0;
		}
		/*
		 * nothing received yet so wait a little bit (polling).
		 * the waiting time is a compromize... what should we use?
		 */
		mclcb->unlock();
		mcl_usleep(DFLT_POLLING_PERIOD);
		mclcb->lock();
	}
	ASSERT(rlen == 0);
	TRACELVL(5, (mcl_stdout, "<- mcl_select: new ADU available\n"))
	mclcb->unlock();
	return 1;
}

