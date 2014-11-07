/* $Id: mcl_layer.cpp,v 1.17 2005/05/24 10:37:57 roca Exp $ */
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
 *	Layer management functions.
 */

#include "mcl_includes.h"

/*
 * init the number of layers as required
 */
#ifdef SVSOA_RECV
int
mcl_init_layer_nb (mcl_cb	*mclcb,
		   int		nb_lay)
#else
int
mcl_init_layer_nb (mcl_cb	*mclcb)
#endif
{
	TRACELVL(5, (mcl_stdout, "-> mcl_init_layer_nb:\n"))
	ASSERT(mclcb->max_nb_layers <= MAX_NB_TX_LAYERS);
	if (mclcb->is_a_receiver()) {
#ifdef SVSOA_RECV
		/* use immediately the number of layers specified */
		mclcb->nb_layers = nb_lay;	
#else
		/* only 1 layer */
		mclcb->nb_layers = 1;
#endif
	} else {
		ASSERT(mclcb->is_a_sender());
		mclcb->nb_layers = mclcb->max_nb_layers;
	}
	TRACELVL(5, (mcl_stdout,
		"<- mcl_init_layer_nb: (%s) nb_layers=%d, max_nb_layers=%d\n",
		(mclcb->is_a_receiver() ? "receiver" : "sender"),
		mclcb->nb_layers, mclcb->max_nb_layers))
	return 0;
}


/*
 * Create all the required sockets and initialize them.
 * Used both for sender and receiver, and unicast or multicast.
 * @param mclcb
 * @return		Return 0 if ok, < 0 if error.
 */
mcl_error_status
mcl_init_layer_sockets (mcl_cb	*mclcb)
{
	int			layer;
	mcl_socket		*so;
	//struct sockaddr_in	saddr;
	mcl_addr		addr;	/* temp session addr for each layer */

	TRACELVL(5, (mcl_stdout, "-> mcl_init_layer_sockets:\n"))

	//for (layer = 0, so = mclcb->socket_tab; layer < mclcb->mcl_max_group;
	for (layer = 0, so = mclcb->socket_tab; layer < mclcb->max_nb_layers;
	     layer++, so++) {
		addr = mclcb->addr;
		if (addr.is_multicast_addr()) {
			/*
			 * if we are a receiver, subscribe only to the
			 * groups of interest and skip the others...
			 */
			if (mclcb->is_a_receiver() &&
			    (layer >= mclcb->nb_layers)) {
				continue;
			}
			/* use a block of mcast addr... */
			addr.incr_addr(layer);
			/* changing the port number is required to make
			 * sure packets are delivered to the right socket */
			addr.incr_port(layer);
		} else {
			/* this is unicast, so a single socket is sufficient */
			if (layer >= 1) {
				continue;
			}
		}
		/*
		 * init everything
		 */
		if (mclcb->is_a_sender()) {
			if (so->init_as_sender(mclcb, layer, &addr,
					mclcb->mcast_if_addr,
					mclcb->mcast_if_name) == MCL_ERROR) {
				TRACELVL(5, (mcl_stdout, "<- mcl_init_layer_sockets: ERROR\n"))
				return MCL_ERROR;
			}	
		} else {
			ASSERT(mclcb->is_a_receiver());
			if (so->init_as_receiver(mclcb, layer, &addr,
#ifdef SSM
					&(mclcb->rx.src_addr),
#endif
					mclcb->mcast_if_addr,
					mclcb->mcast_if_name) == MCL_ERROR) {
				TRACELVL(5, (mcl_stdout, "<- mcl_init_layer_sockets: ERROR\n"))
				return MCL_ERROR;
			}
		}
	}
	TRACELVL(5, (mcl_stdout, "<- mcl_init_layer_sockets:\n"))
	return MCL_OK;
}


/*
 * add the specified layer or highest layer if layer is MCL_HIGHEST_LAYER
 * returns 0 if OK, < 0 if error
 */
