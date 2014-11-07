/* $Id: mcl_rx.cpp,v 1.52 2005/05/23 15:37:18 roca Exp $ */
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

/******************************************************************************/

/**
 * Default constructor.
 */
mcl_rx::mcl_rx ()
{
#ifdef SSM
	this->ssm = false;
#endif
	this->src_addr.reset();
	this->check_src_addr = false;
	this->never_leave_base_layer = true;
}

/**
 * Default destructor.
 */
mcl_rx::~mcl_rx ()
{
#ifdef SSM
	this->ssm = false;
#endif
	this->src_addr.reset();
	this->check_src_addr = false;
	this->never_leave_base_layer = true;
}


/******************************************************************************/


/*
 * private functions 
 */

/* uncomment the following define if you face physical layer duplications */ 
// #define REMOVE_PHY_LAYER_DUPLICATES
#ifdef REMOVE_PHY_LAYER_DUPLICATES
static bool	mcl_is_pkt_duplicated_at_phy_layer (mcl_cb *mclcb,
					hdr_infos_t *hdr_infos, int layer);
#endif
#ifdef RSE_FEC
static void	mcl_decode_all_adu	(mcl_cb *mclcb);
static void	mcl_decode_this_adu	(mcl_cb *mclcb, adu_t *adu);
#endif /* RSE_FEC */


/*
 * Reception thread polling data regularly on the various mcast groups
 */
void*
mcl_rx_thread (void	*arg)
{
	mcl_cb	*mclcb;

	/*
	 * we don't sleep here but in the mcl_recv_pkt func.
	 * the unlock will be done there...
	 */
	mclcb = (mcl_cb*)arg;
	ASSERT(mclcb != NULL);
	TRACELVL(5, (mcl_stdout, "-> mcl_rx_thread:\n"))

#ifdef WIN32
	mcl_thread_testcancel(mclcb);
#else  // UNIX
	/*
	 * cancellation is deferred till next check point , i.e. points
	 * in code where we know everything is in a stable state
	 */
	pthread_setcancelstate(PTHREAD_CANCEL_ENABLE,NULL);
	pthread_setcanceltype(PTHREAD_CANCEL_DEFERRED,NULL);
	mcl_thread_testcancel(mclcb);
#endif
  	mclcb->lock();
	while (1) {
		if (mclcb->fsm.is_closed(mclcb)) {
			/* everything is finished */
			break;
		}
		if (mclcb->get_verbosity() >= 3) {
			PRINT_OUT((mcl_stdout,
				"Rx Thread %ld, ses_id=%d, time %d, state %s\n",
				(UINT32)mclcb->rx_thread, mclcb->get_id(),
				mcl_time_count,
				mclcb->fsm.print_rx_state(mclcb)))
		}
		/*if (mclcb->fsm.close_already_rx(mclcb))*/
		if (mclcb->fsm.no_new_undup_du(mclcb)) {
			/*
			 * we received all DUs, we now wait the application
			 * finishes to read all data and issues a mcl_close.
			 */
			mclcb->unlock();
			mcl_thread_testcancel(mclcb);
			mcl_usleep(DFLT_POLLING_PERIOD);
			mcl_thread_testcancel(mclcb);
			mclcb->lock();
		} else {
			/* get new pkts; mcl_process_pkt is called each time */
			mcl_recv_pkt(mclcb);
		}
	}
	TRACELVL(5, (mcl_stdout, "<- mcl_rx_thread:\n"))
	mclcb->unlock();
	arg = 0;
#ifdef WIN32
	ExitThread(0);
#else
	pthread_exit(arg);
#endif
	return arg;	/* unused */
}


/*
 * Get the packet received...
 * This can be a pure signaling packet or a pure data packet, or
 * a data packet plus a signaling header (usual case).
 */
