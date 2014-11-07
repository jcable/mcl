/* $Id: mcl_rx.cpp,v 1.5 2004/05/26 07:36:04 roca Exp $ */
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


mcl_rx::mcl_rx ()
{
}


mcl_rx::~mcl_rx ()
{
}


/**
 * Process the incoming packet just received.
 * This can be a pure signaling packet or a pure data packet, or
 * a data packet plus a signaling header (usual case).
 * This function and the following ones take control of the pbuf
 * buffer, and can free it when required...
 * This function is used both by a sender (for feedback packets)
 * and a receiver (data and control packets).
 */
void
mcl_rx::process_pkt (mcl_cb	*const mclcb,
		     mcl_rx_pkt	*pkt,
		     mcl_addr	*saddr)
{
	mcl_common_hdr_infos_t	chdr_infos; // infos of common hdr, host format

	TRACELVL(5, (mcl_stdout, "-> mcl_rx::process_pkt:\n"))
	ASSERT(pkt && pkt->pkt_len > 0);
	ASSERT(saddr && saddr->get_port() > 0);
	TRACELVL(4, (mcl_stdout,
		"mcl_rx::process_pkt: ses_id=%d, from %s/%d, len=%d\n",
		mclcb->get_id(),
		saddr->get_addr_string(), saddr->get_port(),
	       	pkt->pkt_len))
	if (mclcb->fsm.is_closed(mclcb)) {
		/*
		 * the session has been closed in the meanwhile... ignore
		 */
		TRACELVL(5, (mcl_stdout,
			"<- mcl_rx::process_pkt: ignored (session closed)\n"))
		delete pkt;
		return;
	}
	if (mclcb->get_verbosity() >= 4) {
		/* dump at most 16 32-bit words */
		mcl_dump_buffer(pkt->get_buf(), pkt->pkt_len, 16);
	}
	/*
	 * process the NORM common header first and perform some
	 * verifications at the same time
	 */
	memset(&chdr_infos, 0, sizeof(mcl_common_hdr_infos_t));
	if (mclcb->norm_pkt_mgmt.parse_common_hdr(mclcb, pkt, &chdr_infos)
	    == MCL_ERROR) {
		PRINT_ERR((mcl_stderr,
			"mcl_rx::process_pkt: ERROR, bad NORM header\n"))
		goto bad_hdr;
	}
	/*
	 * perform type-specific processing now
	 */
	switch (chdr_infos.type) {
	case NORM_DATA:
		/* receiver specific */
		TRACELVL(5, (mcl_stdout,
			"   mcl_rx::process_pkt: NORM_DATA\n"))
		if (this->process_data_pkt(mclcb, pkt, saddr, &chdr_infos)
		    == MCL_ERROR) {
			goto bad;
		}
		break;

	case NORM_CMD:
		TRACELVL(5, (mcl_stdout,
			"   mcl_rx::process_pkt: NORM_CMD\n"))
		if (mclcb->rx_ctrl.process_cmd_pkt(mclcb, pkt, saddr, &chdr_infos)
		    == MCL_ERROR) {
			goto bad;
		}
		break;

	case NORM_NACK:
		/* sender specific */
		TRACELVL(5, (mcl_stdout,
			"   mcl_rx::process_pkt: NORM_NACK\n"))
		if (mclcb->tx_ctrl.process_nack_pkt(mclcb, pkt, saddr, &chdr_infos)
		    == MCL_ERROR) {
			goto bad;
		}
		break;


	default:
		PRINT_ERR((mcl_stderr,
			"mcl_rx::process_pkt: ERROR, unknown NORM header %d\n",
			chdr_infos.type))
		goto bad_hdr;
	}


	TRACELVL(5, (mcl_stdout, "<- mcl_rx::process_pkt:\n"))
	return;

bad_hdr:
	mclcb->stats.bad_hdr++;
	mclcb->stats.errors++;
	delete pkt;

bad:
	TRACELVL(5, (mcl_stdout, "<- mcl_rx::process_pkt: ERROR\n"))
	return;
}


/**
 * Process an incoming NORM_DATA packet.
 * @param mclcb
 * @param pkt		pointer to packet
 * @param saddr		source address for this packet
 * @param chdr_infos	pointer to the common_infos struct
 */
