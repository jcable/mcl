/* $Id: mcl_tx.cpp,v 1.17 2005/05/18 14:37:56 roca Exp $ */
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


mcl_tx::mcl_tx ()
{
	this->next_toi = mcl_iss;
	this->reuse_appli_tx_buffer = false;
	this->adu_head = NULL;
	this->keep_data_mode_set = false;
	this->adu_start = NULL;
	this->adu_end = NULL;
	this->first_tx_for_mclcb = true;
	this->next_adu_is_of_high_prio = false;
	this->there_is_more_to_tx = false;
}


mcl_tx::~mcl_tx ()
{
	ASSERT(this->adu_head == NULL);
}


/**
 * Set the "keep data" mode.
 * => See header file for more informations.
 */
mcl_error_status
mcl_tx::keep_data (mcl_cb	*const mclcb)
{
	if (this->keep_data_mode_set == true) {
		PRINT_ERR((mcl_stderr,
		"mcl_tx::keep_data: ERROR, keep data mode already set\n"))
		return MCL_ERROR;
	}
	ASSERT(this->adu_start == NULL && this->adu_end == NULL);
	this->keep_data_mode_set = true;
	TRACELVL(5, (mcl_stdout, "   mcl_tx::keep_data: set\n"))
	return MCL_OK;
}


/**
 * End the "keep data" mode, or said differently, push all accumulated
 * data.
 * => See header file for more informations.
 */
mcl_error_status
mcl_tx::push_data (mcl_cb	*const mclcb)
{
	TRACELVL(5, (mcl_stdout, "-> mcl_tx::push_data: start %d, end %d\n",
		this->adu_start->seq, this->adu_end->seq))
	if (this->keep_data_mode_set == false) {
		PRINT_ERR((mcl_stderr,
		"mcl_tx::push_data: ERROR, keep data mode not set\n"))
		return MCL_ERROR;
	}
	if (this->adu_start == NULL || this->adu_end == NULL) {
		PRINT_ERR((mcl_stderr,
		"mcl_tx::push_data: ERROR, no ADU submitted\n"))
		return MCL_ERROR;
	}
	/* and now do the tx plannification... */
	UpdateTxPlanning(mclcb, this->adu_start, this->adu_end);
	this->keep_data_mode_set = 0;
	this->adu_start = this->adu_end = NULL;
#if 0
	mclcb->lastADUseq = LastADUSeq(mclcb, mclcb->txlay_tab[0].adu_head);
	for (INT32 i = 0; i < MAX_NB_TX_LAYERS; i++) {
		mclcb->txlay_tab[i].adu_head = NULL;
	}
#endif
	TRACELVL(5, (mcl_stdout, "<- mcl_tx::push_data:\n"))
	return MCL_OK;
}


/**
 * Register a new ADU when in "keep data" mode.
 * => See header file for more informations.
 */
void
mcl_tx::register_adu_in_keep_data_mode (mcl_cb	*const mclcb,
					adu_t	*adu)
{
	TRACELVL(5, (mcl_stdout, "-> mcl_tx::push_data: adu %d\n", adu->seq))
	ASSERT(this->keep_data_mode_set);
	if (this->adu_start == NULL)
		this->adu_start = adu;
	if (this->adu_end == NULL || adu->seq > this->adu_end->seq)
		this->adu_end = adu;
	if (adu->seq < this->adu_start->seq) {
		/* required for instance with the FDT of FLUTE if
		 * this latter is submitted after other ADUs */
		this->adu_start = adu;
	}
}


/**
 * Free all the adus of the tx adu list.
 * => See header file for more informations.
 */
void
mcl_tx::free_all_adu (mcl_cb	*const mclcb)
{
	adu_t		*adu;

	TRACELVL(5, (mcl_stdout, "-> mcl_tx::free_all_adu:\n"))
	while ((adu = adu_head) != NULL) {
		this->remove_adu(mclcb, adu);
		/* now free the adu and all of its blocks/dus */
		mcl_tx_free_this_adu(mclcb, adu);
		free(adu);
	}
	TRACELVL(5, (mcl_stdout, "<- mcl_tx::free_all_adu:\n"))
}


/**
 * Copy data from the buffers/files specified by the msghdr to the ADU.
 * => See header file for more informations.
 */