void
mcl_process_pkt (mcl_cb*	mclcb,
		 mcl_rx_pkt	*pkt,	/* recv'd pkt */
		 mcl_addr	*saddr,	/* src addr */
		 INT32		layer)
{
	hdr_infos_t	hdr_infos;	/* info extracted from the LCT header */
	du_t		*du;		/* DU descriptor of the data buf */
	block_t		*blk;		/* block to which the DU belongs */
	//int		layer;
	int		len;		/* data len */
	adu_t		*adu;		/* ADU to which the DU belongs */
	int		hlen = 0;	/* total header length */
	int		ret = 0;

	TRACELVL(5, (mcl_stdout, "-> mcl_process_pkt:\n"))
	ASSERT(pkt && pkt->pkt_len > 0);
	ASSERT(saddr && saddr->get_port() > 0);
	TRACELVL(4, (mcl_stdout,
		"mcl_process_pkt: ses_id=%d, from %s/%d, len=%d, pkt_struct=x%x\n",
		mclcb->get_id(), saddr->get_addr_string(), saddr->get_port(),
	       	pkt->pkt_len, (int)pkt))
	if (mclcb->fsm.is_closed(mclcb)) {
		/*
		 * the session has been closed in the meanwhile... ignore
		 */
		TRACELVL(5, (mcl_stdout,
			"<- mcl_process_pkt: ignored (session closed)\n"))
		delete pkt;
		return;
	}
	if (mclcb->get_verbosity() >= 4) {
		/* dump at most 16 32-bit words */
		mcl_dump_buffer(pkt->get_buf(), pkt->pkt_len, 16);
	}
	/*
	 * process the LCT header (extension then fixed header) first
	 */
	memset(&hdr_infos, 0, sizeof(hdr_infos));
	hdr_infos.FPI_present	= hdr_infos.FTI_present = hdr_infos.NONEWADU_present = hdr_infos.close = false;
	if (pkt->pkt_len < (int)(sizeof(fixed_lct_hdr_t))) {
		PRINT_ERR((mcl_stderr,
			"mcl_process_pkt: ERROR too short %d\n", pkt->pkt_len))
		goto bad_hdr;
	}
	if ((hlen = alc_hdr_parse(mclcb, (fixed_lct_hdr_t*)pkt->get_buf(),
				  &hdr_infos, pkt->pkt_len)) < 0) {
		PRINT_ERR((mcl_stderr,
			"mcl_process_pkt: ERROR, bad LCT header\n"))
		goto bad_hdr;
	}
	/*
	 * right session ? check the {source_addr; TSI}
	 */
	if ((int)hdr_infos.demux_label != mclcb->demux_label ||
	    (mclcb->rx.check_src_addr && mclcb->rx.src_addr.addr_is_equal(*saddr) == false)) {
		TRACELVL(4, (mcl_stdout,
			"   mcl_process_pkt: pkt for bad session (expected %s/%d, got %s/%d)\n",
			mclcb->rx.src_addr.get_addr_string(), mclcb->demux_label,
			saddr->get_addr_string(), hdr_infos.demux_label))
		mclcb->stats.bad_demux_label++;
		goto bad;
	}
	/*
	 * Some network configurations have the bad idea to duplicate
	 * packets at the physical layer (this is sometimes the case
	 * with some switchs). So remove immediately this kind of
	 * duplicates, without taking them into account in statistics,
	 * and before doing congestion control checks. 
	 */
#ifdef REMOVE_PHY_LAYER_DUPLICATES
	if (mcl_is_pkt_duplicated_at_phy_layer(mclcb, &hdr_infos, layer)) {
		TRACELVL(5, (mcl_stdout,
			"<- mcl_process_pkt: ignored (duplicated pkt at physical layer)\n"))
		delete pkt;
		return;
	}
#endif /* REMOVE_PHY_LAYER_DUPLICATES */
	/*
	 * process the congestion control header then
	 */
	if (mclcb->congestion_control == NO_CC) {
		/*
		 * In no congestion control mode there is no CC header.
		 * Required for FLUTE interoperability tests, but it
		 * prevents loss statistics!
		 */
		layer = 0;
	} else {
		/* process the congestion header */
#ifdef RLC
		if (mclcb->congestion_control == RLC_CC) {
			layer = rlc_rx_analyze_packet(mclcb,
					(rlc_hdr_t*)(pkt->get_buf() + 4));
		} else
#endif 
#ifdef FLIDS
		if (mclcb->congestion_control == FLID_SL_CC) {
			layer = FLIDs_rx_AnalyzePacket(mclcb,
					(flids_hdr_t*)(pkt->get_buf() + 4));
		}
#endif
		if (layer < 0) {
			PRINT_ERR((mcl_stderr,
			"mcl_process_pkt: ERROR while processing RLC/FLIDS header (error=%d)\n", layer))
			goto bad_hdr;
		}
	}
	/*
	 * process the multiple signaling headers if any
	 */
	if (hdr_infos.FTI_present) {
		/* FEC Object Transmission Information (FTI) HE present */
		ret = mcl_process_sig (mclcb, EXT_FTI, &hdr_infos, saddr);
	}
	if (hdr_infos.FDT_present) {
		/* FLUTE's File Delivery Table (FDT) HE present */
		ret = mcl_process_sig (mclcb, EXT_FDT, &hdr_infos, saddr);
	}
#ifdef METAOBJECT_USED	
	if (hdr_infos.MODT_present) {
		/* Object Delivery Table (MODT) HE present */
		ret = mcl_process_sig (mclcb, EXT_MODT, &hdr_infos, saddr);
	}
#endif	
	if ((ret >= 0) && hdr_infos.NONEWADU_present) {
		/* LCT equivalent to NONEWADU */
		if (!mclcb->fsm.no_new_adu(mclcb)) {
			ret = mcl_process_sig (mclcb, EXT_NONEWADU,
						&hdr_infos, saddr);
		}
		/* else ignore (this is a duplicated announce) */
	}
	/*
	 * Warngin: SIG_CLOSE has to be processed at the end of packet
	 * reception
	 */
	if (ret < 0) {
		PRINT_ERR((mcl_stderr,
			"mcl_process_pkt: ERROR in header processing\n")) 
		goto bad_hdr;
	}
	if (mclcb->rx_flute.delete_current_packet == true) {
		/* in FLUTE_DELIVERY mode, do not store adu & packet */
		mclcb->rx_flute.delete_current_packet = false;
		TRACELVL(5, (mcl_stdout,
			"<- mcl_process_pkt: ignored (FLUTE_DELIVERY)\n"))	
		delete pkt;
		return;
	}
	mclcb->stats.rx_totbytes += hlen; /* data len incr later; see below...*/
	/*
	 * check what we got...
	 */
	len = pkt->pkt_len - hlen;	/* data len */
	ASSERT(len >= 0);
	if (len == 0) {
		/* not a data packet... nothing else to do */
		/* we need to check SIG_CLOSE now */
		if (hdr_infos.close) {
			/* LCT equivalent to CLOSE */
			ret = mcl_process_sig (mclcb, SIG_CLOSE, &hdr_infos,
						saddr);
			if (ret < 0) {
				PRINT_ERR((mcl_stderr,
					"mcl_process_pkt: ERROR in SIG_CLOSE processing\n")) 
				goto bad_hdr;
			}
		}
		delete pkt;		/* no more necessary */
		TRACELVL(5, (mcl_stdout, "<- mcl_process_pkt: only SIG\n"))
		return;
	}
	if (hdr_infos.FPI_present == false) {
		/* error, data packet without any payload info */
		PRINT_ERR((mcl_stderr,
			"mcl_process_pkt: ERROR, data packet without any payload info\n")) 
		goto bad_hdr;
	}
	if (mclcb->get_verbosity() >= 1)
		mcl_print_recvd_du(mclcb, 0, layer, &hdr_infos);
	/*
	 * create its descriptor
	 */
	du = CreateDU(mclcb);
	du->seq = hdr_infos.idf_du;
	du->len = len;
	du->data = pkt->get_buf() + hlen;
	du->pkt = pkt;
#ifdef FEC
	du->is_fec = hdr_infos.is_fec;
#endif
	/*
	 * update stats now (includes duplicated packets)
	 */
	mclcb->stats.rx_pkts_per_lvl[layer]++;
	mclcb->stats.rx_bytes_per_lvl[layer] += len;
#ifdef FEC
	if (du->is_fec) {
		mclcb->stats.rx_fec_pkts++;
		mclcb->stats.rx_fec_bytes += len;
	} else {
#endif
		mclcb->stats.rx_pkts++;
		mclcb->stats.rx_bytes += len;
#ifdef FEC
	}
#endif
	mclcb->stats.rx_totbytes += len;/* sig hlen already incr; see above...*/
	/*
	 * find the right ADU first...
	 */
	if (!(adu = mclcb->rx_window.find_adu(mclcb, hdr_infos.idf_adu,
					hdr_infos.FDT_instanceid))) {
	       	/*
		 * orphan du (i.e. we receive a DU before the announcement
		 * of the corresponding ADU).
		 * XXX: put it in mclcb->rx_window.du_head list... TO DO.
		 */	
		PRINT_ERR((mcl_stderr,
			"mcl_process_pkt: ERROR1, orphan DU [%d/%d/%d] (no adu)\n",
			hdr_infos.idf_adu, hdr_infos.idf_block, du->seq))
		free(du);
		goto bad;
	}
	if (adu->rx_status >= ADU_STATUS_DECODED) {
		/* useless du as the ADU is already decoded and/or delivered */
		TRACELVL(3, (mcl_stdout, "mcl_process_pkt: duplicated0\n"))
		goto duplicated;
	}
	/*
	 * ... and then the right block...
	 */
	if (!(blk = FindBlock(mclcb, hdr_infos.idf_block, adu->block_head,
				adu))) {
		/*
		 * orphan du (we received a DU before the announcement
		 * of its ADU). Should put it in mclcb->rxlvl.du_head... TO DO.
		 */
		PRINT_ERR((mcl_stderr,
			"mcl_process_pkt: ERROR2, orphan DU [%d/%d/%d] (no block)\n",
			hdr_infos.idf_adu, hdr_infos.idf_block, du->seq))
		free(du);
		goto bad;
	}
	if (blk->rx_status == BLK_STATUS_DECODED) {
		/* useless du as the block is already decoded */
		TRACELVL(3, (mcl_stdout, "mcl_process_pkt: duplicated1\n"))
		goto duplicated;
	}
	if (blk->rx_status == BLK_STATUS_COMPLETED) {
#ifdef FEC
	       if (du->is_fec) {
			/* can't do anything interesting with it */
			/* useless du as block already completed */
			TRACELVL(3, (mcl_stdout,
				"mcl_process_pkt: duplicated2 fec\n"))
			goto duplicated;
		} else {
#endif
			/* it is possible that this data DU can replace a
			 * FEC du. Depends if DU has already been received
			 * or not!
			 * If possible, do it as FEC decoding is simplified.
			 */
#ifdef FEC
		}
#endif
	}
	du->block = blk;
	ASSERT(du->block->adu == adu);
	/*
	 * insert it in the DU list if appropriate...
	 */
#ifdef FEC	 
	if (du->is_fec) {
#ifdef LDPC_FEC
		if ((adu->fec_scheme != MCL_FEC_SCHEME_LDGM_STAIRCASE_132_0) &&
		    (adu->fec_scheme != MCL_FEC_SCHEME_LDGM_TRIANGLE_132_1))
#endif
		{
			/* register the FEC DU for later decoding */
			if (InsertDU(mclcb, du, &(blk->fec_du_head)) <= 0) {
				/* duplicated FEC du */
				TRACELVL(3, (mcl_stdout,
					"mcl_process_pkt: duplicated3 fec\n"))
				goto duplicated;
			}
			blk->fec_du_nb_in_list++;
		}
#ifdef LDPC_FEC
		else {
			/*
			 * Do not register FEC DUs in case of LDGM codes since 
			 * they will be immediately free'ed after the
			 * fec.decode() call.
			 * A drawback is that there's no way to detect
			 * duplicated FEC DUs, but it makes the total amount
			 * of memory needed far smaller, which is of prime
			 * interest!
			 * This is not a problem though for decoding, since the
			 * LDPC/LDGM codec automatically detects it.
			 */
		}
#endif
	} else 
#endif /* FEC */
	{
		/*
		 * source DU
		 */
		if (InsertDU(mclcb, du, &(blk->du_head)) <= 0) {
			/* duplicated du */
			TRACELVL(3, (mcl_stdout,
				"mcl_process_pkt: duplicated3 data\n"))
			goto duplicated;
		}
		blk->du_nb_in_list++;
		adu->recvd_src_data += du->len;
	}
	/*
	 * store data buffer either in vrfile (when desired and needed)
	 * or in physical memory, whichever is the most appropriate.
	 */
	if (mclcb->rx_storage.store_data(mclcb, du) == MCL_ERROR) {
		PRINT_ERR((mcl_stderr,
		"mcl_process_pkt: ERROR: Virtual Rx Memory service failed\n"))
		goto bad;
	}
#ifdef RSE_FEC
	/*
	 * there is one case where having a duplicated data DU is not
	 * an issue. Check it...
	 */
	if ((du->is_fec == false) &&
	    (adu->fec_scheme == MCL_FEC_SCHEME_RSE_129_0) &&
	     blk->rx_status == BLK_STATUS_COMPLETED) {
		/*
		 * this data DU can indeed replace a FEC DU in case of RSE
		 */
		/* TODO: free one FEC du immediately! */
		/* we need to check SIG_CLOSE now */
		if (hdr_infos.close) {
			/* LCT equivalent to CLOSE */
			ret = mcl_process_sig(mclcb, SIG_CLOSE,
						&hdr_infos, saddr);
			if (ret < 0) {
				PRINT_ERR((mcl_stderr,
					"mcl_process_pkt: ERROR in SIG_CLOSE processing\n")) 
				goto bad_hdr;
			}
		}
		mclcb->stats.rx_dupl_pkts ++;
		mclcb->stats.rx_dupl_bytes += du->len;
		TRACELVL(3, (mcl_stdout, "mcl_process_pkt: duplicated4 data\n"))
		TRACELVL(5, (mcl_stdout, "<- mcl_process_pkt:\n"))
		return;
	}
#endif /* RSE_FEC */
	/*
	 * update stats now (non-duplicated packets only)
	 */
	mclcb->stats.rx_undup_pkts_per_lvl[layer]++;
	/*
	 * perform FEC decoding now.
	 * The codec used is specified by the
	 * {fec_encoding_id; fec_instance_id} fields.
	 */
	switch (adu->fec_scheme) {
#ifdef RSE_FEC
	case MCL_FEC_SCHEME_RSE_129_0:
		/*
		 * was du the last DU of a block/ADU ?
		 */
		if (mcl_rx_enough_du(mclcb, blk)) {
			if (mclcb->postpone_fec_decoding) {
				/* ok, we rx enough data or FEC DUs,
				 * just mark it */
				blk->rx_status = BLK_STATUS_COMPLETED;
			} else {
				/* ok, we rx enough data or FEC DUs,
				 * so decode */
				mclcb->fec.decode(mclcb, blk);
			}
		}
		break;
#endif /* RSE_FEC */

#ifdef LDPC_FEC
	case MCL_FEC_SCHEME_LDGM_STAIRCASE_132_0:
	case MCL_FEC_SCHEME_LDGM_TRIANGLE_132_1:
		/*
		 * iterative decoding takes place for all incoming packets...
		 * Start by specifying the FEC codec used with this ADU.
		 */
		mclcb->fec.set_fec_code(mclcb, adu->fec_scheme);
		mclcb->fec.decode(mclcb, du);
		if (du->is_fec) {
			/* this FEC DU is no longer needed once
			 * fec.decode() has been called
			 * (they have not been stored in fec du list) */
			mclcb->rx_storage.free_data(mclcb, du);
			free(du);
		}
		break;
#endif

	case MCL_FEC_SCHEME_NULL:
		/*
		 * was du the last DU of a block/ADU ?
		 */
		if (mcl_rx_enough_du(mclcb, blk)) {
			blk->rx_status = BLK_STATUS_DECODED;
		}
		break;

	default:
		PRINT_ERR((mcl_stderr,
			"mcl_process_pkt: ERROR: unsupported FEC scheme %d\n",
			adu->fec_scheme))
		mcl_exit(-1);
	}
	/*
	 * see if an ADU has been completely received, and in that
	 * case if some data can be delivered to the application.
	 */
	if (mcl_rx_new_completed_adu(mclcb, adu)) {
		if (mclcb->get_verbosity() >= 1)
			PRINT_OUT((mcl_stdout, "End of ADU %d\n", adu->seq))
		if (mclcb->get_stats_level() == 2)
			mcl_print_rx_stats(mclcb);
#ifdef RSE_FEC
		if (!mclcb->postpone_fec_decoding &&
		    adu->rx_status == ADU_STATUS_COMPLETED) {
			/* do not wait, decode every block of this adu */
			mcl_decode_this_adu (mclcb, adu);
		}
#endif /* RSE_FEC */
		mclcb->rx_window.mark_ready_adu();	/* remember it */
		if (mclcb->fsm.no_new_adu(mclcb) &&
		    mclcb->rx_window.check_if_all_adu_completed(mclcb) == true) {
			/*
			 * we know we won't receive any new ADU and
			 * it was the last packet we were waiting for...
			 */
			if (mclcb->get_verbosity() >= 1)
				PRINT_OUT((mcl_stdout, "All ADUs received\n"))
			mclcb->fsm.update_rx_state(mclcb, REVENT_ALL_DU_RECV);
			if (mclcb->get_stats_level() >= 1) {
				mcl_print_rx_stats(mclcb);
				mcl_print_final_stats(mclcb);
			}
			/*
			 * unsubscribe to all layers (incl. layer 0) to avoid
			 * receiving useless packets
			 */
			mcl_drop_layer(mclcb, MCL_ALL_LAYERS, MCL_DO_IT);
#ifdef RSE_FEC
			/*
			 * we can now decode all ADUs if in postpone mode
			 */
			if (mclcb->postpone_fec_decoding) {
				mcl_decode_all_adu(mclcb);
			}
#endif /* RSE_FEC */
		}
	}
	/* we need to check SIG_CLOSE now */
	if (hdr_infos.close) {
		/* LCT equivalent to CLOSE */
		ret = mcl_process_sig (mclcb, SIG_CLOSE, &hdr_infos, saddr);
		if (ret < 0) {
			PRINT_ERR((mcl_stderr,
			"mcl_process_pkt: ERROR in header processing\n")) 
			goto bad_hdr;
		}
	}
	TRACELVL(5, (mcl_stdout, "<- mcl_process_pkt:\n"))
	return;

duplicated:
	/* we need to check SIG_CLOSE now */
	if (hdr_infos.close) {
		/* LCT equivalent to CLOSE */
		ret = mcl_process_sig (mclcb, SIG_CLOSE, &hdr_infos, saddr);
		if (ret < 0) {
			PRINT_ERR((mcl_stderr,
			"mcl_process_pkt: ERROR in header processing\n")) 
			goto bad_hdr;
		}
	}
	mclcb->stats.rx_dupl_pkts ++;
	mclcb->stats.rx_dupl_bytes += du->len;	/* do it before the free(du)! */
	delete pkt;
	free(du);
	TRACELVL(5, (mcl_stdout, "<- mcl_process_pkt: duplicated\n"))
	return;

bad_hdr:
	mclcb->stats.bad_hdr++;
bad:
	mclcb->stats.errors++;
	delete pkt;
	TRACELVL(5, (mcl_stdout, "<- mcl_process_pkt: ERROR\n"))
	return;
}