int
mcl_add_layer (mcl_cb	*mclcb,
	       INT32	layer)
{
	//struct sockaddr_in	saddr;
	mcl_addr	addr;

	TRACELVL(5, (mcl_stdout, "-> mcl_add_layer: %d\n", layer))
	if (layer == MCL_HIGHEST_LAYER)
		layer = mclcb->nb_layers;
	if (layer != mclcb->nb_layers) {
		/* cumulative scheme so do not allow gaps in layers */
		goto bad;
	}
	if (mclcb->nb_layers + 1 > mclcb->max_nb_layers) {
		/* we have reached the maximum nb of layer */
		goto bad;
	}
	if (mclcb->ucast_mcast_mode & MODE_MCAST_RX) {
		/* only with multicast */
		ASSERT(mclcb->addr.is_multicast_addr());
		addr = mclcb->addr;
		addr.incr_addr(layer);
		/* changing the port number is in fact only required
		 * by Solaris, not by Linux! */
		addr.set_port(addr.get_port() + layer);
		/*
		 * now init everything
		 */
		if (mclcb->socket_tab[layer].init_as_receiver(mclcb, layer,
					&addr,
#ifdef SSM
					&(mclcb->rx.src_addr),
#endif
					mclcb->mcast_if_addr,
					mclcb->mcast_if_name) == MCL_ERROR) {
			TRACELVL(5, (mcl_stdout, "<- mcl_init_layer_sockets: ERROR\n"))
			return MCL_ERROR;
		}
	}
	mclcb->nb_layers++;
	if (mclcb->get_verbosity() == 2) {
		struct timeval	time;
		time = mcl_get_tvtime();
		PRINT_OUT((mcl_stdout, "\n%ld.%06ld\tadd_layer %d\n",
			time.tv_sec, time.tv_usec, layer))
	}
	TRACELVL(5, (mcl_stdout, "<- mcl_add_layer: new nb_layer=%d\n", mclcb->nb_layers))
	return 0;

bad:
	if (mclcb->get_verbosity() >= 4)
		PRINT_ERR((mcl_stderr, "mcl_add_layer: ERROR, can't add layer %d\n",
			layer))
	TRACELVL(5, (mcl_stdout, "<- mcl_add_layer: ERROR\n"))
	return -1;
}


/*
 * drop the specified layer or highest layer if layer is MCL_HIGHEST_LAYER
 * after unsubscribing to multicast group
 * returns 0 if OK, < 0 if error
 * It is possible to call it in "check" mode just to see if dropping
 * would be possible...
 *
 * TO DO: case where multiple layer -> single socket
 */
int
mcl_drop_layer (mcl_cb	*mclcb,
		INT32	layer,		/* layer to drop */
		int	check)		/* MCL_CHECK_ONLY if check w/o */
					/* dropping MCL_DO_IT otherwise */
{
	TRACELVL(5, (mcl_stdout, "-> mcl_drop_layer: layer=%d\n", layer))
	if (mclcb->nb_layers <= 0) {
		/* already done, return */
		TRACELVL(5, (mcl_stdout, "   mcl_drop_layer: already dropped\n"))
		goto ok;
	}
	if (layer == MCL_ALL_LAYERS) {
		/* drop everything, including layer 0 if possible */
		while (mclcb->nb_layers > 1) {
			mcl_drop_layer(mclcb, MCL_HIGHEST_LAYER,
					MCL_DO_IT);
		}
		ASSERT(mclcb->nb_layers == 1);
		if (!mclcb->rx.never_leave_base_layer)
			mcl_drop_this_layer(mclcb, 0); /* and layer 0 */
		goto ok;
	} else if (layer == MCL_HIGHEST_LAYER) {
		layer = mclcb->nb_layers - 1;
	} else if (layer != mclcb->nb_layers - 1) {
		/* this is a cumulative scheme */
		/* => can only drop highest layer */
		goto bad;
	}
	if (layer <= 0) {
		/* can't drop layer 0, no matter wether */
		/* never_leave_base_layer is set or not */
		goto bad;
	}
	if (check == MCL_CHECK_ONLY) {
		TRACELVL(5, (mcl_stdout, "<- mcl_drop_layer: drop possible\n"))
		return 0;		/* do nothing */
	}
	/* ok, possible so drop it */
	mcl_drop_this_layer(mclcb, layer);
ok:
	TRACELVL(5, (mcl_stdout, "<- mcl_drop_layer:\n"))
	return 0;

bad:
	if (mclcb->get_verbosity() >= 4)
		PRINT_ERR((mcl_stderr, "mcl_drop_layer: ERROR cant drop layer %d\n",
			layer))
	TRACELVL(5, (mcl_stdout, "<- mcl_drop_layer: ERROR\n"))
	return -1;
}


/*
 * same as mcl_drop_layer except that there is no check (which enables
 * to drop layer 0!). Use with care...
 */