mcl_error_status
mcl_tx::copy_from_iovec_to_adu
				(mcl_cb			*const mclcb,
				struct mcl_msghdr	*msg,
				adu_t			*adu)
{
	UINT32		len;		/* size without padding */
	UINT32		padded_len;	/* size with padding to 0 for FEC */
	struct mcl_iovec  *iov;		/* iovec */
	bool		is_buffer;	/* does input come in a buffer or not */
	void		*data;		/* input data buffer (if applicable) */
	INT32		fd;		/* input data file (if applicable) */

	TRACELVL(5, (mcl_stdout,
		"-> mcl_tx::copy_from_iovec_to_adu:\n"))

	if (msg->msg_iovlen != 1) {
		/*
		 * Complex case where there are multiple iovec entries.
		 */
		PRINT_ERR((mcl_stderr,
			"mcl_tx::copy_from_iovec_to_adu: ERROR, having more than 1 iovec entry is not supported!\n"))
		goto error;
		//return this->copy_from_multiple_iovec_entries(mclcb, msg,adu);
	}
	/*
	 * easy case where there is one iovec entry.
	 */
	/*
	 * find the input data file/buffer.
	 */
	iov = msg->msg_iov;
	if (iov->iov_type == MCL_IOV_TYPE_BUFFER) {
		is_buffer = true;
		data = iov->iov_base;
	} else {
		is_buffer = false;
		if (iov->iov_filename == NULL) {
			PRINT_ERR((mcl_stderr,
			"mcl_tx::copy_from_iovec_to_adu: ERROR, NULL iov_filename\n"))
			goto error;
		}
		fd = mcl_file_open(iov->iov_filename, O_RDONLY, 0);
		if (fd < 0) {
			PRINT_ERR((mcl_stderr,
				"mcl_tx::copy_from_iovec_to_adu: ERROR, mcl_file_open() failed for iov_filename \"%s\"\n",
				iov->iov_filename))
			return MCL_ERROR;
		}
		if (mcl_file_lseek(fd, iov->iov_offset) < 0) {
			PRINT_ERR((mcl_stderr,
			"mcl_tx::copy_from_iovec_to_adu: ERROR, mcl_file_lseek() failed\n"))
			return MCL_ERROR;
		}
	}
	/*
	 * calculate the ADU length.
	 */
	len = iov->iov_len;
	adu->len = len;
	padded_len = (UINT32)(ceil((double)len / (double)adu->symbol_len)
				* (double)adu->symbol_len);
	adu->padded_len = padded_len;
#ifdef VIRTUAL_TX_MEM
	if (mcl_vtm_can_store_in_vtm(mclcb, padded_len)) {
		/*
		 * use the VTMEM service to register data
		 */
		if (mcl_vtm_store_data(mclcb, adu, NULL, (char*)data, fd,
					len, padded_len) ) {
			PRINT_ERR((mcl_stderr,
			"mcl_tx::copy_from_iovec_to_adu: Virtual Tx Memory service failed\n"))
			goto error;
		}
		if (is_buffer && mclcb->tx.can_reuse_appli_buf()) {
			/*
			 * free the data buffer.
			 * In fact using reuse_appli is here useless!
			 */
			free((void*)data);
			PRINT_ERR((mcl_stderr, "mcl_tx::copy_from_iovec_to_adu: using reuse_appli_tx_buffer is useless in Virtual Tx Memory mode\n"))
			goto error; /* NB: error can be ignored safely though */
		}
		TRACELVL(5, (mcl_stdout,
			"<- mcl_tx::copy_from_iovec_to_adu:\n"))
		return MCL_OK;
	}
#endif /* VIRTUAL_TX_MEM */
	/*
	 * store in physical tx memory (ptm)
	 */
	if (is_buffer) {
		/*
		 * input data is already in a buffer.
		 */
		if (mclcb->tx.can_reuse_appli_buf()) {
			/* take control of buf and avoid an extra data copy */
			if (padded_len != len) {
				/* add a null padding to end of block */
				if (!(adu->data = (char*)realloc((void*)data, padded_len))) {
					PRINT_ERR((mcl_stderr,
					"mcl_tx::copy_from_iovec_to_adu: out of memory\n"))
					goto error;
				}
				memset(adu->data + len, 0, padded_len - len);
			}
		} else {
			if (!(adu->data = (char*)malloc(padded_len))) {
				PRINT_ERR((mcl_stderr,
				"mcl_tx::copy_from_iovec_to_adu: out of memory\n"))
				goto error;
			}
			memcpy(adu->data, data, len);
			memset(adu->data + len, 0, padded_len - len);/*padding*/
			/*PRINT_OUT((mcl_stdout,
				"copied data to x%x, len=%d, padded_len=%d\n",
				(int)adu->data, len, padded_len))*/
		}
	} else {
		/* no need to care about can_reuse_appli_buf() here */
		if (!(adu->data = (char*)malloc(padded_len))) {
			PRINT_ERR((mcl_stderr,
			"mcl_tx::copy_from_iovec_to_adu: out of memory\n"))
			goto error;
		}
		if (mcl_file_read(fd, adu->data, len) < 0) {
			PRINT_ERR((mcl_stderr,
			"mcl_tx::copy_from_iovec_to_adu: ERROR, mcl_file_read() failed\n"))
			goto error;
		}
		memset(adu->data + len, 0, padded_len - len);/*padding*/
		/*PRINT_OUT((mcl_stdout,
			"copied data to x%x, len=%d, padded_len=%d\n",
			(int)adu->data, len, padded_len))*/
	}
#ifdef VIRTUAL_TX_MEM
	if (mclcb->vtm_used) {
		/* remember it is kept in physical memory */
		mcl_vtm_register_in_ptm(mclcb, adu, NULL, padded_len);
	}
#endif /* VIRTUAL_TX_MEM */
	TRACELVL(5, (mcl_stdout,
		"<- mcl_tx::copy_from_iovec_to_adu:\n"))
	return MCL_OK;

error:
	TRACELVL(5, (mcl_stdout,
		"<- mcl_tx::copy_from_iovec_to_adu: ERROR\n"))
	return MCL_ERROR;
}


