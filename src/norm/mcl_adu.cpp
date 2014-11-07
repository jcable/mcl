/* $Id: mcl_adu.cpp,v 1.4 2004/02/18 07:56:18 roca Exp $ */
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


/**
 * Default constructor USED BY THE SENDER.
 */
mcl_adu::mcl_adu (mcl_cb	*const mclcb,
		  INT32		const alen,
		  mcl_addr	const* saddr)
{
	TRACELVL(5, (mcl_stdout, "-> mcl_adu::mcl_adu: TX, len=%d\n", len))
	memset(this, 0, sizeof(*this));
	this->type = MCL_TX;
	this->len = alen;
	this->tx_status = ADU_TSTATUS_IN_PROGRESS;
	//this->data = NULL;
	this->seq = mclcb->tx.get_seq_for_new_adu();
	if (saddr != NULL) {
		// use the provided address
		this->addr = *saddr;
	} else {
		// else use the default session addr
		this->addr = mclcb->ses_channel.ses_addr;
	}
	// calculate and set the padded length
	INT32 plen = (INT32)
		(ceil((double)alen / (double)mclcb->get_payload_size())
		 * (double)mclcb->get_payload_size());
	this->padded_len = plen;
	TRACELVL(4, (mcl_stdout, "   mcl_adu::mcl_adu: NEW ADU seq=%d, len =%d\n",
		this->seq, this->len))
	TRACELVL(5, (mcl_stdout, "<- mcl_adu::mcl_adu:\n"))
}


/**
 * Default constructor USED BY THE RECEIVER.
 */
mcl_adu::mcl_adu (mcl_cb		*const mclcb,
		  mcl_data_hdr_infos_t	const* dhdr_infos,
		  mcl_addr		const* saddr)
{
	mcl_block	*blk;
	UINT32		rem;
	UINT32		bseq;	// block sequence number

	TRACELVL(5, (mcl_stdout, "-> mcl_adu::mcl_adu: RX\n"))
	memset(this, 0, sizeof(*this));
	this->type = MCL_RX;
	this->seq = dhdr_infos->idf_adu;
	this->len = dhdr_infos->adu_len;
	this->rx_status = ADU_STATUS_IN_PROGRESS;
	//this->data = NULL;
	if (saddr != NULL) {
		// use the provided address
		this->addr = *saddr;
	} // else zero'ed address
	/*
	 * NON-NORM-COMPLIANT:
	 * assume (1) block_len is for full-size block and (2) is in bytes
	 */
	this->full_size_block_len = dhdr_infos->block_len;
	/*
	 * calculate the number of DUs for each block and
	 * allocate the block structs...
	 * do it simply: allocate a tab of block_t structs
	 * rather than a linked list!
	 */
	this->block_nb = (int)ceil((double)this->len /
				  (double)this->full_size_block_len);
	blk = new mcl_block [this->block_nb];// no mcl_block default constructor
					    // for performance reasons!
	ASSERT(blk);
	memset(blk, 0, sizeof(mcl_block) * this->block_nb);
	this->block_head = blk;
	rem = this->len;
	for (bseq = 0; (INT32)bseq < this->block_nb; bseq++, blk++) {
		blk->set_rx_status(BLK_STATUS_IN_PROGRESS);
		blk->adu = this;
		blk->seq = bseq;
		blk->len = min(rem, dhdr_infos->block_len);
		blk->du_nb = (int)ceil((double)blk->len /
				       (double)mclcb->get_payload_size());
		/*
		 * blk->du_head, fec_du_head, fec_du_nb, du_rx
		 * already set to NULL/0
		 */
		rem -= blk->len;
	}
	TRACELVL(1, (mcl_stdout,
		"New ADU: seq=%d, len=%d, composed of %d blks at most %d bytes/%d DUs long\n",
		this->seq, this->len, this->block_nb,
		this->full_size_block_len, this->block_head->du_nb))
	TRACELVL(5, (mcl_stdout, "<- mcl_adu::mcl_adu:\n"))
}


