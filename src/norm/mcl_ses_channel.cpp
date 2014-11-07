/* $Id: mcl_ses_channel.cpp,v 1.4 2004/06/15 15:53:27 roca Exp $ */
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

#include "mcl_includes.h"


mcl_ses_channel::mcl_ses_channel ()
{
	struct sockaddr_in	saddr;	// temp storage of default session addr

	this->ses_sock = 0;
	this->priv_sock = 0;
	this->ttl = 1;
	this->socket_size = MCL_DFLT_SOCKET_SIZE;
	this->can_tx = false;
	this->can_rx = false;

	this->is_mcast = -1;
	FD_ZERO(&(this->fds));
	this->nfds = -1;
	this->n_fd = 0;
#if defined(SIMUL_TX_LOSSES) || defined(SIMUL_RX_LOSSES)
	this->simul_losses_state = 0;
#endif

	// default session addr init.
	saddr.sin_family = AF_INET;
	saddr.sin_port = htons(2323);
	if (inet_aton("225.1.2.3", &(saddr.sin_addr)) == 0) {
		perror("mcl_ses_channel::mcl_ses_channel: inet_aton");
		mcl_exit(1);
	}
	this->ses_addr.set_addr_struct(&saddr);
}


mcl_ses_channel::~mcl_ses_channel ()
{
	this->ses_sock = 0;
	this->priv_sock = 0;
	this->can_tx = this->can_rx = false;
	this->is_mcast = -1;
}


/**
 * Sends a data or control packet on this channel.
 */
mcl_error_status
mcl_ses_channel::send_pkt (mcl_cb		*const mclcb,
			   norm_common_hdr_t	*const hdr,
			   INT32		const hlen,
			   mcl_du		*const du)
{
	//mcl_addr	a;
	if (du) {
		/* use the adu destination addr */
		//a = du->block->adu->addr;
		//return (this->send_pkt(mclcb, &(du->block->adu->addr),
		return (this->send_pkt(mclcb, du->block->adu->addr,
					hdr, hlen, du));
	} else {
		/* use the default destination addr */
		//a = &(mclcb->ses_channel.ses_addr);
		//return (this->send_pkt(mclcb, &(mclcb->ses_channel.ses_addr),
		return (this->send_pkt(mclcb, mclcb->ses_channel.ses_addr,
					hdr, hlen, du));
	}
}


/**
 * Sends a data or control packet on this channel.
 * The destination address is specified and can be either a unicast
 * or multicast address.
 */