/*
 * for packet traces
 * store msg in a buf and print everything atomically
 */
#define TRC_BUF_SIZE	1024
static char	trc_buf[TRC_BUF_SIZE];
static UINT32	trc_offset = 0;
#define ADD_TO_STR_BUF(m)	{ if (trc_offset + strlen(m) < TRC_BUF_SIZE) { \
					strcpy(&trc_buf[trc_offset], m);     \
					trc_offset += strlen(m);	     \
				} }


/**
 * Try to send as much data as possible in a tx tick for this session.
 * => See header file for more informations.
 */
void
mcl_tx::try_to_send (mcl_cb	*mclcb)
{
        INT32		i;
	INT32		du_nb = 0;	/* number of DUs to send */
	INT32		du_nb_high = 0;	/* number of DUs to send */
	txlay_t		*tl;
	//bool		something_sent;	/* has any DU been sent on base layer?*/
	double		true_du_per_tick; /* takes into account fractions */
					/* of DUs  not sent in previous tick */
	INT32		du_for_this_tick; /* actual rounded # DUs to send */

	TRACELVL(5, (mcl_stdout, "-> mcl_tx::try_to_send: mclcb=x%x\n", (int)mclcb))
	/*
	 * see what to send next for each tx layer.
	 */
	if (mclcb->is_a_sender() && !mclcb->fsm.is_closed(mclcb)) {
		//something_sent= false;	/* reset first */
		this->there_is_more_to_tx = false;	/* reset first */
		for (i = 0, tl = mclcb->txlay_tab; i < mclcb->nb_layers;
		     i++, tl++) {
			if (tl->tx_high_timer > 0) {
				tl->tx_high_timer--;
			} else {
				/* number of ticks per second (in order to
				 * send important data every second) */
				tl->tx_high_timer = (INT32)
					IMPORTANT_DATA_FREQUENCY / MCL_TIMER_PERIOD;
			}
			/*
			 * how many DUs (of high or normal priority) should
			 * be sent?
			 */
			true_du_per_tick = tl->du_per_tick + tl->remaining_du_per_tick;
			du_for_this_tick = (INT32)true_du_per_tick;
			tl->remaining_du_per_tick = true_du_per_tick - du_for_this_tick;
			if (tl->tx_high_timer == 0 || tl->tx_high_flush == true) {
				du_nb_high = min(du_for_this_tick,
						 tl->tx_tab_high->tot_rem);
			} else {
			 	du_nb_high = 0;
			}
			du_nb = min(du_for_this_tick - du_nb_high,
				    tl->tx_tab->tot_rem);
			if (du_nb_high + du_nb > 0) {
				/*
				 * something to send!
				 */
				if (this->first_tx_for_mclcb) {
					this->first_tx_for_mclcb = false;
#ifdef RLC
					if (mclcb->congestion_control == RLC_CC) {
						/* reset SP at first pkt tx */
						rlc_reset_tx_sp(mclcb);
					}
#endif
				}
				this->send_pkt(mclcb, tl, du_nb, du_nb_high);
#if 0
				if (i == 0) {
					/* something sent on base layer */
					something_sent = true;
				}
#endif
			}
			if (tl->tx_tab->tot_rem == 0) {
				/*
				 * get ready for a new tx cycle
				 * on that layer for tx_tab, if needed.
				 */
				tl->tx_tab->new_tx_cycle(mclcb);
			}
			
			if (tl->tx_tab_high->tot_rem == 0) {
				/*
				 * get ready for a new tx cycle
				 * on that layer for tx_tab_high, if needed.
				 */
				if (tl->tx_high_flush == true)
				{
					/* we finished flushing */
					tl->tx_high_flush = false;
				}
				tl->tx_tab_high->new_tx_cycle(mclcb);
			}
		}
		/*
		 * print all the packet traces atomically
		 */
		if (trc_offset > 0) {
			ADD_TO_STR_BUF("\n");
			PRINT_OUT((mcl_stdout, trc_buf))
			trc_offset = 0;
		}
		tl = mclcb->txlay_tab;
		if ((tl->tx_tab_high->tot_rem > 0) || (tl->tx_tab->tot_rem > 0)) {
			/* there remain DUs to transmit next time for base
			 * layer... */
			this->there_is_more_to_tx = true;
		} else if (mclcb->fsm.no_new_adu(mclcb)) {
			/*
			 * no transmission on base layer!
			 * stop everything and send a close message...
			 */
			mclcb->fsm.update_tx_state(mclcb, TEVENT_ALL_DU_SENT);
			SendCLOSE(mclcb);
			mclcb->fsm.update_tx_state(mclcb, TEVENT_CLOSE_SENT);
		}
	}
	TRACELVL(5, (mcl_stdout, "<- mcl_tx::try_to_send:\n"))
}