/**
 * Default destructor.
 */
mcl_adu::~mcl_adu ()
{
	this->remove_and_free_all_buffers(NULL);
}


/**
 */
void
mcl_adu::remove_and_free_all_buffers (mcl_cb	*const mclcb)
{
	UINT32		i;
	mcl_block	*blk;

	if (this->type == MCL_TX) {
		/* Sender */
		/* TODO: check there are no remaining references to this adu */
		if (this->block_nb > 0) {
			/* free FEC dus created in each block */
			for (i = this->block_nb, blk = this->block_head;
			     i > 0; i--, blk++) {
				blk->remove_and_free_all_fec_dus (mclcb);
			}
			/* free all blk and du descriptors for this adu */
			ASSERT(this->block_head->get_du_head());
			delete [] this->block_head->get_du_head();
			this->block_head->set_du_head(NULL);
			ASSERT(this->block_head);
			delete [] this->block_head;
			this->block_head = NULL;
			this->block_nb = 0;
			/* free buffer containing adu data */
			ASSERT(this->data);
			free(this->data);
			this->data = NULL;
		}
	} else {
		/* Receiver */
		ASSERT(this->type == MCL_RX);
		if (this->block_nb > 0) {
			/* free all DUs in each block */
			for (i = this->block_nb, blk = this->block_head;
			     i > 0; i--, blk++) {
				blk->remove_and_free_all_fec_dus (mclcb);
				blk->remove_and_free_all_data_dus (mclcb);
			}
			/* free all blocks and du descriptors for this adu */
			ASSERT(this->block_head);
			delete [] this->block_head;
			this->block_head = NULL;
			this->block_nb = 0;
			this->set_tx_status(ADU_TSTATUS_DONE);
		} else {
			ASSERT(this->get_tx_status() == ADU_TSTATUS_DONE);
		}
	}
}


/**
 * Check if this adu is JUST completed and take appropriate measures.
 * Used by a receiver when a new non-duplicated du is received for
 * a block.
 * @param mclcb
 * @return	true if completed and appropriate measures taken,
 * 		false otherwise.
 */
bool
mcl_adu::check_if_completed_and_process (mcl_cb		*const mclcb)
{
	TRACELVL(5, (mcl_stdout,
		"-> mcl_adu::check_if_completed_and_process:\n"))
	if (this->check_if_just_completed(mclcb)) {
		TRACELVL(1, (mcl_stdout, "End of ADU %d\n", this->seq))
		if (mclcb->get_stats_level() == 2)
			mclcb->stats.print_rx_stats(mclcb);
		if (!mclcb->rx_window.postpone_fec_decoding &&
		    this->rx_status == ADU_STATUS_COMPLETED) {
			/* do not wait, decode every block of this adu */
			if (this->decode_all_blocks(mclcb) == MCL_ERROR) {
				mcl_exit(-1);
			}
		}
		mclcb->rx_window.mark_ready_adu();
		if (mclcb->fsm.no_new_adu(mclcb) &&
		    mclcb->rx_window.check_if_all_adu_completed(mclcb)) {
			/*
			 * we know we won't receive any new ADU and
			 * it was the last packet we were waiting for...
			 */
			TRACELVL(1, (mcl_stdout, "All ADUs received\n"))
			mclcb->fsm.update_rx_state(mclcb, REVENT_ALL_DU_RECV);
			if (mclcb->get_stats_level() >= 1) {
				mclcb->stats.print_rx_stats(mclcb);
				mclcb->stats.print_final_stats(mclcb);
			}
#if 0
			/*
			 * unsubscribe to all layers (incl. layer 0) to avoid
			 * receiving useless packets
			 */
			mcl_drop_layer(mclcb, MCL_ALL_LAYERS, MCL_DO_IT);
			/*
			 * we can now decode all ADUs if in postpone mode
			 */
			if (mclcb->postpone_fec_decoding) {
				mcl_decode_all_adu(mclcb);
			}
#endif
		}
		TRACELVL(5, (mcl_stdout,
			"<- mcl_adu::check_if_completed_and_process: yes\n"))
		return true;
	} else {
		TRACELVL(5, (mcl_stdout,
			"<- mcl_adu::check_if_completed_and_process: no\n"))
		return false;
	}
}


