/* $Id: mcl_network.cpp,v 1.30 2005/03/18 12:06:17 roca Exp $ */
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
 *	Network functions (lower socket interface).
 */

#include "mcl_includes.h"


#ifdef SIMUL_LOSSES
/*
 * Simulate packets losses randomly
 * Returns 0 if OK, 1 if packet should be lost...
 */
static int RandomLoss(mcl_cb	*mclcb,
		      int	layer)
{
	int IsLost = 0;

	TRACELVL(5, (mcl_stdout, "-> RandomLoss\n"))
	switch ( mclcb->simul_losses_state ) {
	case 0: /* last packet was sent OK. */
#ifdef CONSTANT_LOSS_RATIO
		if ((float)(random()%100) < (float)P_LOSS_WHEN_OK)
#else
		if ((float)(random()%100) < (P_LOSS_WHEN_OK*(float)layer))
#endif
		{
			IsLost = 1;
			mclcb->simul_losses_state = 1;
		}
		break;
	case 1: /* last packet was lost */
#ifdef CONSTANT_LOSS_RATIO
		if ((float)(random()%100) < (float)P_LOSS_WHEN_LOSSES)
#else
		if ((float)(random()%100) < (P_LOSS_WHEN_LOSSES*(float)layer))
#endif
		{
			IsLost = 1;
		}
		else
			mclcb->simul_losses_state = 0;
		break;
	default:
		perror("RandomLoss: unknown state");
		mcl_exit(1);
		break;
	}
	TRACELVL(5, (mcl_stdout, "<- RandomLoss (layer:%d, IsLost:%d)\n", layer, IsLost))
	if (IsLost)
		mclcb->stats.tx_simul_loss_lost++;
	else
		mclcb->stats.tx_simul_loss_sent++;
	return IsLost;
}
#endif /* SIMUL_LOSSES */



/*
 * Create and send a packet (with/without data and/or SIG) immediately
 * Returns 0 if OK, < 0 otherwise.
 */