/**
 * Send this number of DUs for this session on this layer.
 * => See header file for more informations.
 */
void
mcl_tx::send_pkt (mcl_cb	*mclcb,
		  txlay_t	*tl,
		  INT32		du_nb,
		  INT32		du_nb_high)
{
	du_t		*du = NULL;
	int		rem;
	char		s[64];	/* for string */
	
	TRACELVL(5, (mcl_stdout,
		"-> mcl_tx::sendit: lay %d: %d DUs, %d high prio DUs\n",
		tl->layer, du_nb, du_nb_high))
	ASSERT(du_nb > 0 || du_nb_high > 0);
	ASSERT(tl && (tl->tx_tab->tot_rem >= du_nb));
	ASSERT((tl->tx_tab_high->tot_rem >= du_nb_high));

	if (mclcb->get_verbosity() == 1)
		ADD_TO_STR_BUF("\t\t")

	/* high importance data */
	for (rem = du_nb_high; rem > 0; rem--) {
		du = tl->tx_tab_high->get_next_du (mclcb);
		ASSERT(du);
		if (mclcb->get_verbosity() == 1) {
			/* compact, imprecise but easily readable trace */
#ifdef FEC
			if (du->is_fec)
				sprintf(s, "(%d/%d/%d) ", du->block->adu->seq,
					du->block->seq, du->seq);
			else
#endif /* FEC */
				sprintf(s, "[%d/%d/%d] ", du->block->adu->seq,
					du->block->seq, du->seq);
			ADD_TO_STR_BUF(s)
		}
		/* then send data*/
		mcl_send_pkt(mclcb, tl->layer, du, du->block->adu);
	}

	/* normal importance data */	
	for (rem = du_nb; rem > 0; rem--) {
		du = tl->tx_tab->get_next_du (mclcb);
		ASSERT(du);
		if (mclcb->get_verbosity() == 1) {
			/* compact, imprecise but easily readable trace */
#ifdef FEC
			if (du->is_fec)
				sprintf(s, "(%d/%d/%d) ", du->block->adu->seq,
					du->block->seq, du->seq);
			else
#endif /* FEC */
				sprintf(s, "[%d/%d/%d] ", du->block->adu->seq,
					du->block->seq, du->seq);
			ADD_TO_STR_BUF(s)
		}
		/* then send data*/
		mcl_send_pkt(mclcb, tl->layer, du, du->block->adu);
	}
	TRACELVL(5, (mcl_stdout, "<- mcl_tx::sendit:\n"))
}