mcl_error_status
mcl_rx::process_data_pkt(mcl_cb			*const mclcb,
			 class mcl_rx_pkt	*pkt,
			 mcl_addr		*saddr,
			 mcl_common_hdr_infos_t	*chdr_infos)
{
	mcl_data_hdr_infos_t	dhdr_infos; // infos of data hdr, host format
	INT32			hlen;	// total header length
	INT32			len;	// payload length (without headers)
	mcl_du			*du;	// DU for this data packet
	mcl_block		*blk;	// block for this data packet
	mcl_adu			*adu;	// ADU for this data packet

	TRACELVL(5, (mcl_stdout, "-> mcl_rx::process_data_pkt:\n"))
	/*
	 * process the NORM DATA header
	 */
	memset(&dhdr_infos, 0, sizeof(mcl_data_hdr_infos_t));
	if (mclcb->norm_pkt_mgmt.parse_data_hdr(mclcb, pkt, &dhdr_infos)
	    == MCL_ERROR) {
		PRINT_ERR((mcl_stderr,
			"mcl_rx::process_data_pkt: ERROR, bad NORM header\n"))
		goto bad_hdr_no_du_free;
	}
	hlen = sizeof(norm_common_hdr_t) + sizeof(norm_data_hdr_t);
	len = pkt->pkt_len - hlen;
	/*
	 * check what we got...
	 */
	ASSERT(len >= 0);
	if (len == 0) {
		/* empty data packet... nothing else to do */
		PRINT_ERR((mcl_stderr,
			"mcl_recv_it: ERROR, empty data packet\n")) 
		goto bad_hdr_no_du_free;
	}
	/*
	 * check that packet actually contains the announced payload
	 */
	if (len != (INT32)dhdr_infos.du_len) {
		/* no payload whereas some has been announced */
		PRINT_ERR((mcl_stderr,
			"mcl_rx::parse_data_hdr: ERROR, too short, no room for payload (%d announced, got %d)\n",
			dhdr_infos.du_len, len))
		goto bad_hdr_no_du_free;
	}
	if (mclcb->get_verbosity() >= 1)
		mcl_print_recvd_du(mclcb, true, 0, &dhdr_infos);
	/*
	 * create its descriptor
	 */
	du = new mcl_du;
	du->block = NULL;
	du->seq = dhdr_infos.idf_du;
	du->len = len;
	du->data = pkt->get_buf() + hlen;
	du->pkt = pkt;
	du->is_fec = dhdr_infos.is_fec;
	du->set_next(NULL);
	du->set_prev(NULL);
	/*
	 * update stats now (includes duplicated packets)
	 */
	if (du->is_fec) {
		mclcb->stats.rx_fec_pkts++;
		mclcb->stats.rx_fec_bytes += hlen + len;
	} else {
		mclcb->stats.rx_data_pkts++;
		mclcb->stats.rx_data_bytes += hlen + len;
	}
	mclcb->stats.rx_tot_pkts++;
	mclcb->stats.rx_tot_bytes += hlen + len;
	/*
	 * find the right ADU first...
	 */
	if (!(adu = mclcb->rx_window.find_adu(mclcb, dhdr_infos.idf_adu))) {
		/*
		 * this is the first packet of a new ADU, so register the adu
		 * first...
		 */
		if (mclcb->rx_window.process_adu_announcement(mclcb, saddr, &dhdr_infos) == MCL_ERROR) {
			PRINT_ERR((mcl_stderr, "mcl_rx::parse_data_hdr: adu announcement registering failed\n"))
			delete du;
			goto bad;
		}
		if (!(adu = mclcb->rx_window.find_adu(mclcb, dhdr_infos.idf_adu))) {
			/*
			 * orphan du (we received a DU before the announcement
			 * of its ADU).
			 */
			PRINT_ERR((mcl_stderr,
				"mcl_rx::parse_data_hdr: ERROR1, orphan DU [%d/%d/%d] (no adu)\n",
				dhdr_infos.idf_adu, dhdr_infos.idf_block, du->seq))
			delete du;
			goto bad;
		}
	}
	if (adu->get_rx_status() == ADU_STATUS_DECODED ||
	    adu->get_rx_status() == ADU_STATUS_DELIVERED) {
		/* useless du as the ADU is already decoded and/or delivered */
		TRACELVL(5, (mcl_stdout,
		"   mcl_rx::process_data_pkt: useless,  ADU already decoded or completed\n"))
		goto duplicated;
	}
	/*
	 * ... and then the right block...
	 */
	if (!(blk = adu->find_block(mclcb, dhdr_infos.idf_block))) {
		/*
		 * orphan du (we received a DU before the announcement
		 * of its ADU). Should put it in mclcb->rxlvl.du_head... TO DO.
		 */
		PRINT_ERR((mcl_stderr,
			"mcl_recv_it: ERROR2, orphan DU [%d/%d/%d] (no block)\n",
			dhdr_infos.idf_adu, dhdr_infos.idf_block, du->seq))
		delete du;
		goto bad;
	}
	if (blk->get_rx_status() == BLK_STATUS_DECODED) {
		/* useless du as the block is already decoded */
		TRACELVL(5, (mcl_stdout,
		"   mcl_rx::process_data_pkt: useless, block already decoded\n"))
		goto duplicated;
	}
	if (blk->get_rx_status() == BLK_STATUS_COMPLETED) {
	       if (du->is_fec) {
			/* can't do anything interesting with it */
			/* useless du as block already completed */
			TRACELVL(5, (mcl_stdout,
			"   mcl_rx::process_data_pkt: useless FEC du, block already completed\n"))
			goto duplicated;
		} else {
			/* it is possible that this data DU can replace a
			 * FEC du. Depends if DU has already been received
			 * or not!
			 * If possible, do it as FEC decoding is simplified.
			 */
		}
	}
	du->block = blk;
	ASSERT(du->block->adu == adu);

	/*
	 * ... check if it is a duplicated du or not
	 */
	if (du->is_fec) {
		if (blk->insert_in_fec_du_list(mclcb, du) == MCL_ERROR) {
			/* duplicated FEC du */
			TRACELVL(5, (mcl_stdout,
			"<- mcl_rx::process_data_pkt: duplicated FEC du\n"))
			goto duplicated;
		}
		//blk->fec_du_nb++;
	} else {
		/* this is a data DU */
		if (blk->insert_in_du_list(mclcb, du) == MCL_ERROR) {
			/* duplicated du */
			TRACELVL(5, (mcl_stdout,
			"<- mcl_rx::process_data_pkt: duplicated data du\n"))
			goto duplicated;
		}
		if (blk->get_rx_status() == BLK_STATUS_COMPLETED) {
			/* this data DU can indeed replace a FEC du */
#if 0 //#ifdef VIRTUAL_RX_MEM
			if (mcl_vrm_can_store_in_vrm(mclcb, du->len)) {
				/* use the VRMEM service to register data */
				if (mcl_vrm_store_data(mclcb, du, du->data, du->len)) {
					PRINT_ERR((mcl_stderr, "mcl_recv_it: ERROR: Virtual Rx Memory service failed\n"))
					goto bad;
				}
				/* pbuf no longer required... free it! */
				free(pbuf);
				pbuf = NULL;
			} else if (mclcb->vrm_used) {
				/* remember it kept in physical memory */
				mcl_vrm_register_in_prm(mclcb, du, du->len);
			}
#endif /* VIRTUAL_RX_MEM */
			/* TODO: free a fec du immediately! */
			goto duplicated_nofree;	/* to update dupl stats */
		}
	}
	/*
	 * ... no, so store it
	 */
#if 0 //#ifdef VIRTUAL_RX_MEM
	if (mcl_vrm_can_store_in_vrm(mclcb, du->len)) {
		/*
		 * use the VRMEM service to register data
		 */
		if (mcl_vrm_store_data(mclcb, du, du->data, du->len)) {
			PRINT_ERR((mcl_stderr, "mcl_recv_it: ERROR: Virtual Rx Memory service failed\n"))
			goto bad;
		}
		/* pbuf no longer required... free it! */
		free(pbuf);
		pbuf = NULL;
	} else if (mclcb->vrm_used) {
		/* remember it kept in physical memory */
		mcl_vrm_register_in_prm(mclcb, du, du->len);
	}
#endif /* VIRTUAL_RX_MEM */
	/*
	 * update stats now (non-duplicated packets only)
	 */
	mclcb->stats.rx_undup_pkts++;
	mclcb->stats.buf_space += len;
	if (mclcb->stats.buf_space > mclcb->stats.max_buf_space)
		mclcb->stats.max_buf_space = mclcb->stats.buf_space;
	/*
	 * was du the last DU of a block?
	 */
	if (blk->check_if_completed_and_process(mclcb)) {
		/*
		 * was du the last DU of an ADU ?
		 */
		adu->check_if_completed_and_process(mclcb);
	}

	TRACELVL(5, (mcl_stdout, "<- mcl_rx::process_data_pkt:\n"))
	return MCL_OK;


duplicated:
	mclcb->stats.rx_dupl_pkts++;
	mclcb->stats.rx_dupl_bytes += du->len;
	delete du;		// free the pkt too
	TRACELVL(5, (mcl_stdout, "<- mcl_rx::process_data_pkt: duplicated_1\n"))
	return MCL_OK;

duplicated_nofree:
	mclcb->stats.rx_dupl_pkts++;
	mclcb->stats.rx_dupl_bytes += du->len;
	TRACELVL(5, (mcl_stdout, "<- mcl_rx::process_data_pkt: duplicated_2\n"))
	return MCL_OK;


bad_hdr_no_du_free:
	mclcb->stats.bad_hdr++;
	mclcb->stats.errors++;
	delete pkt;		// no du object yet, so free pkt explicitely
	TRACELVL(5, (mcl_stdout, "<- mcl_rx::process_data_pkt: ERROR, bad hdr\n"))
	return MCL_ERROR;

bad:
	TRACELVL(5, (mcl_stdout, "<- mcl_rx::process_data_pkt: ERROR\n"))
	return MCL_ERROR;
}