/**
 * Identify packets duplicated at the physical layer.
 * Some network configurations have the bad idea to duplicate
 * packets at the physical layer (this is sometimes the case
 * with some switchs). So identify immediately this kind of
 * duplicates.
 * This function has limitations:
 * - it only works with data packets
 * - since we compare the new packet with the previous one, it
 *   only works if a duplicate is sent immediately (not necessarily
 *   true with high transmission rates)
 * - it only works with a single source (with two or more sources,
 *   we should keep a copy of idf values per source)
 *
 * @param mclcb
 * @param hdr_infos
 * @param lay		layer where packet has been received
 * @return		true if packet is duplicated, false otherwise.
 */
#ifdef REMOVE_PHY_LAYER_DUPLICATES
static bool
mcl_is_pkt_duplicated_at_phy_layer (mcl_cb	*mclcb,
				    hdr_infos_t	*hdr_infos,
				    int		layer)
{
	static	UINT32	prev_idf_adu[MAX_NB_TX_LAYERS];
	static	UINT32	prev_idf_block[MAX_NB_TX_LAYERS];
	static	UINT32	prev_idf_du[MAX_NB_TX_LAYERS];
	static	UINT32	demux_label;
	static bool		initialized = false;
	bool			dup;
	int			i;

	if (!initialized) {
		/* initialize everything with a random value */
		for (i = 0; i < MAX_NB_TX_LAYERS; i++) {
			prev_idf_adu[i] = prev_idf_block[i] =
			prev_idf_du[i] = 32109876;
		}
		demux_label = 32109876;
		initialized = true;
	}
	dup = false;
	if (hdr_infos->FPI_present) {
		if ((hdr_infos->idf_du == prev_idf_du[layer]) &&
		    (hdr_infos->idf_block == prev_idf_block[layer]) &&
		    (hdr_infos->idf_adu == prev_idf_adu[layer]) &&
		    (hdr_infos->demux_label == demux_label)) {
			dup = true;
		} else {
		    	prev_idf_du[layer] = hdr_infos->idf_du;
		    	prev_idf_block[layer] = hdr_infos->idf_block;
		    	prev_idf_adu[layer] = hdr_infos->idf_adu;
			demux_label = hdr_infos->demux_label;
		}
	}
	TRACELVL(5, (mcl_stdout, "<- mcl_is_pkt_duplicated_at_phy_layer: %s\n",
		(dup ? "yes" : "no")))
	return dup;
}
#endif /* REMOVE_PHY_LAYER_DUPLICATES */