mcl_error_status
mcl_ses_channel::send_pkt (mcl_cb		*const mclcb,
			   //mcl_addr		*const addr1,
			   mcl_addr		const addr1,
			   norm_common_hdr_t	*const hdr,
			   INT32		const hlen,
			   mcl_du		*const du)
{
	mcl_adu			*adu;
	INT32			len;
	MCL_IOVEC		iov[2];		/* to describe header + data */
	struct sockaddr_in	sa;
	INT32			sa_len;
#ifdef WIN32
	struct sockaddr	*msg_name;
	u_int32_t	msg_namelen;
#else
	struct msghdr	msg;		/* for the sendmsg() Socket syscall */
#endif 

	TRACELVL(5, (mcl_stdout, "-> mcl_ses_channel::send_pkt:\n"))
	/*
	 * update traces and statistics
	 */
	if (mclcb->get_verbosity() >= 2) {
		mcl_print_sent_du(mclcb, du);
		if (du && mclcb->get_verbosity() >= 4) {
			/* header and data are in two diff buffers */
			mcl_dump_buffer(du->data, hlen, (hlen>> 2));
		}
	}
	if (du) {
		adu = du->block->adu;
		len = du->len;
	} else {
		adu = NULL;
		len = 0;
	}
	if (len > 0) {
		if (du->is_fec) {
			mclcb->stats.tx_fec_pkts++; 
			mclcb->stats.tx_fec_bytes += len + hlen;
		} else {
			mclcb->stats.tx_data_pkts++;
			mclcb->stats.tx_data_bytes += len + hlen;
		}
	} 
	mclcb->stats.tx_tot_pkts++;
	mclcb->stats.tx_tot_bytes += len + hlen;

	/*
	 * prepare the iovec and the provided destination addr
	 */
	MCL_IOV_BUFF(iov[0]) = (MCL_IOV_BUFF_TYPE)hdr;
	MCL_IOV_LEN(iov[0])  = hlen;
	if (len > 0) {
		ASSERT(du->data);
		MCL_IOV_BUFF(iov[1]) = (MCL_IOV_BUFF_TYPE)du->data;
		MCL_IOV_LEN(iov[1]) = len;

	}
	//ASSERT(addr1);
#ifdef WIN32
	TODO
#else /* UNIX */
#if 0
printf("adu->addr = x%x\n", (int)&(adu->addr));
printf("adu->addr = %s\n", adu->addr.get_addr_string());
printf("addr = x%x\n", (int)addr1);
printf("port = %d\n", addr1->get_port());
printf("addr = %s\n", addr1->get_addr_string());
#endif
	memset(&msg, 0, sizeof(msg));
	addr1.get_addr_struct(&sa);
	sa_len = sizeof(sa);
	msg.msg_name = (char*)&sa;
	msg.msg_namelen = sa_len;

	msg.msg_iov = iov;
	msg.msg_iovlen = (len > 0 ? 2 : 1);
#endif /* OS_DEP */

	/*
	 * and now send the packet
	 */
#ifdef SIMUL_TX_LOSSES
	if (this->sim_random_loss(mclcb) == false) {
		/* no simulated loss, tx can take place */
#endif /* SIMUL_TX_LOSSES */
#ifdef WIN32
		if (WSASendTo(mclcb->ses_channel.priv_sock, iov, (len > 0 ? 2 : 1), NULL, 0,
			      msg_name, msg_namelen, NULL, NULL) == 0) {
			/*
			 * if (mclcb->verbose == 2)
			 *	malloc_stats();
			 */
			perror("mcl_ses_channel::send_pkt: sendmsg");
			mcl_exit(1);
		}
#else /* UNIX */
		if (sendmsg(mclcb->ses_channel.priv_sock, &msg, 0) != hlen + len) {
			/*
			 * if (mclcb->verbose == 2)
			 *	malloc_stats();
			 */
			perror("mcl_ses_channel::send_pkt: sendmsg");
			mcl_exit(1);
		}
#endif /* OS_DEP */

		if (mclcb->get_verbosity() >= 4) {
#ifdef WIN32
			PRINT_OUT((mcl_stdout,
			"sendmsg: priv_sock=%d, dst=%s/%d, hdr_len=%d, payload_len=%d\n",
			mclcb->ses_channel.priv_sock,
			inet_ntoa(((struct sockaddr_in*)msg_name)->sin_addr),
			ntohs(((struct sockaddr_in*)msg_name)->sin_port),
			hlen, len))
#else /* UNIX */
			PRINT_OUT((mcl_stdout,
			"sendmsg: priv_sock=%d, dst=%s/%d, hdr_len=%d, payload_len=%d\n",
			mclcb->ses_channel.priv_sock,
			inet_ntoa(((struct sockaddr_in*)(msg.msg_name))->sin_addr),
			ntohs(((struct sockaddr_in*)(msg.msg_name))->sin_port),
			hlen, len))
#endif /* OS_DEP */
		}
#ifdef SIMUL_TX_LOSSES
	} else {
		/* lost... */
		TRACELVL(3, (mcl_stdout, "  mcl_ses_channel:send_pkt: loss simulated\n"))
	}
#endif /* SIMUL_TX_LOSSES */

	TRACELVL(5, (mcl_stdout, "<- mcl_ses_channel::send_pkt:\n"))
	return MCL_OK;
}


/**
 * Receive a data or control packet on this channel.
 * @param mclcb
 * @return Completion status (MCL_OK or MCL_ERROR).
 */
