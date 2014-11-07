/* $Id: mcl_adu.cpp,v 1.20 2005/03/18 12:06:15 roca Exp $ */
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


/*
 * Create and init a new adu struct.
 */
adu_t *
mcl_create_adu (mcl_cb *mclcb)
{
	adu_t	*adu;

	if (!(adu = (adu_t*)calloc(1, sizeof(adu_t)))) {
		PRINT_ERR((mcl_stderr, "mcl_create_adu: ERROR, no memory"))
		mcl_exit(-1);
	}
	adu->addr.reset();
	return (adu);
}


/*
 * Insert the ADU in the adu list.
 */
void
mcl_insert_adu (mcl_cb	*mclcb,
		adu_t	*adu,
		adu_t	**list)
{
	adu_t	*adul;

	ASSERT(adu && list);
	TRACELVL(5, (mcl_stdout, "-> mcl_insert_adu: adu=x%x, seq=%d\n", (int)adu, adu->seq))
	if (!(*list)) {
		/*
		 * first adu in list
		 */
		*list = adu;
		adu->next = adu->prev = adu;
		TRACELVL(5, (mcl_stdout, "<- mcl_insert_adu: only one in list\n"))
		return;
	}
	adul = (*list)->prev;
	ASSERT(adul);
	/*
	 * start from the highest seq number
	 */
	for (; adu->seq < adul->seq; adul = adul->prev) {
		if (adul == *list) {
			/* we have cycled, so adu must be the first of list */
			*list = adu;
			adu->next = adul;
			adu->prev = adul->prev;
			adul->prev->next = adu;
			adul->prev = adu;
			TRACELVL(5, (mcl_stdout, "<- mcl_insert_adu: at start of list\n"))
			return;
		}
	}
	//ASSERT(adu->seq != adul->seq);	/* no duplicated adu for the present */
	adu->next = adul->next;
	adul->next->prev = adu;
	adul->next = adu;
	adu->prev = adul;
	TRACELVL(5, (mcl_stdout, "<- mcl_insert_adu: ok\n"))
}


/*
 * Remove an ADU from the adu list
 */
void
mcl_remove_adu (mcl_cb	*mclcb,
		adu_t	*adu,
		adu_t	**list)
{
	adu_t	*prev, *next;

	ASSERT(adu && list);
	TRACELVL(5, (mcl_stdout, "-> mcl_remove_adu: adu=x%x, seq=%d\n", (int)adu,adu->seq))
	ASSERT(*list);
	prev = adu->prev;
	next = adu->next;
	if (prev == adu) {	/* only one ADU in list */
		ASSERT(next == adu);
		*list = NULL;	/* list is now empty */
	} else {
		prev->next = next;
		next->prev = prev;
		if (*list == adu) {
			*list = next;	/* adu was the first in list */
		}
	}
	TRACELVL(5, (mcl_stdout, "<- mcl_remove_adu:\n"))
}


/*
 * Returns 1 if the adu has JUST been completely received (decoded or not),
 * 0 otherwise.
 */
int
mcl_rx_new_completed_adu (mcl_cb	*mclcb,
		 adu_t		*adu)
{
	block_t			*blk;
	int			i;
	mcl_adu_rx_status	new_status;

	TRACELVL(5, (mcl_stdout, "-> mcl_rx_new_completed_adu:\n"))
	ASSERT(adu);	/* for tests, but could happen in case of tx errors */
	if (adu->rx_status >= ADU_STATUS_COMPLETED) {
		/* in COMPLETED, DECODED, or DELIVERED mode */
		/* we have already recvd everything */
		TRACELVL(5, (mcl_stdout, "<- mcl_rx_new_completed_adu: already completed\n"))
		return 0;
	}
	new_status =  ADU_STATUS_DECODED;
	for (i = adu->blocking_struct.block_nb, blk = adu->block_head; i > 0; i--, blk++) {
		if (blk->rx_status != BLK_STATUS_COMPLETED &&
		    blk->rx_status != BLK_STATUS_DECODED) {
			TRACELVL(5, (mcl_stdout, "<- mcl_rx_new_completed_adu: no\n"))
			return 0;
		}
		if (blk->rx_status == BLK_STATUS_COMPLETED) {
			/* at least one blk is not decoded, remember it */
			new_status = ADU_STATUS_COMPLETED;
		}
		if (i == 1) {
			/* we have cycled, yes this is a new completed ADU! */
			break;
		}
	}
	/*
	 * we have everything. Update status and stats...
	 */
	adu->rx_status = new_status;
	mclcb->stats.finish_times[mclcb->stats.finish_index][STATS_FIN_TIME] = mcl_time_count;
	mclcb->stats.finish_times[mclcb->stats.finish_index][STATS_SEQ] = adu->seq;
	if (++(mclcb->stats.finish_index) == MAX_FIN_TIME)
		mclcb->stats.finish_index = 0;
	mclcb->stats.adus_compl++;
	TRACELVL(5, (mcl_stdout, "<- mcl_rx_new_completed_adu: yes\n"))
	return 1;
}