/*
 * Process the various types of signalling
 * returns:	< 0, if error
 *		>= 0 if ok
 */
int
mcl_process_sig (mcl_cb	*mclcb,
		 int		type,		/* sig type */
		 hdr_infos_t	*hdr_infos,
		 mcl_addr	*saddr)		/* src addr or NULL */
{
	adu_t 	*adu;

	ASSERT(!mclcb->fsm.is_closed(mclcb));
	switch (type) {
	case EXT_FDT:		/* File Delivery Table (FLUTE specific) */
		TRACELVL(5, (mcl_stdout, "-> mcl_process_sig: EXT_FDT (%d)\n",
			type))		
		break;
#ifdef METAOBJECT_USED
	case EXT_MODT:		/* Meta-Object Delivery Table */
		TRACELVL(5, (mcl_stdout, "-> mcl_process_sig: EXT_MODT (%d)\n",
			type))
		if (mclcb->meta_obj_layer->in_use() == false) mclcb->meta_obj_layer->set_in_use(true);
		break;
#endif
	case EXT_FTI: {		/* FEC Object Transmission Information */
		block_t		*blk;
		u_int		rem;
		UINT32	tot_blk_nb;	/* total nb of blocks required*/
		UINT32	k_for_this_blk; /* all blks are not same len */
		UINT32	blk_seq;	/* block seq number */
		mcl_blocking_struct_t	*bs;	/* ptr to ADU blocking struct */
#ifdef FEC
		double	fec_ratio = 0.0;/* n/k value, constant for all blocks */
					/* even if some blks are shorter */
#endif

		TRACELVL(5, (mcl_stdout, "-> mcl_process_sig: EXT_FTI (%d)\n",
			type))
		adu = mclcb->rx_window.find_adu(mclcb, hdr_infos->idf_adu,
						hdr_infos->FDT_instanceid);
		if (adu && !(adu->ia_adu)) {
			/*
			 * ADU already announced and completed!
			 * NB: to counter losses, new ADUs are
			 *     announced in each data packet (EXT_FTI)
			 */
			break;
		}
		if (mclcb->get_verbosity() >= 1) {
			PRINT_OUT((mcl_stdout,
			"New ADU: seq=%d, len=%d, max_k=%d, max_n=%d, symbol_size=%d\n",
				hdr_infos->idf_adu, hdr_infos->adu_len,
				hdr_infos->max_k, hdr_infos->max_n,
				hdr_infos->symbol_len))
		}
		if (adu == NULL) {
			/*
			 * completely new ADU, so update state and
			 * allocate the struct...
			 * if sequence number == 0 and if in FLUTE mode or
			 * object aggregation mode, do not update fsm (see NB_1 note
			 * in mcl_fsm.cpp in the receivers state machine).
			 */
			if (!(hdr_infos->idf_adu == 0 && mclcb->fsm.get_rx_state(mclcb) == RSTATE_FINISH_RX))
			{
				mclcb->fsm.update_rx_state(mclcb, REVENT_NEW_ADU);
			}
			/*
			 * Without FLUTE, always store the ADU.
			 * With FLUTE, only store the ADU if it is a FDT, or
			 * it has been requested, or all ADUs have been
			 * requested.
			 */
			if ((mclcb->is_flute_compatible() == false) ||
			    (hdr_infos->idf_adu == 0) ||
			    (mclcb->rx_flute.use_flute_store_all_adus_by_default() == true) ||
			    (mclcb->rx_flute.is_toi_requested(mclcb, hdr_infos->idf_adu) == true)) {
				adu = mcl_create_adu(mclcb);
				adu->seq = hdr_infos->idf_adu;
				adu->FDT_instanceid = hdr_infos->FDT_instanceid;
				if (mclcb->rx_window.insert_adu(mclcb, adu) == MCL_ERROR) {
					PRINT_ERR((mcl_stderr,
					"mcl_process_sig: ERROR, insert_adu() failed\n"))
					return -1;
				}
				mclcb->stats.adus_announced++;
			} else {
				/* do not store the adu */
				mclcb->rx_flute.delete_current_packet = true;
				return 0;
			}
		} else {
			/* no longer implicite */
			adu->ia_adu = 0;
		}
		adu->len = hdr_infos->adu_len;
		adu->max_k = hdr_infos->max_k;
		/* FEC codec used for this ADU */
		adu->fec_encoding_id = hdr_infos->fec_encoding_id;
		adu->fec_instance_id = hdr_infos->fec_instance_id;
		if (mclcb->fec.enc_inst_to_scheme (mclcb,
					 adu->fec_encoding_id,
					 adu->fec_instance_id,
					 &(adu->fec_scheme)) == MCL_ERROR) {
			PRINT_ERR((mcl_stderr,
			"mcl_process_sig: EXT_FTI: ERROR, unknown FEC scheme (%d/%d)\n",
				hdr_infos->fec_encoding_id,
				hdr_infos->fec_instance_id))
			return -1;
		}
		/* store src addr for mcl_recvfrom() */
		adu->addr = *saddr;
		adu->symbol_len = hdr_infos->symbol_len;
		/*
		 * compute the blocking struct
		 */
		bs = &(adu->blocking_struct);
		mcl_compute_blocking_struct(adu->max_k, adu->len,
					    adu->symbol_len, bs);
		/*
		 * and remember max_n (not always transmitted in FTI)
		 * NB: the FEC ratio is the same for all blocks of this adu
		 */
		switch (adu->fec_scheme) {
#ifdef RSE_FEC
		case MCL_FEC_SCHEME_RSE_129_0:
			adu->max_n = hdr_infos->max_n;
			fec_ratio = (double)adu->max_n / (double)adu->max_k;
			mclcb->fec.set_fec_code(mclcb,
					MCL_FEC_SCHEME_RSE_129_0);
			break;
#endif /* RSE_FEC */

#ifdef LDPC_FEC
		case MCL_FEC_SCHEME_LDGM_STAIRCASE_132_0:
			adu->max_n = hdr_infos->max_n;
			fec_ratio = (double)adu->max_n / (double)adu->max_k;
			mclcb->fec.set_fec_code(mclcb,
					MCL_FEC_SCHEME_LDGM_STAIRCASE_132_0);
			break;

		case MCL_FEC_SCHEME_LDGM_TRIANGLE_132_1:
			adu->max_n = hdr_infos->max_n;
			fec_ratio = (double)adu->max_n / (double)adu->max_k;
			mclcb->fec.set_fec_code(mclcb,
					MCL_FEC_SCHEME_LDGM_TRIANGLE_132_1);
			break;
#endif /* LDPC_FEC */

		case MCL_FEC_SCHEME_NULL:
			adu->max_n = adu->max_k;
			mclcb->fec.set_fec_code(mclcb, MCL_FEC_SCHEME_NULL);
			break;

		default:
			/* NB: FEC Enc. ID 128 no longer supported */
			mcl_exit(-1);
		}

		/*
		 * remember them just in case. Do not get confused by the
		 * ADU max_k/n values and the FEC codec maximum k/n values.
		 */
		mclcb->fec.set_k(adu->max_k);
		mclcb->fec.set_n(adu->max_n);
		/*
		 * allocate the block structs...
		 * do it simply: allocate a tab of block_t structs
		 * rather than a linked list!
		 */
		tot_blk_nb = bs->block_nb;
		if (!(blk = (block_t*)calloc(tot_blk_nb, sizeof(block_t)))) {
			PRINT_ERR((mcl_stderr,
				"mcl_process_sig: ERROR, no memory\n"))
			mcl_exit(-1);
		}
		adu->block_head = blk;
		/* ... and now initialize all the blocks */
		rem = adu->len;
		for (blk_seq = 0; blk_seq < tot_blk_nb; blk_seq++) {
			if (blk_seq < bs->I)
				k_for_this_blk = bs->A_large;
			else
				k_for_this_blk = bs->A_small;
			blk->adu = adu;
			blk->seq = blk_seq;
			blk->k   = k_for_this_blk;
			blk->len = min(rem, k_for_this_blk * adu->symbol_len);
			switch (adu->fec_scheme) {
#ifdef RSE_FEC			
			case MCL_FEC_SCHEME_RSE_129_0:
				blk->n = (UINT32)floor((double)blk->k * fec_ratio);
				break;
#endif /* RSE_FEC */
#ifdef LDPC_FEC
			case MCL_FEC_SCHEME_LDGM_STAIRCASE_132_0:
			case MCL_FEC_SCHEME_LDGM_TRIANGLE_132_1:
				/*
				 * WARNING: due to the way fec_key is passed
				 * to receiver, the same value is used for
				 * all blocks of an ADU
				 */
				blk->fec_key = hdr_infos->fec_key;
				blk->n = (UINT32)floor((double)blk->k * fec_ratio);
				TRACELVL(4, (mcl_stdout,
					"   New ADU: LDPC/LDGM blk_seq=%d: key=%d, (k;n)=(%d;%d) fec_ratio=%.3f\n",
					blk->seq, blk->fec_key, blk->k, blk->n,
					(float)fec_ratio))
				break;
#endif /* LDPC_FEC */
			default:
				ASSERT(adu->fec_scheme == MCL_FEC_SCHEME_NULL);
				blk->n = blk->k;	/* no FEC here */
				break;
			}
			/*
			 * blk->du_head, fec_du_head, fec_du_nb_in_list, du_rx
			 * already set to NULL/0
			 */
			rem -= blk->len;
			blk++;	/* switch to next block */
		}

		TRACELVL(4, (mcl_stdout, "   New ADU: %d blocks, max_block_len=%d (%d DUs), min_block_len=%d (%d DUs)\n",
			adu->blocking_struct.block_nb, adu->block_head->len, adu->block_head->k,
			(blk-1)->len, (blk-1)->k))
		break;
		}

	case EXT_NONEWADU: {
		u_int	seq;

		TRACELVL(5, (mcl_stdout, "-> mcl_process_sig: EXT_NONEWADU (%d)\n", type))
		/* this sig header must not be a duplicate */
		ASSERT(!mclcb->fsm.no_new_adu(mclcb));
		ASSERT(hdr_infos->max_idf_adu >= mcl_iss);
		if (mclcb->get_verbosity() >= 1)
			mcl_print_recvd_du(mclcb, EH_SIG, type, hdr_infos);
		ASSERT(mclcb->is_flute_compatible() == false);
		mclcb->fsm.update_rx_state(mclcb, REVENT_NO_NEW_ADU);
		/*
		 * check if there are implicit declarations of NEWADU,
		 * i.e. ADUs that we know exist thanks to the max_adu_idf
		 * info of NONEWADU, but for which we did not receive any
		 * packet yet...
		 */
		/* search more efficient in ascending ADU seq number order */
		for (seq = mcl_iss; seq <= hdr_infos->max_idf_adu; seq++) {
			adu = mclcb->rx_window.find_adu(mclcb, seq, 0);
			if (!adu) {
				adu = mcl_create_adu(mclcb);
				mclcb->stats.adus_announced++;
				adu->seq = seq;
				adu->FDT_instanceid = 0;
				adu->ia_adu = 1; /* implicitely announced */
				if (mclcb->rx_window.insert_adu(mclcb, adu) == MCL_ERROR) {
					PRINT_ERR((mcl_stderr,
					"mcl_process_sig: ERROR, insert_adu() failed\n"))
					return -1;
				}
			}
		}
		/*
		 * did we receive everything ?
		 */
		if (mclcb->rx_window.check_if_all_adu_completed(mclcb) == true) {
			if (mclcb->get_verbosity() >= 1)
				PRINT_OUT((mcl_stdout, "\nAll ADUs received\n"))
			/*
			 * the receiver has finished
			 */
			mclcb->fsm.update_rx_state(mclcb, REVENT_ALL_DU_RECV);
			if (mclcb->get_stats_level() >= 1) {
				mcl_print_rx_stats(mclcb);
				mcl_print_final_stats(mclcb);
			}
			/*
			 * unsubscribe to all layers (incl. layer 0) to avoid
			 * receiving useless packets
			 */
			mcl_drop_layer(mclcb, MCL_ALL_LAYERS, MCL_DO_IT);
#ifdef RSE_FEC
			/*
			 * we can now decode all ADUs if in postpone mode
			 */
			if (mclcb->postpone_fec_decoding) {
				mcl_decode_all_adu(mclcb);
			}
#endif /* RSE_FEC */
		} /* otherwise wait... */
		break;
		}

	case SIG_CLOSE: {
		TRACELVL(5, (mcl_stdout, "-> mcl_process_sig: SIG_CLOSE (%d)\n", type))
		if (mclcb->fsm.close_already_rx(mclcb)) {
			/* this is a duplicated announce; ignore */
			break;
		}
		if (mclcb->get_verbosity() >= 2)
			mcl_print_recvd_du(mclcb, EH_SIG, type, hdr_infos);
		mclcb->fsm.update_rx_state(mclcb, REVENT_CLOSE_RECV);
		ASSERT(mclcb->fsm.no_new_adu(mclcb));
		if (mclcb->get_stats_level() >= 1) {
			mcl_print_rx_stats(mclcb);
			mcl_print_final_stats(mclcb);
		}
		break;
		}

	default:
		PRINT_ERR((mcl_stderr,
			"mcl_process_sig: ERROR, unknown SIG type: %d\n", type))
		return -1;
	}
	/* update stats */
	//mclcb->stats.rx_sig_pkts++; /* there is a SIG header */
	TRACELVL(5, (mcl_stdout, "<- mcl_process_sig: ok\n"))
	return 0;
}