mcl_error_status
mcl_ses_channel::recv_pkt (mcl_cb *const mclcb)
{
	mcl_rx_pkt	*pkt;		// recv'd packet buffer
	struct sockaddr	saddr;		// buffer for src addr
	socklen_t	saddr_len;	// buffer for src addr len
	mcl_addr	addr;		// class for src addr
	INT32		n;
	fd_set		tmp_fds;
	struct timeval	tv;		// don't wait indefinitely in select

	TRACELVL(5, (mcl_stdout, "-> mcl_ses_channel::recv_pkt:\n"))
again:
	/*
	 * wait to receive packets... a select on several fd is required!
	 */
	if (this->n_fd == 0) {
		/*
		 * nothing to select; wait a bit and return...
		 */
		mclcb->unlock();
#ifndef WIN32
		pthread_testcancel();
#endif
		mcl_usleep(DFLT_POLLING_PERIOD);
#ifndef WIN32
		pthread_testcancel();
#endif
		mclcb->lock();
		TRACELVL(5, (mcl_stdout, "<- mcl_ses_channel::recv_pkt: no fd to listen\n"))
		return MCL_OK;
	}
	tmp_fds = this->fds;
	tv.tv_sec = 1;	/* wait at most 1s to have opportunity to update fds */
	tv.tv_usec = 0;
	mclcb->unlock();
#ifndef WIN32
	pthread_testcancel();
#endif
	if ((n = select(this->nfds, &tmp_fds,  NULL, NULL, &tv)) < 0) {
		/*
		 * in practice always try again, without trying to
		 * differentiate on type of error (usually select has
		 * been interrupted...)
		 */
		mclcb->lock();
		goto again;	
	}
#ifndef WIN32
	pthread_testcancel();
#endif
	mclcb->lock();
	if (n == 0) {
		/* nothing received */
		TRACELVL(5, (mcl_stdout, "<- mcl_ses_channel::recv_pkt: timeout\n"))
		return MCL_OK;
	}
	/*
	 * read the packet(s) received (there may be more than one!)
	 */
	//if (n>0) printf("select returned n=%d\n", n);
	//if (this->ses_sock > 0 && FD_ISSET(this->ses_sock, &tmp_fds)) {
	/* XXX: doing so, a source ignores mcast packets sent by receivers!
	 * Change it when the filtering problem is solved */
	if (mclcb->is_a_receiver() &&
	    this->ses_sock > 0 && FD_ISSET(this->ses_sock, &tmp_fds)) {
		while (this->ses_sock > 0) {
			pkt = new mcl_rx_pkt(mclcb->get_max_datagram_size());
			ASSERT(pkt);
			saddr_len = sizeof(saddr);
			memset(&saddr, 0, sizeof(saddr));
			if ((pkt->pkt_len = recvfrom(this->ses_sock,
					pkt->get_buf(), pkt->get_buf_len(),
					0, &saddr,
					&saddr_len)) < 0) {
				/* we are in non-blocking mode! */
#ifdef SOLARIS	/* cannot check errno reliably on Solaris! don't know why!!! */
				/* XXX: assume no ready packet anymore*/
				delete pkt;
				break;

#elif defined(WIN32)
				if (WSAGetLastError() == WSAEWOULDBLOCK) {
					/* no ready packet anymore */
					delete pkt;
					break;
				}
#else /* LINUX */
				if (errno == EAGAIN || errno == EWOULDBLOCK) {
					/* no ready packet anymore */
					delete pkt;
					break;
				}
#endif /* OS */

#ifdef WIN32
				TRACELVL(5, (mcl_stdout, "   mcl_ses_channel::recv_pkt: ERROR, ses_sock=%d, error=%d\n", this->ses_sock, WSAGetLastError()))
#else  /* UNIX */
				TRACELVL(5, (mcl_stdout, "   mcl_ses_channel::recv_pkt: ERROR, ses_sock=%d, error=%d\n", this->ses_sock, errno))
#endif
				perror("mcl_ses_channel::recv_pkt: recvfrom");
				mcl_exit(1);
			}
			addr.set_addr_struct((struct sockaddr_in*)&saddr);
#if 0
			/* drop packets that we sent ourselves! */
			if (addr.is_equal(mclcb->ses_channel.priv_addr)) {
				TRACELVL(5, (mcl_stdout,
				"   mcl_ses_channel::recv_pkt: dropped (sent by us)\n"))
				delete pkt;
				break;
			}
#endif
			/* and finally process the packet */
			mclcb->rx.process_pkt(mclcb, pkt, &addr);
			/*
			 * try again...
			 */
			break;
		}
	}
	if (this->priv_sock > 0 && FD_ISSET(this->priv_sock, &tmp_fds)) {
		while (this->ses_sock > 0) {
			pkt = new mcl_rx_pkt(mclcb->get_max_datagram_size());
			ASSERT(pkt);
			saddr_len = sizeof(saddr);
			memset(&saddr, 0, sizeof(saddr));
			if ((pkt->pkt_len = recvfrom(this->priv_sock,
					pkt->get_buf(), pkt->get_buf_len(),
					0, &saddr,
					&saddr_len)) < 0) {


				/* we are in non-blocking mode! */
#ifdef SOLARIS	/* cannot check errno reliably on Solaris! don't know why!!! */
				/* XXX: assume no ready packet anymore*/
				delete pkt;
				break;

#elif defined(WIN32)
				if (WSAGetLastError() == WSAEWOULDBLOCK) {
					/* no ready packet anymore */
					delete pkt;
					break;
				}
#else /* LINUX */
				if (errno == EAGAIN || errno == EWOULDBLOCK) {
					/* no ready packet anymore */
					delete pkt;
					break;
				}
#endif /* OS */

#ifdef WIN32
				TRACELVL(5, (mcl_stdout, "   mcl_ses_channel::recv_pkt: ERROR, priv_sock=%d, error=%d\n", this->priv_sock, WSAGetLastError()))
#else  /* UNIX */
				TRACELVL(5, (mcl_stdout, "   mcl_ses_channel::recv_pkt: ERROR, priv_sock=%d, error=%d\n", this->priv_sock, errno))
#endif
				perror("mcl_ses_channel::recv_pkt: recvfrom");
				mcl_exit(1);
			}
			addr.set_addr_struct((struct sockaddr_in*)&saddr);
			/* and finally process the packet */
			mclcb->rx.process_pkt(mclcb, pkt, &addr);
			/*
			 * try again...
			 */
			break;
		}
	}
	TRACELVL(5, (mcl_stdout, "<- mcl_ses_channel::recv_pkt: \n"))
	return MCL_OK;
}