/*
 * Find an ADU in the tx list with its sequence number and instance id.
 * If parameter FDT_instanceid is -1, don't care for instance ID when 
 * searching ADU, and take the first ADU with the specified sequence
 * number seq.
 */
adu_t *
mcl_tx::find_adu (mcl_cb	*const mclcb,
				   UINT32	idf_adu,
				   INT32	FDT_instanceid)
{
	adu_t		*adu;
	adu_t		*list = this->adu_head;

	TRACELVL(5, (mcl_stdout, "-> mcl_tx::find_adu: seq=%d\n", idf_adu))
	if (!list)  {
		TRACELVL(5, (mcl_stdout, "<- mcl_tx::find_adu: empty list\n"))
		return NULL;
	}
	/* start from the highest seq number */
	adu = list->prev;
	while (1) {
		ASSERT(adu);
		if (adu->seq == idf_adu && ((INT32) adu->FDT_instanceid == FDT_instanceid || FDT_instanceid == -1)) {
			/* found */
			TRACELVL(5, (mcl_stdout, "<- mcl_tx::find_adu: found\n"))
			return adu;
		} 
		if (adu == list || adu->seq < idf_adu) {
			/* we have cycled or new adu cannot be in list */
			TRACELVL(5, (mcl_stdout, "<- mcl_tx::find_adu: new\n"))
			return NULL;
		}
		adu = adu->prev;
	}
}


#if 0
/*
 * scan all the remaining du transmissions and free
 * buffers for ADUs completely sent
 */
void
mcl_tx_cleanup (mcl_cb	*mclcb)
{
	adu_t		*list;		/* head of adu list */
	adu_t		*adu;
	txlay_t		*tl;
	int		i;

	TRACELVL(5, (mcl_stdout, "-> mcl_tx_cleanup:\n"))
	ASSERT(mclcb->delivery_mode == DEL_MODE_PUSH);
	if ((list = mclcb->txlay_tab[0].adu_head) != NULL) {
		/*
		 * reset the in_txtab flag for each ADU
		 */
		adu = list;
		do {
#ifdef DEBUG
			if (adu->blocking_struct.block_nb == 0) {
				/* already free'ed, just to check */
				ASSERT((adu->block_head == NULL &&
					adu->data == NULL))
			}
#endif /* DEBUG */
			adu->in_txtab = 0;	/* by default not in txtab */
			adu = adu->next;
		} while (adu != list);
		/*
		 * search in txtab all the layers which ADUs have pending DUs
		 */
		for (i = 0, tl = mclcb->txlay_tab; i < mclcb->nb_layers;
		     i++, tl++) {
			 tl->tx_tab->mark_adus_in_txtab(mclcb);
			 tl->tx_tab_high->mark_adus_in_txtab(mclcb);
		}
		/*
		 * and free ADUs with no pending DU
		 */
		adu = list;
		do {
			if (adu->blocking_struct.block_nb > 0 && adu->in_txtab == 0) {
				/*
				 * everything has been sent; free this ADU
				 * nb: in fact adu buffer is not free'ed!
				 */
				mcl_tx_free_this_adu(mclcb, adu);
				ASSERT((adu->blocking_struct.block_nb == 0 &&
					adu->block_head == NULL &&
					adu->data == NULL))
			}
			adu = adu->next;
		} while (adu != list);
	}
	TRACELVL(5, (mcl_stdout, "<- mcl_tx_cleanup: ok\n"))
}
#endif