#ifdef RSE_FEC /* { */
/*
 * we have just finished to receive everything. Now make sure that
 * all blocks of all ADUs are actually decoded.
 * Used in postpone_fec_decoding mode after having received everything.
 * Specific to RSE !
 */
static void
mcl_decode_all_adu (mcl_cb	*mclcb)
{
	adu_t		*adu;
	adu_t		*first;

	TRACELVL(5, (mcl_stdout, "-> mcl_decode_all_adu:\n"))
	ASSERT(mclcb->postpone_fec_decoding);
	first = adu = mclcb->rx_window.get_first_adu(mclcb);
	do {
		ASSERT(adu);
		adu = adu->prev;
		if (adu->fec_scheme == MCL_FEC_SCHEME_RSE_129_0) {
			/* postponed decoding only applies to RSE */
			if (adu->rx_status == ADU_STATUS_COMPLETED) {
				mcl_decode_this_adu(mclcb, adu);
			} /* else nothing to do */
		}
		/* nothing to do with other FEC codes */
	} while (adu != first);
	TRACELVL(5, (mcl_stdout, "<- mcl_decode_all_adu: ok\n"))
}


/**
 * RSE specific function.
 */
static void
mcl_decode_this_adu (mcl_cb	*mclcb,
		     adu_t	*adu)
{
	block_t		*blk;
	int		i;

	ASSERT(adu->rx_status == ADU_STATUS_COMPLETED);
	ASSERT(adu->fec_scheme == MCL_FEC_SCHEME_RSE_129_0);
	for (i = adu->blocking_struct.block_nb, blk = adu->block_head; i > 0; i--, blk++) {
		if (blk->rx_status == BLK_STATUS_DECODED)
			continue;
		if (mclcb->fec.decode(mclcb, blk) < 0) {
			PRINT_ERR((mcl_stderr, "mcl_decode_this_adu: ERROR, decode() failed\n"))
			mcl_exit(-1);
		}
	}
#ifdef GET_SYSINFO
	mcl_print_sysinfo(mclcb);
#endif
	adu->rx_status = ADU_STATUS_DECODED;
}
#endif /* } RSE_FEC */