/*
 * Returns NULL if this ADU has never been announced before, the
 * adu_t descriptor otherwise.
 * Used only by the receivers.
 * If parameter FDT_instanceid is -1, don't care for instance ID when searching
 * ADU, and take the first ADU with the specified sequence number seq
 */
adu_t*
mcl_find_adu (mcl_cb	*mclcb,
		UINT32	seq,
		INT32	FDT_instanceid,
		adu_t	*list)
{
	adu_t		*adu;

	TRACELVL(5, (mcl_stdout, "-> mcl_find_adu: seq=%d\n", seq))
	if (!list)  {
		TRACELVL(5, (mcl_stdout, "<- mcl_find_adu: empty list\n"))
		return NULL;
	}
	if (mclcb->findadu_cache && mclcb->findadu_cache->seq == seq &&
	    ((INT32) mclcb->findadu_cache->FDT_instanceid == FDT_instanceid || FDT_instanceid == -1)) {
		TRACELVL(5, (mcl_stdout, "<- mcl_find_adu: found in cache\n"))
		return mclcb->findadu_cache;
	}
	/*
	 * start from the highest seq number
	 */
	adu = list->prev;
	while (1) {
		ASSERT(adu);
		if (adu->seq == seq && ((INT32) adu->FDT_instanceid == FDT_instanceid || FDT_instanceid == -1)) {
			/* found */
			mclcb->findadu_cache = adu;
			TRACELVL(5, (mcl_stdout, "<- mcl_find_adu: found\n"))
			return adu;
		} 
		if (adu == list || adu->seq < seq) {
			/* we have cycled or new adu cannot be in list */
			TRACELVL(5, (mcl_stdout, "<- mcl_find_adu: new\n"))
			return NULL;
		}
		adu = adu->prev;
	}
}


/*
 * Returns NULL if this block has never been announced before, the
 * block_t descriptor otherwise.
 * Used only by the receivers.
 */
block_t*
FindBlock (mcl_cb	*mclcb,
	   UINT32	seq,
	   block_t	*list,		/* in fact this is a tab, not a list! */
	   adu_t	*adu)
{
  	TRACELVL(5, (mcl_stdout, "-> FindBlock: seq=%d\n", seq))
	if (!list)
		return NULL;
	if (seq < 0 || seq > adu->blocking_struct.block_nb) {
	  TRACELVL(5, (mcl_stdout, "<- FindBlock: ERROR, out of bound\n"))
		return NULL;
	} else {
	  TRACELVL(5, (mcl_stdout, "<- FindBlock: found\n"))
		return (list + seq);
	}
}


/*
 * Returns the highest ADU TOI of this list.
 */
int
mcl_get_highest_toi (mcl_cb	*mclcb,
		      adu_t	*list)
{
	ASSERT(list);
	return list->prev->seq;
}


/*
 * Segment the ADU in blocks (ie. block_t) and packets (ie. du_t)
 * Static info of the adu struct (seq, len, symbol_size) must be
 * initialized upon calling this function.
 * Used only by the sender.
 */