/*
 * @returns	true if the adu has JUST been completely received (decoded
 * 		or not), false otherwise.
 */
bool
mcl_adu::check_if_just_completed (mcl_cb	*const mclcb)
{
	mcl_block		*blk;
	INT32			i;
	mcl_adu_rx_status	new_status;

	TRACELVL(5, (mcl_stdout, "-> mcl_adu::check_if_just_completed:\n"))
	if (this->rx_status >= ADU_STATUS_COMPLETED) {
		/* in COMPLETED, DECODED, or DELIVERED mode */
		/* we have already recvd everything */
		TRACELVL(5, (mcl_stdout, "<- mcl_adu::check_if_just_completed: already completed\n"))
		return false;
	}
	new_status =  ADU_STATUS_DECODED;
	for (i = this->block_nb, blk = this->block_head; i > 0; i--, blk++) {
		if (blk->get_rx_status() != BLK_STATUS_COMPLETED &&
		    blk->get_rx_status() != BLK_STATUS_DECODED) {
			TRACELVL(5, (mcl_stdout, "<- mcl_adu::check_if_just_completed: no\n"))
			return false;
		}
		if (blk->get_rx_status() == BLK_STATUS_COMPLETED) {
			/* at least one blk is not decoded, remember it */
			new_status = ADU_STATUS_COMPLETED;
		}
		//if (i == 1) {
		//	/* we have cycled, yes this is a new completed ADU! */
		//	break;
		//}
	}
	/*
	 * we have everything. Update status and stats...
	 */
	this->rx_status = new_status;
	mclcb->stats.finish_times[mclcb->stats.finish_index][STATS_FIN_TIME] = mcl_time_count;
	mclcb->stats.finish_times[mclcb->stats.finish_index][STATS_SEQ] = this->seq;
	if (++(mclcb->stats.finish_index) == MAX_FIN_TIME)
		mclcb->stats.finish_index = 0;
	mclcb->stats.adus_completed++;
	TRACELVL(5, (mcl_stdout, "<- mcl_adu::check_if_just_completed: yes\n"))
	return true;
}


/**
 * Decode all the block of an ADU.
 */
mcl_error_status
mcl_adu::decode_all_blocks (mcl_cb	*const mclcb)
{
	mcl_block	*blk;
	int		i;

	ASSERT(this->rx_status == ADU_STATUS_COMPLETED)
	for (i = this->block_nb, blk = this->block_head; i > 0; i--, blk++) {
		if (blk->get_rx_status() == BLK_STATUS_DECODED)
			continue;
		if (mclcb->fec.decode(mclcb, blk) < 0) {
			PRINT_ERR((mcl_stderr,
			"mcl_adu::decode_all_blocks: ERROR, decode failed\n"))
			return MCL_ERROR;
		}
	}
#ifdef GET_SYSINFO
	mcl_print_sysinfo(mclcb);
#endif
	this->rx_status = ADU_STATUS_DECODED;
	return MCL_OK;
}