int
mcl_drop_this_layer (mcl_cb	*mclcb,
		     INT32	layer)	/* layer to drop */
{
	mcl_socket		*so;
	mcl_addr		addr;

	TRACELVL(5, (mcl_stdout, "-> mcl_drop_this_layer: layer=%d\n", layer))
	mclcb->nb_layers--;
	/* close the socket but do not free anything (call reset for that) */
	so = &(mclcb->socket_tab[layer]);
	so->close_sockets(mclcb);
#if 0
	if (mclcb->ucast_mcast_mode & MODE_MCAST_RX) {
		/*
		 * drop multicast group first...
		 * NB: check consistency with mcl_layer_sock_init()
		 */
		struct ip_mreq	imr;

		mg = &(mclcb->socket_tab[layer]);
#if 0
		imr.imr_multiaddr.s_addr = mg->saddr.sin_addr.s_addr;
		imr.imr_interface.s_addr = htonl(mclcb->mcast_if);
#endif
		imr.imr_multiaddr.s_addr = htonl(mg->addr.get_addr());
		imr.imr_interface.s_addr = htonl(mclcb->mcast_if.get_addr());
		if (setsockopt(mg->ses_sock, IPPROTO_IP, IP_DROP_MEMBERSHIP,
		    (char *)&imr, sizeof(imr)) < 0) {
			perror("mcl_drop_this_layer: IP_DROP_MEMBERSHIP");
			mcl_exit(1);
		}
		if (mg->can_rx()) {
			/* remove from select() */
			FD_CLR((u_int) mg->ses_sock, &(mclcb->rxlvl.fds));
			if (mclcb->rxlvl.nfds == (int)mg->ses_sock + 1) /* TODO WIN32 compliant? */
				mclcb->rxlvl.nfds--;
			mclcb->rxlvl.n_fd--;
		}
		mg->can_rx = mg->can_tx = 0;

		if (mcl_is_valid_sock(mg->ses_sock)) {
#ifdef WIN32
			closesocket(mg->ses_sock);
			mg->ses_sock = INVALID_SOCKET;
#else /* UNIX */
			close(mg->ses_sock);
			mg->ses_sock = 0;
#endif
		}
		if (mcl_is_valid_sock(mg->priv_sock)) {
#ifdef WIN32
			closesocket(mg->priv_sock);
			mg->priv_sock = INVALID_SOCKET;
#else /* UNIX */
			close(mg->priv_sock);
			mg->priv_sock = 0;
#endif
		}
		memset(mg, 0, sizeof(*mg));
	} else if ((mclcb->ucast_mcast_mode & MODE_UNI_RX) && layer == 0) {
		/*
		 * we are a unicast rx...
		 * do nothing (layering is only simulated) except if
		 * layer 0 is dropped in which case we close everything!
		 */
		ASSERT(!(mclcb->ucast_mcast_mode & MODE_MCAST_RX));
		mg = &(mclcb->socket_tab[0]);
		if (mg->can_rx) {
			/* remove from select() */
			FD_CLR((u_int) mg->ses_sock, &(mclcb->rxlvl.fds));
			if (mclcb->rxlvl.nfds == (int)mg->ses_sock + 1) /* TODO WIN32 compliant? */
				mclcb->rxlvl.nfds--;
			mclcb->rxlvl.n_fd--;
		}
		mg->can_rx = mg->can_tx = 0;

		if (mcl_is_valid_sock(mg->ses_sock)) {
#ifdef WIN32
			closesocket(mg->ses_sock);
			mg->ses_sock = INVALID_SOCKET;
#else /* UNIX */
			close(mg->ses_sock);
			mg->ses_sock = 0;
#endif
		}
		if (mcl_is_valid_sock(mg->priv_sock)) {
#ifdef WIN32
			closesocket(mg->priv_sock);
			mg->priv_sock = INVALID_SOCKET;
#else /* UNIX */
			close(mg->priv_sock);
			mg->priv_sock = 0;
#endif
		}
		memset(mg, 0, sizeof(*mg));
	} else if ((mclcb->ucast_mcast_mode & MODE_MCAST_TX) ||
		   ((mclcb->ucast_mcast_mode & MODE_UNI_TX) && layer == 0)) {
		/*
		 * we are a source...
		 * just close the sending socket (there is no session fd)
		 */
		mg = &(mclcb->socket_tab[layer]);
		ASSERT(!mg->ses_sock);
		if (mcl_is_valid_sock(mg->priv_sock)) {
#ifdef WIN32
			closesocket(mg->priv_sock);
			mg->priv_sock = INVALID_SOCKET;
#else /* UNIX */
			close(mg->priv_sock);
			mg->priv_sock = 0;
#endif
		}
	}
#endif

	if (mclcb->get_verbosity() == 2) {
		struct timeval	time;
		time = mcl_get_tvtime();
		PRINT_OUT((mcl_stdout, "\n%ld.%06ld\tdrop_layer %d\n",
			time.tv_sec, time.tv_usec, layer))
	}
	TRACELVL(5, (mcl_stdout, "<- mcl_drop_this_layer: new nb_lvl=%d\n",
		mclcb->nb_layers))
	return 0;
}


/*
 * Free all the layers and associated sockets.
 * Used both for sender and receiver, and unicast or multicast.
 * Return 0 if ok, < 0 if error.
 */
int
mcl_free_all_layers (mcl_cb	*const mclcb)
{
	INT32			i;
	mcl_socket		*so;

	TRACELVL(5, (mcl_stdout, "-> mcl_close_sockets:\n"))
	if (mclcb->is_a_receiver()) {
		/*
		 * we are above all a receiver. Use the dedicated function...
		 */
		mcl_drop_layer(mclcb, MCL_ALL_LAYERS, MCL_DO_IT);
	} else {
		/*
		 * we are above all a sender. Go through all socket entries...
		 */
		for (i = 0, so= mclcb->socket_tab; i < mclcb->mcl_max_group;
		     i++, so++) {
			if (!mclcb->addr.is_multicast_addr() && i > 0) {
				/*
				 * there are several socket entries but send 
				 * only on one socket !
				 */
				continue;
			}
			/* now close it */
			mcl_drop_this_layer(mclcb, i);
		}
	}
	TRACELVL(5, (mcl_stdout, "<- mcl_close_sockets:\n"))
	return 0;
}