#if defined(SIMUL_TX_LOSSES) || defined(SIMUL_RX_LOSSES)
/**
 * Simulate packets losses randomly
 * Returns false if OK, true if packet should be lost...
 */
bool
mcl_ses_channel::sim_random_loss(mcl_cb		*const mclcb)
{
	bool	is_lost = false;

	TRACELVL(5, (mcl_stdout, "-> mcl_ses_channel::sim_random_loss:\n"))
	switch (this->simul_losses_state) {
	case 0: /* last packet was sent OK. */
#ifdef CONSTANT_LOSS_RATIO
		if ((float)(random()%100) < (float)P_LOSS_WHEN_OK) {
#else
		TODO...
#endif
			is_lost = true;
			this->simul_losses_state = 1;
		}
		break;
	case 1: /* last packet was lost */
#ifdef CONSTANT_LOSS_RATIO
		if ((float)(random()%100) < (float)P_LOSS_WHEN_LOSSES) {
#else
		TODO...
#endif
			is_lost = true;
		}
		else
			this->simul_losses_state = 0;
		break;
	default:
		perror("mcl_ses_channel::sim_random_loss:: unknown state");
		mcl_exit(1);
		break;
	}
	TRACELVL(5, (mcl_stdout, "<- mcl_ses_channel::sim_random_loss: %s\n",
		(is_lost ? "lost" : "ok")))
	if (is_lost)
		mclcb->stats.tx_simul_loss_lost++;
	else
		mclcb->stats.tx_simul_loss_sent++;
	return is_lost;
}
#endif /* SIMUL_XX_LOSSES */


bool
mcl_ses_channel::is_mcast_session()
{
	if (this->is_mcast < 0) {
		if (this->ses_addr.is_multicast_addr()) {
			this->is_mcast = 1;
		} else {
			this->is_mcast = 0;
		}
	}
	return (bool)this->is_mcast;
}


/*
 * Performs the socket initialization work
 * @param mclcb
 * @param saddr	session address (unicast or mcast)
 */