void
mcl_tx_segment_adu (mcl_cb	*mclcb,
	    adu_t	*adu)
{
	block_t		*blk;
	du_t		*du;
	UINT32	tot_blk_nb;	/* total nb of blocks required */
	UINT32	blk_seq;	/* block seq number */
	UINT32	tot_du_nb;	/* total nb of DUs required */
	UINT32	k_for_this_blk;
	UINT32	i;
#ifdef LDPC_FEC
	UINT32	fec_key;
#endif
	int		rem;
	char		*ptr;
	mcl_blocking_struct_t	*bs;

	/*
	 * max blk size in DU, and max_n. Both values depend on the FEC codec.
	 * remember it, it will be used for EXT_FTI generation
	 */
	adu->max_k = mclcb->fec.get_k();
	adu->max_n = mclcb->fec.get_n();
	/*
	 * compute the blocking struct
	 */
	bs = &(adu->blocking_struct);
	mcl_compute_blocking_struct(adu->max_k, adu->len, adu->symbol_len, bs);
	/*
	 * segment the ADU...
	 * do it simply: allocate a tab of du_t and block_t structs
	 * rather than two linked lists!
	 */
	tot_du_nb = (int)ceil((double)adu->len / (double)adu->symbol_len);
	ASSERT(tot_du_nb == bs->I * bs->A_large + (bs->block_nb - bs->I) * bs->A_small);
	if (!(du = (du_t*)calloc(tot_du_nb, sizeof(du_t)))) {
		goto no_memory;
	}
	tot_blk_nb = bs->block_nb;
	ASSERT(tot_blk_nb == (UINT32)ceil((double)tot_du_nb / (double)adu->max_k));
	if (!(blk = (block_t*)calloc(tot_blk_nb, sizeof(block_t)))) {
		goto no_memory;
	}
	adu->block_head = blk;

#ifdef LDPC_FEC
	/* 
	 * All blocks of one ADU must use the same FEC key! 
	 * Because of FTI limitations (only one value carried).
	 */
	fec_key = random(); /* 32bit random value */
#endif

	ptr = adu->data;
	rem = adu->len;
	for (blk_seq = 0; blk_seq < tot_blk_nb; blk_seq++) {
		if (blk_seq < bs->I)
			k_for_this_blk = bs->A_large;
		else
			k_for_this_blk = bs->A_small;
		blk->adu = adu;
		blk->seq = blk_seq;
		blk->du_head = du;
		blk->k = k_for_this_blk;
#ifdef LDPC_FEC
		blk->fec_key = fec_key;
#endif
		/* blk->fec_du_head and fec_du_nb_in_list already set to 0 */
		for (i = 0; i < k_for_this_blk; i++, du++) {
			du->block = blk;
			du->seq = i;
			du->len = min(rem, mclcb->get_payload_size());
#ifdef VIRTUAL_TX_MEM
			du->vtm_info.du_in_seq_in_txtab = 1; /*default*/
			if (adu->vtm_info.in_vtm) {
				ASSERT(mclcb->vtm_used);
				mcl_vtm_register_du(mclcb, du,
						    (UINT32)(ptr - adu->data));
				/* also sets du->data to NULL */
			} else
#endif /* VIRTUAL_TX_MEM */
			du->data = ptr;
			ptr += du->len;
			rem -= du->len;
			blk->len += du->len; /* last DU may be non full-sized */
		}
		blk++;	/* switch to next block */
	}
	ASSERT(rem == 0);
	if (mclcb->get_verbosity() >= 3) {
		PRINT_OUT((mcl_stdout,
			"New ADU: seq=%d, len=%d, max_k=%d, symbol_size=%d\n",
			adu->seq, adu->len,
			adu->block_head->k, adu->symbol_len))
#if defined(LDPC_FEC) && defined(DEBUG)
		if (mclcb->fec.get_fec_code() ==
					MCL_FEC_SCHEME_LDGM_STAIRCASE_132_0 ||
		    mclcb->fec.get_fec_code() ==
					MCL_FEC_SCHEME_LDGM_TRIANGLE_132_1) {
			TRACELVL(4, (mcl_stdout,
				"   New ADU: LDPC/LDGM key=%d\n",
				adu->block_head->fec_key))
		}
#endif
		TRACELVL(4, (mcl_stdout, "   New ADU: %d blocks, max_block_len=%d (%d DUs), last_block_len=%d (%d DUs)\n",
			adu->blocking_struct.block_nb, adu->block_head->len, adu->block_head->k,
			(blk-1)->len, (blk-1)->k))
	}
	return;

no_memory:
	PRINT_ERR((mcl_stderr, "mcl_tx_segment_adu: ERROR, no memory"))
	mcl_exit(-1);
}


/*
 * Free all buffers for this transmission ADU.
 * WARNING: this is the sending side function, do not use for a receiver!
 * WARNING: does not remove the ADU struct from the linked list of ADUs
 * WARNING: does not free the ADU buffer
 */