#if 0
/**
 * Process an incoming NORM_CMD packet.
 * @param mclcb
 * @param pkt		pointer to packet
 * @param saddr		source address for this packet
 * @param chdr_infos	pointer to the common_infos struct
 */
mcl_error_status
mcl_rx::process_cmd_pkt(mcl_cb			*const mclcb,
			class mcl_rx_pkt	*pkt,
			mcl_addr		*saddr,
			mcl_common_hdr_infos_t	*chdr_infos)
{
	mcl_data_hdr_infos_t	dhdr_infos; // infos of data hdr, host format
	//INT32			hlen;	// total header length
	//INT32			len;	// payload length (without headers)

	TRACELVL(5, (mcl_stdout, "-> mcl_rx::process_cmd_pkt:\n"))
	/*
	 * process the NORM DATA header
	 */
	memset(&dhdr_infos, 0, sizeof(mcl_data_hdr_infos_t));
	if (mclcb->norm_pkt_mgmt.parse_cmd_hdr(mclcb, pkt, &dhdr_infos)
	    == MCL_ERROR) {
		PRINT_ERR((mcl_stderr,
			"mcl_rx::process_cmd_pkt: ERROR, bad NORM header\n"))
		goto bad_hdr;
	}

	switch (dhdr_infos.cmd_flavor) {
	case NORM_CMD_FLUSH:
		TRACELVL(5, (mcl_stdout, "   mcl_rx::process_cmd_pk: FLUSH\n"))
		// TODO
		break;

	case NORM_CMD_NO_NEW_OBJECT:
		TRACELVL(5, (mcl_stdout, "   mcl_rx::process_cmd_pk: NO_NEW_OBJECT\n"))
		if (mclcb->fsm.no_new_adu(mclcb)) {
			/* this is a duplicated announce; ignore */
			break;
		}
		if (mclcb->get_verbosity() >= 2)
			mcl_print_recvd_du(mclcb, false, MCL_SIG_NONEWADU, &dhdr_infos);
		mclcb->rx.set_highest_adu_seq_of_session(dhdr_infos.max_idf_adu);
		mclcb->fsm.update_rx_state(mclcb, REVENT_NO_NEW_ADU);
	
		break;

	case NORM_CMD_CLOSE:
		TRACELVL(5, (mcl_stdout, "   mcl_rx::process_cmd_pk: CLOSE\n"))
		if (mclcb->fsm.close_already_rx(mclcb)) {
			/* this is a duplicated announce; ignore */
			break;
		}
		if (mclcb->get_verbosity() >= 2)
			mcl_print_recvd_du(mclcb, false, MCL_SIG_CLOSE, &dhdr_infos);
		mclcb->fsm.update_rx_state(mclcb, REVENT_CLOSE_RECV);
		ASSERT(mclcb->fsm.no_new_adu(mclcb));
		if (mclcb->get_stats_level() >= 1) {
			mclcb->stats.print_rx_stats(mclcb);
			mclcb->stats.print_final_stats(mclcb);
		}
		break;

	default:
		exit(-1);
	}

	TRACELVL(5, (mcl_stdout, "<- mcl_rx::process_cmd_pkt:\n"))
	return MCL_OK;


bad_hdr:
	mclcb->stats.bad_hdr++;
	mclcb->stats.errors++;
	delete pkt;		// no du object yet, so free pkt explicitely
	TRACELVL(5, (mcl_stdout, "<- mcl_rx::process_data_pkt: ERROR, bad hdr\n"))
	return MCL_ERROR;

#ifdef NOT_YET
bad:
	TRACELVL(5, (mcl_stdout, "<- mcl_rx::process_data_pkt: ERROR\n"))
	return MCL_ERROR;
#endif
}
#endif // 0