mcl_error_status
mcl_ses_channel::sock_init (mcl_cb		*const mclcb)
{
	/* solaris seems to care about the int/char type, not linux! */
#ifdef WIN32
	UINT32			mcast_ttl;	/* for setsockopt */
//	UINT8			mcast_loop;
#else
	UINT8			mcast_ttl;	/* for setsockopt */
	UINT8			mcast_loop;
#endif /* OS_DEP */

	UINT32			reuse_addr;	/* for setsockopt */
	struct ip_mreq		imr;		/* ... to join mcast group */
	struct in_addr		if_addr;	/* interface IP address */
	struct sockaddr_in	tmp_addr;	/* temporary addr */
	socklen_t		tmp_len;	/* temporary addr length */
	INT32			sock_size;	/* socket size */
	INT32			mode = 0;	// tx/rx mode

	TRACELVL(5, (mcl_stdout, "-> mcl_ses_channel::sock_init:\n"))
	/*
	 * Step 1: determine the exact mode.
	 */
	if (mclcb->is_a_sender()) {
		mode |= MODE_SIG_UNI_TX;
		mode |= MODE_SIG_UNI_RX;
		if (this->ses_addr.is_multicast_addr()) {
			mode |= MODE_MCAST_TX;
			mode |= MODE_SIG_MCAST_TX;
			mode |= MODE_SIG_MCAST_RX;
		} else {
			mode |= MODE_UNI_TX;
		}
	}
	if (mclcb->is_a_receiver()) {
		mode |= MODE_SIG_UNI_RX;
		mode |= MODE_SIG_UNI_TX;
		if (this->ses_addr.is_multicast_addr()) {
			mode |= MODE_MCAST_RX;
			mode |= MODE_SIG_MCAST_TX;
			mode |= MODE_SIG_MCAST_RX;
		} else {
			mode |= MODE_UNI_RX;
		}
	}

	if (this->mcast_if_addr)
		if_addr.s_addr = htonl(this->mcast_if_addr->get_addr());
	else
		if_addr.s_addr = htonl(INADDR_ANY);

	/*
	 * Step 2: initialize the private socket
	 */
	if ((mode & MODE_UNI_TX) || (mode & MODE_MCAST_TX) ||
	    (mode & MODE_SIG_UNI_TX) || (mode & MODE_SIG_UNI_RX) ||
	    (mode & MODE_SIG_MCAST_TX)) {
		this->can_tx = 1;
		/*
		 * private socket is always used for tx.
		 * required both by mcast traffic source and receivers
		 * (signaling)
		 */
		if ((this->priv_sock = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
			perror("mcl_ses_channel::sock_init: socket");
			goto error;
		}
		if (mode & MODE_SIG_UNI_RX) {
			/*
			 * priv_sock is also used for rx of signaling.
			 * This is required both by the sender (who receives
			 * unicast signaling)
			 * and by receivers (who also receive unicast responses
			 * from the sender)
			 */
			this->can_rx = 1;
			/*
			 * For the future select().
			 */
			FD_SET((UINT32)this->priv_sock, &(this->fds));
			this->nfds = max(this->nfds, this->priv_sock + 1);
			this->n_fd++;
		}
		/*
		 * bind with 0 to get a locally unique port number
		 */
		tmp_addr.sin_family = AF_INET;
		tmp_addr.sin_port = htons(0);  
		tmp_addr.sin_addr.s_addr = htonl(INADDR_ANY);
#ifdef WIN32
		if (bind(this->priv_sock, (struct sockaddr FAR *)&tmp_addr,
			  sizeof (tmp_addr)) == SOCKET_ERROR)
#else  /* UNIX */
		if (bind(this->priv_sock, (struct sockaddr *)&tmp_addr,
			  sizeof(tmp_addr)) < 0)
#endif /* WIN32 */
		{
			perror("mcl_ses_channel::sock_init: bind() Error");
			goto error;
		}
	}

	if ((mode & MODE_MCAST_TX) || (mode & MODE_SIG_MCAST_TX)) {
		if (this->mcast_if_addr) {
			/* specify multicast interface */
			if (setsockopt(this->priv_sock, IPPROTO_IP,
					IP_MULTICAST_IF, 
			    		(char*)&if_addr.s_addr,
					sizeof(if_addr.s_addr)) < 0) {
				perror("mcl_ses_channel::sock_init (sender): IP_MULTICAST_IF");
				goto error;
			}
		}
		/* specify ttl */
		mcast_ttl = min(this->ttl, 255);
		if (setsockopt(this->priv_sock, IPPROTO_IP, IP_MULTICAST_TTL, 
		    (char *)&mcast_ttl, sizeof(mcast_ttl)) < 0 ) {
			perror("mcl_ses_channel::sock_init (sender): IP_MULTICAST_TTL");
			goto error;
		}
#ifndef WIN32
		/*
		 * turn on the loop back of multicast packets to enable
		 * receivers local to this host to get data
		 */
		mcast_loop = 1;
		if (setsockopt(this->priv_sock, IPPROTO_IP, IP_MULTICAST_LOOP, 
		    (char *)&mcast_loop, sizeof(mcast_loop)) != 0) {
			perror("mcl_ses_channel::sock_init (sender): IP_MULTICAST_LOOP");
			goto error;
		}
#endif /* NOT WIN32 */
	}
	/*
	 * finally store in priv_addr the allocated local addr/port number
	 * of priv_sock
	 */
	memset(&tmp_addr, 0, sizeof(tmp_addr));
	tmp_len = sizeof(tmp_addr);
	if (this->priv_sock > 0 &&
	    getsockname(this->priv_sock, (struct sockaddr *)&tmp_addr,
			&tmp_len) < 0) {
		perror("mcl_ses_channel::sock_init: getsockname");
		goto error;
	}
	if (this->mcast_if_addr && (this->mcast_if_addr->get_addr() != 0)) {
		// use this addr since we are on a multihomed host and
		// mcast_if the interface being used...
		this->priv_addr = *(this->mcast_if_addr);
	} else {
		struct in_addr a;
		// XXX: TODO: find default if addr... (in the meantime use 0)
		if (inet_aton("0.0.0.0", &a) < 0)
			perror("mcl_ses_channel::sock_init: inet_aton");
		this->priv_addr.set_addr(ntohl(a.s_addr));
	}
	// tmp_addr contains the local port number...
	this->priv_addr.set_port(ntohs(tmp_addr.sin_port));
	// and mcast_if contains the local address
	//this->priv_addr.set_addr(this->mcast_if.get_addr());

	/*
	 * Step 3: initialize the session socket
	 */
	if ((mode & MODE_UNI_RX) || (mode & MODE_MCAST_RX) ||
	    (mode & MODE_SIG_MCAST_RX)) {
		/* session socket is always used for mcast rx */
		if ((this->ses_sock = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
			perror("mcl_ses_channel::sock_init: socket");
			goto error;
		}
		/* set non-blocking mode for future read on the session sock */
#ifdef WIN32
		{
		unsigned long NonBlocking = 1;
		if (ioctlsocket(mg->ses_sock, FIONBIO, &NonBlocking) == SOCKET_ERROR ) {
			perror("mcl_ses_channel::sock_init: fcntl");
			goto error;
		}
		}
#else
		if (fcntl(this->ses_sock, F_SETFL, O_NONBLOCK) < 0) {
			perror("mcl_ses_channel::sock_init: fcntl");
			goto error;
		}
#endif
		/* increase the default socket size */
		sock_size = this->socket_size;
		if (setsockopt(this->ses_sock, SOL_SOCKET, SO_RCVBUF, (char*)&sock_size, sizeof(sock_size)) < 0) {
			perror("mcl_ses_channel::sock_init: SO_RCVBUF");
			goto error;
		}
		/* for the future select() */
		if (mclcb->is_a_receiver()) {
			/* XXX: doing so, a source ignores mcast packets sent
			 * by receivers!
			 * Change it when the filtering problem is solved */
			FD_SET((UINT32)this->ses_sock, &(this->fds));
			this->nfds = max(this->nfds, this->ses_sock + 1);
			this->n_fd++;
		}

		/* allow for reuse of the couple addr/port */
		reuse_addr = 1;
		if (this->is_mcast_session() &&
		    setsockopt(this->ses_sock, SOL_SOCKET, SO_REUSEADDR,
			       (char *)&reuse_addr, sizeof(reuse_addr)) < 0) {
			perror("mcl_ses_channel::sock_init (recv): REUSEADDR");
			goto error;
		}
		/* now bind the session port number (with INADDR_ANY addr) */
		// memcpy(&tmp_addr, saddr, sizeof(*saddr));
		tmp_addr.sin_family = AF_INET;
		tmp_addr.sin_port = htons(this->ses_addr.get_port());
		tmp_addr.sin_addr.s_addr = htonl(INADDR_ANY);
		if (bind(this->ses_sock, (struct sockaddr*)&tmp_addr,
		    sizeof(tmp_addr)) < 0) {
			perror("mcl_ses_channel::sock_init (recv): bind");
			PRINT_ERR((mcl_stderr,
				"mcl_ses_channel::sock_init (recv): ERROR; bind to %s/%d failed\n",
				inet_ntoa(tmp_addr.sin_addr),
				ntohs(tmp_addr.sin_port)))
			goto error;
		}
	}

	if ((mode & MODE_MCAST_RX) || (mode & MODE_SIG_MCAST_RX)) {
		if (this->mcast_if_addr) {
			/* specify multicast interface */
			if (setsockopt(this->ses_sock, IPPROTO_IP,
					IP_MULTICAST_IF, 
			    		(char*)&if_addr.s_addr,
					sizeof(if_addr.s_addr)) < 0) {
				perror("mcl_ses_channel::sock_init (recv): IP_MULTICAST_IF");
				goto error;
			}
		}
#if 0 // USELESS
		/* loop back multicast packets */

#ifndef WIN32
		mcast_loop = 1;
		if (setsockopt(this->ses_sock, IPPROTO_IP, IP_MULTICAST_LOOP, 
		    (char *)&mcast_loop, sizeof(mcast_loop)) != 0) {
			perror("mcl_ses_channel::sock_init (recv): IP_MULTICAST_LOOP");
			goto error;
		}
#endif /* NOT WIN32 */
#endif

		/* multicast join */
		imr.imr_multiaddr.s_addr = htonl(this->ses_addr.get_addr());
		imr.imr_interface.s_addr = if_addr.s_addr;

		if (setsockopt(this->ses_sock, IPPROTO_IP, IP_ADD_MEMBERSHIP,
		    (char *)&imr, sizeof(imr)) < 0) {
			perror("mcl_ses_channel::sock_init (recv): IP_ADD_MEMBERSHIP");
			goto error;
		}
	}

	if (mclcb->get_verbosity() >= 4) {
		// NB: use several print_out since get_addr_string() (and
		// more fondamentally inet_ntoa()) is not reentrant!
		PRINT_OUT((mcl_stdout,
		"mcl_ses_channel::sock_init: ses_sock=%d (%s/%d),",
			this->ses_sock, this->ses_addr.get_addr_string(),
			this->ses_addr.get_port()))
		PRINT_OUT((mcl_stdout, " priv_sock=%d (%s/%d)\n",
			this->priv_sock, this->priv_addr.get_addr_string(),
			this->priv_addr.get_port()))
	}
	TRACELVL(5, (mcl_stdout, "<- mcl_ses_channel::sock_init:\n"))
	return MCL_OK;

error:
	TRACELVL(5, (mcl_stdout, "<- mcl_ses_channel::sock_init: failed\n"))
	return MCL_ERROR;
}


/**
 * Close all the remaining sockets.
 */
mcl_error_status
mcl_ses_channel::sock_close (mcl_cb	*const mclcb)
{
	if ((this->priv_sock > 0 && close(this->priv_sock)) < 0) {
		perror("mcl_ses_channel::sock_close: close on priv_sock");
		goto error;
	}
	if ((this->ses_sock > 0 && close(this->ses_sock)) < 0) {
		perror("mcl_ses_channel::sock_close: close on ses_sock");
		goto error;
	}
	TRACELVL(5, (mcl_stdout, "<- mcl_ses_channel::sock_close:\n"))
	return MCL_OK;

error:
	TRACELVL(5, (mcl_stdout, "<- mcl_ses_channel::sock_close: failed\n"))
	return MCL_ERROR;
}