void
mcl_tx_free_this_adu (mcl_cb	*mclcb,
		      adu_t	*adu)
{
	block_t		*blk;
	UINT32		i;
#ifdef FEC
	UINT32		j;
	du_t		*du;
#endif /* FEC */

	ASSERT(adu);
	TRACELVL(5, (mcl_stdout, "-> mcl_tx_free_this_adu: adu=x%x, aseq=%d\n",
		(int)adu, adu->seq))
	ASSERT(mclcb->is_a_sender());
	/*
	 * step1: free all DUs from each block
	 */
	for (i = 0, blk = adu->block_head; i < adu->blocking_struct.block_nb; i++, blk++) {
		/* no data block here, only points in the global data block */
		ASSERT((blk))
		if (blk->k > 0) {
			if (i == 0) {
				/* in fact single du table (ie malloc) */
				ASSERT(blk->du_head);
				free(blk->du_head);
			}
			blk->du_head = NULL;
			blk->k = 0;
		}
#ifdef FEC
		/*
		 * FEC DUs data blocks are allocated in several independant
		 * mallocs, but the FEC DU structs in a single block.
		 */
		for (j = 0,  du = blk->fec_du_head; j < blk->fec_du_nb_in_list;
		     j++, du++) {
			ASSERT((du))
#ifdef VIRTUAL_TX_MEM
			if (!mclcb->vtm_used || !du->vtm_info.in_vtm)
#endif
			{
				ASSERT((du->data))
				free(du->data);
				du->data = NULL;
			}
		}
		if (blk->fec_du_head) {
			free(blk->fec_du_head);
			blk->fec_du_head = NULL;
			blk->fec_du_nb_in_list = 0;
		}
#endif /* FEC */
	}
	/*
	 * step2: free the block table
	 */
	if (adu->blocking_struct.block_nb > 0) {
		ASSERT((adu->block_head))
		free(adu->block_head);
		adu->block_head = NULL;
		adu->blocking_struct.block_nb = 0;
	}
	/*
	 * step3: free data buffer
	 */
#ifdef VIRTUAL_TX_MEM
	if (!mclcb->vtm_used || !adu->vtm_info.in_vtm)
#endif
	{
		if (adu->data) {
			free(adu->data);
			adu->data = NULL;
			mclcb->stats.buf_space -= adu->len;	/* update stats */
		}
	}
	TRACELVL(5, (mcl_stdout, "<- mcl_tx_free_this_adu: ok\n"))
}


/**
 * Free all buffers for this reception ADU.
 * WARNING: this is the receiving side function, do not use for a sender!
 * WARNING: does not remove the ADU struct from the linked list of ADUs
 * WARNING: does not free the ADU buffer
 */
void
mcl_rx_free_this_adu (mcl_cb	*mclcb,
		      adu_t	*adu)
{
	block_t		*blk;
	UINT32		i;
#ifdef FEC
	UINT32		j;
	du_t		*du;
#endif /* FEC */

	ASSERT(adu);
	TRACELVL(5, (mcl_stdout, "-> mcl_rx_free_this_adu: adu=x%x, aseq=%d\n",
		(int)adu, adu->seq))
	ASSERT(mclcb->is_a_receiver());
	/*
	 * step1: free all DUs from each block
	 */
	for (i = 0, blk = adu->block_head; i < adu->blocking_struct.block_nb; i++, blk++) {
		/*
		 * FEC DUs data blocks and source DUs data blocks are
		 * allocated separately, upon packet reception/decoding.
		 */
#ifdef FEC
		for (j = 0,  du = blk->fec_du_head; j < blk->fec_du_nb_in_list;
		     j++, du = du->next) {
			ASSERT((du))
			mclcb->rx_storage.free_data(mclcb, du);
		}
#endif /* FEC */
		for (j = 0,  du = blk->du_head; j < blk->du_nb_in_list;
		     j++, du = du->next) {
			ASSERT((du))
			mclcb->rx_storage.free_data(mclcb, du);
		}
	}
	/*
	 * step2: free the block table
	 */
	if (adu->blocking_struct.block_nb > 0) {
		ASSERT((adu->block_head))
		free(adu->block_head);
		adu->block_head = NULL;
		adu->blocking_struct.block_nb = 0;
	}
	TRACELVL(5, (mcl_stdout, "<- mcl_tx_free_this_adu: ok\n"))
}


#if 0
/**
 * Free all ADUs.
 */
void
mcl_free_all_adu (mcl_cb	*mclcb)
{
	adu_t	*adu;
	txlay_t	*tl;
	//int	i;

	if (mclcb->is_a_sender()) {
		mclcb->tx.free_all_adu(mclcb);
#if 0
		/* ADUs are only stored on the base txlvl structure */
		tl = mclcb->txlay_tab;
		while ((adu = tl->adu_head) != NULL) {
			mcl_remove_adu(mclcb, adu, &(tl->adu_head));
			mcl_tx_free_this_adu(mclcb, adu);
			free(adu);	/* and free the buffer */
		}
#endif
	}
	if (mclcb->is_a_receiver()) {
		/* nothing yet */
		/* XXX: TODO ! */
	}
}
#endif