mcl_error_status
mcl_adu::insert_in_list (mcl_cb		*const mclcb,
			 mcl_adu	**head)
{
	mcl_adu		*adul;	// temp pointer to adus currently in the list

	ASSERT(head);
	TRACELVL(5, (mcl_stdout,
		"-> mcl_adu::insert_in_list: adu=x%x, seq=%d\n",
		(int)this, this->seq))
	if (!(*head)) {
		/*
		 * first adu in list
		 */
		*head = this;
		this->next = this->prev = this;
		TRACELVL(5, (mcl_stdout,
			"<- mcl_adu::insert_in_list: ok, inserted in empty list\n"))
		return MCL_OK;
	}
	adul = (*head)->prev;
	ASSERT(adul);
	/*
	 * start from the highest seq number
	 */
	for (; this->seq < adul->seq; adul = adul->prev) {
		if (adul == *head) {
			/* we have cycled, so adu must be the first of list */
			*head = this;
			this->next = adul;
			this->prev = adul->prev;
			adul->prev->next = this;
			adul->prev = this;
			TRACELVL(5, (mcl_stdout,
				"<- mcl_adu::insert_in_list: ok, inserted at start of list\n"))
			return MCL_OK;
		}
	}
	ASSERT(this->seq != adul->seq);	/* no duplicated adu for the present */
	this->next = adul->next;
	adul->next->prev = this;
	adul->next = this;
	this->prev = adul;
	TRACELVL(5, (mcl_stdout, "<- mcl_adu::insert_in_list:\n"))
	return MCL_OK;
}


/**
 * Remove an ADU from the adu list
 */
mcl_error_status
mcl_adu::remove_from_list(mcl_cb	*const mclcb,
			  mcl_adu	**head)
{
	mcl_adu		*p, *n;		// temp prev and next pointers

	ASSERT(head);
	TRACELVL(5, (mcl_stdout,
			"-> mcl_adu::remove_from_list: adu=x%x, seq=%d\n",
			(int)this, this->seq))
	ASSERT(*head);
	p = this->prev;
	n = this->next;
	if (p == this) {	/* only one ADU in list */
		ASSERT(n == this);
		*head = NULL;	/* list is now empty */
	} else {
		p->next = n;
		n->prev = p;
		if (*head == this) {
			*head = n;	/* adu was the first in list */
		}
	}
	TRACELVL(5, (mcl_stdout, "<- mcl_adu::remove_from_list:\n"))
	return MCL_OK;
}


/**
 * Find an adu in a list with its sequence number.
 * @param mclcb
 * @param head	pointer to the list head pointer
 * @param seq	ADU seq number
 * @return	returns a pointer to the adu if found, NULL otherwise
 */
mcl_adu *
mcl_adu::find_in_list (mcl_cb		*const mclcb,
		       mcl_adu		*head,
		       UINT32		aseq)
{
	mcl_adu		*adu;

	TRACELVL(5, (mcl_stdout, "-> mcl_adu::find_in_list: seq=%d\n", aseq))
	if (!head)  {
		TRACELVL(5, (mcl_stdout, "<- mcl_adu::find_in_list: empty list\n"))
		return NULL;
	}
#if 0
	if (mclcb->findadu_cache && mclcb->findadu_cache->seq == aseq) {
		TRACELVL(5, (mcl_stdout, "<- mcl_adu::find_in_list: found in cache\n"))
		return mclcb->findadu_cache;
	}
#endif
	/*
	 * start from the highest seq number
	 */
	adu = head->prev;
	while (1) {
		ASSERT(adu);
		if (adu->seq == aseq) {
			/* found */
#if 0
			mclcb->findadu_cache = adu;
#endif
			TRACELVL(5, (mcl_stdout, "<- mcl_adu::find_in_list: found\n"))
			return adu;
		} 
		if (adu == head || adu->seq < aseq) {
			/* we have cycled or new adu cannot be in list */
			TRACELVL(5, (mcl_stdout, "<- mcl_adu::find_in_list: new\n"))
			return NULL;
		}
		adu = adu->prev;
	}
}


/**
 * Search a given block in this adu.
 * @param
 * @return	returns a pointer to the block if found, NULL otherwise
 */