/**
 * Try to return an ADU to the appli.
 * Maybe there is a gap and this is not possible...
 * Return the amount of data copied to userbuf, 0 if some
 * data is available but not copied to userbuf, and < 0 if
 * no data is available.
 */
INT32
mcl_rx::return_adu_to_appli(mcl_cb	*const mclcb,
			    char		* userbuf,
			    INT32		userlen,
			    struct sockaddr	* saddr,
			    INT32		* saddr_len)
{
	mcl_adu		*adu;
	mcl_block	*blk;
	mcl_du		*du;
	mcl_du		*ndu;		/* next du */
	char		*dst;		/* where to do data copy in userbuf */
	INT32		i, j;
	INT32		len, rem;	/* remaining data for copy to userbuf */


	TRACELVL(5, (mcl_stdout,
		"-> mcl_rx::return_adu_to_appli: userbuf=x%x, len=%d\n",
		 (INT32)userbuf, userlen))
	/*
	 * find a ready adu first, depending on the receiving delivery mode...
	 */
	adu = mclcb->rx_window.get_next_ready_adu(mclcb);
	if (adu == NULL) {
		TRACELVL(5, (mcl_stdout, "<- mcl_rx::return_adu_to_appli: no adu ready\n"))
		return -1;
	}
	if (userlen <= 0) {
		/* it was just to check if an ADU was available... */
		ASSERT(userbuf);
		return 0;
	}
	ASSERT(userbuf && userlen > 0);
#if 0
	/*
	 * ok, so remove the adu from rx list
	 */
	mclcb->rx_window.remove_adu(mclcb, adu);
#endif
	/*
	 * then copy data...
	 * it requires to go through all the DUs of all the blocks of the ADU
	 */
	rem = userlen;
	dst = userbuf;
	for (i = adu->get_block_nb(), blk = adu->get_block_head();
	     i > 0; i--, blk++) {
		ASSERT(blk->du_nb <= blk->get_du_nb_in_list());
		ASSERT(blk->get_fec_du_nb_in_list() == 0);
		for (j = blk->du_nb, du = blk->get_du_head(); j > 0 && rem > 0;
		     j--, du = ndu) {
			/* don't copy more than remaining space in buf */
			if (rem < du->len) {
				len = rem;
				PRINT_ERR((mcl_stderr,
					"mcl_rx::return_adu_to_appli: ERROR: user buffer too short (%d required, %d available). Truncated\n",
					adu->get_len(), userlen))
				mclcb->stats.other_errors++;
			} else {
				len = du->len;
			}
			/* copy data in appli's buffer */
			memcpy(dst, du->data, len);
			dst += len;
			rem -= len;
			ndu = du->get_next();
			//mclcb->stats.buf_space -= du->len;
		}
	}
	/*
	 * adu is about to be delivered...
	 */
	mclcb->rx_window.mark_adu_delivered(adu);
	/*
	 * free all buffers (blocks, data DUs, FEC DUs) now...
	 */
	adu->remove_and_free_all_buffers(mclcb);
	/*
	 * copy source addr in appli's buffer
	 */
	if (saddr) {
		if (saddr_len == NULL ||
		    *saddr_len < (INT32)sizeof(struct sockaddr_in)) {
			PRINT_ERR((mcl_stderr,
			"mcl_rx::return_adu_to_appli: ERROR, addr buffer too short (%d expected, got %d)\n",
			sizeof(struct sockaddr_in), *saddr_len))
			mcl_exit(-1);
		}
		adu->addr.get_addr_struct((struct sockaddr_in*)saddr);
		*saddr_len = sizeof(struct sockaddr_in);
	}
	//ASSERT(mclcb->ready_data >= 0);
#ifdef DEBUG
	if (mclcb->get_verbosity() >= 4) {
		/* just to check data received... will be in appli */
		mcl_dump_buffer(userbuf, adu->get_len(), adu->get_len() >> 2);
	}
#endif
	if (mclcb->get_verbosity() == 2) {
		struct timeval	time;
		time = mcl_get_tvtime();
		PRINT_OUT((mcl_stdout, "\n%ld.%06ld\tadu_delivered seq=%d len=%d\n",
			time.tv_sec, time.tv_usec, adu->get_seq(), adu->get_len()))
#ifdef GET_SYSINFO
		mcl_print_sysinfo(mclcb);
#endif
	}
	/*
	 * update stats
	 */
	TRACELVL(5, (mcl_stdout, "<- GiveADUtoAppli: %d bytes returned\n", userlen - rem))
	return (userlen - rem);
}