int
mcl_send_pkt (mcl_cb	*mclcb,
	      int	layer,
	      du_t	*du,
	      adu_t	*adu)
{
	block_t		*blk;
	mcl_socket	*so;
#define PBUF_LEN	(MAX_DATAGRAM_SIZE)
				/* DU (hdr+data) cant be larger than PBUF_LEN */
	char 		buf[PBUF_LEN];
	fixed_lct_hdr_t	*lct_hdr;	/* fixed size LCT header */
	hdr_infos_t	hdr_infos;	/* struct given to LCT creation funcs */
	int		hlen;		/* total (RLC/FLIDS+LCT) header length*/
	int		len;
	int		sig_nb;		/* SIG # sent in this packet */
	MCL_IOVEC	iov[2];		/* to describe header + data */
#ifdef WIN32
	struct sockaddr	*msg_name;
	UINT32	msg_namelen;
	DWORD nb_sent=0;
#else
	struct msghdr	msg;		/* for the sendmsg() Socket syscall */
#endif

	TRACELVL(5, (mcl_stdout, "-> mcl_send_pkt: layer=%d, du=x%x, adu=x%x\n",
		layer, (int)du, (int)adu))
	/* prepare for new sig copies */
	so  = mclcb->txlay_tab[layer].socket;
	/* reset hdr_infos */
	memset(&hdr_infos, 0, sizeof(hdr_infos));
	hdr_infos.TOI_present	= hdr_infos.FPI_present
				= hdr_infos.FDT_present
				= hdr_infos.FTI_present
				= hdr_infos.NONEWADU_present
				= false;
#ifdef METAOBJECT_USED
	hdr_infos.MODT_present = false;
#endif
	if (du) {
		ASSERT(adu);		/* also required in that case */
		blk = du->block;
		ASSERT(blk);
		hdr_infos.TOI_present = true;
		hdr_infos.idf_adu   = adu->seq;
		hdr_infos.idf_block = blk->seq;
		hdr_infos.idf_du    = du->seq;
#ifdef FEC
		hdr_infos.is_fec    = du->is_fec;
#endif /* FEC */
		/* ADU seq == 0 sometimes require special handling... */
		if (adu->seq == 0) {
		       if (mclcb->is_flute_compatible() == true) {
				/* it's the file delivery table, FDT */
				hdr_infos.FDT_present = true;
				hdr_infos.FDT_instanceid = adu->FDT_instanceid;

			} 
#ifdef METAOBJECT_USED			
			else if (mclcb->meta_obj_layer->in_use() == true) {
				/* it's the meta object descr table, MODT */
				hdr_infos.MODT_present = true;
				hdr_infos.FDT_instanceid = adu->FDT_instanceid;
			}
#endif
		}
		hdr_infos.FPI_present = true;	/* add FPI (required if data) */
		hdr_infos.demux_label = mclcb->demux_label;
		hdr_infos.fec_encoding_id = adu->fec_encoding_id;
		hdr_infos.fec_instance_id = adu->fec_instance_id;
#if 0
		/* don't add any FTI HE with RSE for packets sent on layer 3
		 * and above */
		if (adu->fec_instance_id == FEC_INSTANCE_RSE && layer >= 3) {
			hdr_infos.FTI_present = false;
		} else {
#endif
			hdr_infos.FTI_present = true;
			hdr_infos.adu_len     = adu->len;
			hdr_infos.k           = blk->k;	/* current blk */
			hdr_infos.max_k       = adu->max_k;
			hdr_infos.max_n	      = adu->max_n;
			hdr_infos.symbol_len  = adu->symbol_len;
#ifdef LDPC_FEC
			hdr_infos.fec_key     = blk->fec_key;
#endif
		/*}*/
		len = du->len;
	} else {

		/* add some required flags, even if there is no data */
		hdr_infos.demux_label= mclcb->demux_label;
		hdr_infos.fec_encoding_id = FEC_ENCODING_ID_NO_FEC;/* no FEC */
		hdr_infos.fec_instance_id = 0;			   /* no FEC */
		len = 0;
	}

	/*
	 * Prepar signaling header extensions to be added to the LCT header.
	 * There is enough room for 16 bytes of additional signaling header
	 * extensions (in particular for NONEWADU).
	 */
	CopySigReset(mclcb);
	sig_nb = CopySigToLCTinfos(mclcb, layer, &hdr_infos, 16);
	CleanupSigTab(mclcb);
	/*
	 * create the ALC/LCT headers now
	 * NB: hlen includes both ALC, LCT and RLC/FLIDS headers!
	 */
	lct_hdr = (fixed_lct_hdr_t*)buf;
	hlen = alc_hdr_create(mclcb, lct_hdr, (hdr_infos_t*)&hdr_infos);
	if (hlen < 0)
		goto bad;
	ASSERT(hlen <= MAX_ALC_HEADER_SIZE); /* make sure... */
	/*
	 * create the appropriate congestion control header
	 */
	if (mclcb->congestion_control == NO_CC) {
		/*
		 * In no cogestion control mode there is no CC header.
		 * Required for FLUTE interoperability tests, but a limitation
		 * is that no loss statistics are possible
		 */
		lct_hdr->cci = 0;
	}
#ifdef RLC 
	else if (mclcb->congestion_control == RLC_CC) {
		if (rlc_tx_fill_header(mclcb, (rlc_hdr_t*)&(lct_hdr->cci),
					(UINT8) layer) != MCL_OK)
			goto bad;
#endif
#ifdef FLIDS
	} else if (mclcb->congestion_control == FLID_SL_CC) {
		if (FLIDs_tx_FillHeader(mclcb, (flids_hdr_t*)&(lct_hdr->cci),
					(UINT8)layer ) != MCL_OK)
			goto bad;
	}
#endif
	/*
	 * update traces and statistics
	 */
	if (mclcb->get_verbosity() >= 2) {
		mcl_print_sent_du(mclcb, 0, layer, &hdr_infos);
		if (mclcb->get_verbosity() >= 5) {
			/* header and data are in two diff buffers */
			mcl_dump_buffer(buf, hlen, (hlen>> 2));
			/*mcl_dump_buffer(du->data, len, (len >> 2));*/
		}
	}
	if (len > 0) {
		if (hdr_infos.is_fec) {
			mclcb->stats.tx_fec_pkts++; 
			mclcb->stats.tx_fec_bytes += len;
		} else {
			mclcb->stats.tx_pkts++;
			mclcb->stats.tx_bytes += len;
		}
		mclcb->stats.tx_pkts_per_lvl[layer]++;
		mclcb->stats.tx_bytes_per_lvl[layer] += len;
	} 
	mclcb->stats.tx_totbytes += hlen + len;
	/*
	 * and now send the packet
	 */
	MCL_IOV_BUFF(iov[0]) = buf;
	MCL_IOV_LEN(iov[0])  = hlen;
	if (len > 0) {
#ifdef VIRTUAL_TX_MEM
		/* make sure data is in phy mem (perhaps in VTM cache) first */
		mcl_vtm_get_data(mclcb, du);
		ASSERT(du->data);
#endif /* VIRTUAL_TX_MEM */
		MCL_IOV_BUFF(iov[1]) = du->data;
		MCL_IOV_LEN(iov[1])  = len;
	} else {
		/* security */
		MCL_IOV_BUFF(iov[1]) = NULL;
                MCL_IOV_LEN(iov[1]) = 0;
	}

#ifdef WIN32
	if (adu && adu->addr_valid) {
		/* use this dest addr */
		msg_name = (struct sockaddr*) adu->addr.get_internal_struct_addr();
		msg_namelen = adu->addr.get_addr_struct_len();
	} else {
		/* use the default dest addr */
		msg_name = (struct sockaddr*) so->addr.get_internal_struct_addr();
		msg_namelen = so->addr.get_addr_struct_len();
	}
#else /* UNIX */
	memset(&msg, 0, sizeof(msg));
	if (adu && adu->addr_valid) {
		/* use this dest addr */
		msg.msg_name = (char*)(adu->addr.get_internal_struct_addr());
		msg.msg_namelen = adu->addr.get_addr_struct_len();
	} else {
		/* use the default dest addr */
		msg.msg_name = (char*)(so->addr.get_internal_struct_addr());
		msg.msg_namelen = so->addr.get_addr_struct_len();
	}
	msg.msg_iov = iov;
	msg.msg_iovlen = (len > 0 ? 2 : 1);
#endif /* OS_DEP */

#ifdef SIMUL_LOSSES
	if (!RandomLoss(mclcb,layer)) {
#endif /* SIMUL_LOSSES */
#ifdef WIN32
		if (WSASendTo(so->priv_sock, iov, (len > 0 ? 2 : 1), &nb_sent, 0,
			      msg_name, msg_namelen, NULL, NULL) == SOCKET_ERROR) {
			PRINT_ERR((mcl_stderr,"mcl_send_pkt: WSASendTo (code:%d - sock:%d)\n", WSAGetLastError(), so->priv_sock))
			mcl_exit(1);
		}
#else /* UNIX */
		if (sendmsg(so->priv_sock, &msg, 0) != hlen + len) {
			perror("mcl_send_pkt: sendmsg");
			PRINT_ERR((mcl_stderr,
				"mcl_send_pkt: ERROR, sendmsg failed; priv_sock=%d, family=%d, dst=%s/%d, hlen=%d, len=%d\n",
				so->priv_sock,
				((struct sockaddr_in*)(msg.msg_name))->sin_family,
				/* TODO: the following line is only for IPv4! */
				inet_ntoa(((struct sockaddr_in*)(msg.msg_name))->sin_addr),
				ntohs(((struct sockaddr_in*)(msg.msg_name))->sin_port),
				hlen, len))

			mcl_exit(1);
		}
#endif /* OS_DEP */
		if (mclcb->get_verbosity() >= 4) {
#ifdef WIN32
			PRINT_OUT((mcl_stdout,
			"sendmsg: priv_sock=%d, family=%d, dst=%s/%d, hlen=%d, len=%d\n",
			(int)so->priv_sock,
			((struct sockaddr_in*)msg_name)->sin_family,
			inet_ntoa(((struct sockaddr_in*)msg_name)->sin_addr),
			ntohs(((struct sockaddr_in*)msg_name)->sin_port),
			hlen, len))
#else /* UNIX */
			PRINT_OUT((mcl_stdout,
			"sendmsg: priv_sock=%d, family=%d, dst=%s/%d, hlen=%d, len=%d\n",
			so->priv_sock,
			((struct sockaddr_in*)(msg.msg_name))->sin_family,
			inet_ntoa(((struct sockaddr_in*)(msg.msg_name))->sin_addr),
			ntohs(((struct sockaddr_in*)(msg.msg_name))->sin_port),
			hlen, len))
#endif /* OS_DEP */
		}
#ifdef SIMUL_LOSSES
	} else {
		TRACELVL(3, (mcl_stdout, "=> Random Loss on layer %d\n", layer))
	}
#endif /* SIMUL_LOSSES */
	//len = 0;	/* no data to send next time */
	TRACELVL(5, (mcl_stdout, "<- mcl_send_pkt:\n"))
	return 0;

bad:
	TRACELVL(5, (mcl_stdout, "<- mcl_send_pkt: ERROR\n"))
	return -1;
#undef PBUF_LEN
}


/*
 * Receive a new packet
 * Returns the packet len if ok, -1 if error
 */
void
mcl_recv_pkt (mcl_cb	*mclcb)
{
	/*
	 * using MAX_ETHERNET_SIZE is usefull if the datagram
	 * size may change during the session or it is unknown
	 */
	//#define PBUF_LEN	(mclcb->payload_size + MAX_ALC_HEADER_SIZE)
#define PBUF_LEN	(MAX_ETHERNET_MTU - UDP_IPv4_HEADER_SIZE)

	mcl_rx_pkt	*pkt;		// recv'd packet buffer
	struct sockaddr_storage	saddr;	/* buffer for src addr or NULL */
	int		saddr_len;	/* src addr len */
	class mcl_addr	addr;
	mcl_socket	*so;
	int		n;
	int		lay;
	fd_set		tmp_fds;
	struct timeval	tv;		/* don't wait indefinitely in select */

	TRACELVL(5, (mcl_stdout, "-> mcl_recv_pkt:\n"))
again:
	/*
	 * wait to receive packets... a select on several fd is required!
	 */
	if (mclcb->rxlvl.n_fd == 0) {
		/* nothing to select, return! */
		return;
	}
	tmp_fds = mclcb->rxlvl.fds;
	tv.tv_sec = 1;	/* wait at most 1s to have opportunity to update fds */
	tv.tv_usec = 0;
	mclcb->unlock();
	mcl_thread_testcancel(mclcb);
	if ((n = select(mclcb->rxlvl.nfds, &tmp_fds,  NULL, NULL, &tv)) < 0) {
		/* always try again in practice */
		mclcb->lock();
		goto again;
	}
	mcl_thread_testcancel(mclcb);
	mclcb->lock();
	if (n == 0) {
		/* nothing received */
		TRACELVL(5, (mcl_stdout, "<- mcl_recv_pkt: timeout\n"))
		return;
	}
	/*
	 * read the packet(s) received (there may be more than one!)
	 */
	//if (n > 0) PRINT_OUT((mcl_stdout, "select returned n=%d\n", n))
	for (lay = 0, so = mclcb->socket_tab;
	     lay <  mclcb->mcl_max_group;
	     lay++, so++) {
		if (mcl_is_valid_sock(so->ses_sock) &&
		    FD_ISSET((int)so->ses_sock, &tmp_fds)) {
			while (mcl_is_valid_sock(so->ses_sock)) {
				pkt = new mcl_rx_pkt(PBUF_LEN);
				ASSERT(pkt);
				saddr_len = sizeof(saddr);
				memset(&saddr, 0, sizeof(saddr));
				pkt->pkt_len = recvfrom(so->ses_sock,
					pkt->get_buf(), pkt->get_buf_len(),
					0, (sockaddr *) &saddr,
#if defined(LINUX) || defined(AIX)
					(size_t*)
#elif defined(FREEBSD)
					(socklen_t*)
#endif
					&saddr_len);
#ifdef WIN32
				if(pkt->pkt_len == SOCKET_ERROR || pkt->pkt_len == 0) {
					/* we are in non-blocking mode! */
					if (WSAGetLastError() == WSAEWOULDBLOCK || WSAGetLastError() == WSAENOTSOCK) {
						/* no ready packet anymore */
						delete pkt;
						break;
					} /* else exit... */
					PRINT_ERR((mcl_stderr, "mcl_recv_pkt: ERROR, ses_sock=%d, error=%d\n", so->ses_sock, WSAGetLastError()))
					mcl_exit(1);
				}
#elif defined(SOLARIS)
				if (pkt->pkt_len < 0) {
					/* we are in non-blocking mode! */
					/* I cannot check errno reliably on
					 * Solaris but I don't know why! */
					/* XXX: assume no ready packet anymore*/
					delete pkt;
					break;
				}
#else /* LINUX */
				if (pkt->pkt_len < 0) {
					/* we are in non-blocking mode! */
					if (errno == EAGAIN || errno == EWOULDBLOCK) {
						/* no ready packet anymore */
						delete pkt;
						break;
					} /* else exit... */
					TRACELVL(5, (mcl_stdout, "mcl_recv_pkt: ERROR, ses_sock=%d, error=%d\n", so->ses_sock, errno))
					perror("mcl_recv_pkt: recvfrom");
					mcl_exit(1);
				}
#endif /* OSDEP */
				if (saddr.ss_family == AF_INET)
				{
					addr.set_addr_struct((struct sockaddr_in*)&saddr);
				}
#ifdef INET6
				else if (saddr.ss_family == AF_INET6)
				{
					addr.set_addr_struct((struct sockaddr_in6*)&saddr);
				}
#endif
				/* and finally process the packet */
				mcl_process_pkt (mclcb, pkt, &addr, lay);

#if 0				/* removed to catch duplicated pkt, see above */
				/*
				 * try again only on layers >= 2 where we know
				 * we can receive more than 2 pkts per cycle
				 */
				if (lay <= 1)
					break;
#endif
			}
		}
		mclcb->unlock();
		mcl_thread_testcancel(mclcb);
		mclcb->lock();
	}
	TRACELVL(5, (mcl_stdout, "<- mcl_recv_pkt: \n"))
#undef PBUF_LEN
}