mcl_block *
mcl_adu::find_block (mcl_cb	*const mclcb,
		     UINT32	bseq)
{
	mcl_block	*list;		/* in fact this is a tab, not a list! */

 	TRACELVL(5, (mcl_stdout, "-> mcl_adu::find_block: seq=%d\n", bseq))
	if (!(list = this->block_head)) {
		TRACELVL(5, (mcl_stdout,
			"<- mcl_adu::find_block: empty, return NULL\n"))
		return NULL;
	}
	if (bseq > (UINT32)this->block_nb) {
		TRACELVL(5, (mcl_stdout,
			"<- mcl_adu::find_block: ERROR, out of bound\n"))
		return NULL;
	} else {
		TRACELVL(5, (mcl_stdout, "<- mcl_adu::find_block: found\n"))
		return (list + bseq);
	}
}


/**
 * Segment the ADU into blocks (ie. mcl_block) and packets (ie. mcl_du).
 * Used only by the sender.
 */
mcl_error_status
mcl_adu::segment_for_tx (mcl_cb		*const mclcb)
{
	mcl_block	*blk;
	mcl_du		*du;
	INT32		tot_blk_nb;	/* total nb of blocks required */
	INT32		blk_seq;	/* block seq number */
	INT32		tot_du_nb;	/* total nb of DUs required */
	INT32		rem_du_nb;
	INT32		nb;
	INT32		i;
	INT32		rem;
	char		*ptr;
	INT32		max_k;		/* max blk size in DU, depends on fec */

	TRACELVL(5, (mcl_stdout, "-> mcl_adu::segment_for_tx:\n"))
	max_k = mclcb->fec.get_k();
	/*
	 * segment the ADU...
	 * do it simply: allocate a tab of mcl_du and mcl_block objects
	 * rather than two linked lists!
	 */
	tot_du_nb = (int)ceil((double)this->get_len() /
			      (double)mclcb->get_payload_size());
	du = new mcl_du [tot_du_nb];	// no mcl_du default constructor
					// for performance reasons!
	if (du == NULL)
		goto no_memory;
	memset(du, 0, tot_du_nb * sizeof(mcl_du));

	tot_blk_nb = (int)ceil((double)tot_du_nb / (double)max_k);
	blk = new mcl_block [tot_blk_nb];// no mcl_block default constructor
					// for performance reasons!
	if (blk == NULL)
		goto no_memory;
	memset(blk, 0, tot_blk_nb * sizeof(mcl_block));

	this->block_head = blk;
	this->block_nb = tot_blk_nb;

	blk_seq = 0;
	ptr = this->data;
	rem = this->len;
	for (rem_du_nb = tot_du_nb; rem_du_nb > 0; rem_du_nb -= nb) {
		nb = min(rem_du_nb, max_k);
		blk->adu = this;
		blk->seq = blk_seq++;
		blk->set_du_head(du);
		blk->du_nb = nb;
		blk->set_tx_status(BLK_TSTATUS_IN_PROGRESS);
		/* blk->fec_du_head and fec_du_nb already set to NULL/0 */
		for (i = 0; i < nb; i++, du++) {
			du->block = blk;
			du->seq = i;
			du->len = min(rem, mclcb->get_payload_size());
			du->data = ptr;
			ptr += du->len;
			rem -= du->len;
			blk->len += du->len; /* last DU may be non full-sized */
		}
		blk++;	/* switch to next block */
	}
	/*
	 * the full_size_block_len is the size of the first block, since
	 * if there is only one block this latter will not be full...
	 */
	this->full_size_block_len = this->block_head->len;

	ASSERT(rem == 0);
	TRACELVL(1, (mcl_stdout,
		"New ADU: seq=%d, len=%d, composed of %d blocks at most %d bytes/%d DUs long, total %d DUs, max payload_size=%d\n",
		this->seq, this->len, this->block_nb,
		this->full_size_block_len, this->block_head->du_nb,
		tot_du_nb, mclcb->get_payload_size()))
	TRACELVL(5, (mcl_stdout, "<- mcl_adu::segment_for_tx:\n"))
	return MCL_OK;

no_memory:
	PRINT_ERR((mcl_stderr, "mcl_adu::segment_for_tx: ERROR, no memory"))
	mcl_exit(-1);
	return MCL_ERROR;	// unreachable
}


