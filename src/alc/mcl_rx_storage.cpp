/* $Id: mcl_rx_storage.cpp,v 1.5 2005/05/17 12:36:58 roca Exp $ */
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
 * Default constructor.
 * Allocates the data buffer with the size provided.
 */
mcl_rx_pkt::mcl_rx_pkt (INT32	max_size)
{
	this->buf = new char[max_size];
	ASSERT(this->buf);
#ifdef LDPC_FEC	/* LDPC DecodeFecStep requires that buffer be
		 * cleared (problem of last packet of a block)
		 * not required by RSE; padding of last pkt of
		 * a block cleared while copying into temp buf
		 */
	memset(this->buf, 0, max_size);
#endif
	this->buf_len = max_size;
	this->pkt_len = 0;
}


/**
 * Default destructor.
 * This destructor deletes both the pkt and the buffer, but NOT the DU
 * (if any).
 */
mcl_rx_pkt::~mcl_rx_pkt ()
{
	if (this->buf) {
		delete [] this->buf;
		this->buf = NULL;
	}
	this->buf_len = 0;
	this->pkt_len = 0;
}


/******************************************************************************/


/**
 * Constructor.
 */
mcl_rx_storage::mcl_rx_storage()
{
	this->vrmem_desired = true;
	this->vrmem_used = false;
#ifdef VIRTUAL_RX_MEM
	this->vrmem = NULL;
#endif
}


/**
 * Destructor.
 */
mcl_rx_storage::~mcl_rx_storage()
{
#ifdef VIRTUAL_RX_MEM
	this->close_vrmem(NULL);
#endif
	this->vrmem_desired = true;
}


#ifdef VIRTUAL_RX_MEM
/**
 * Closes the Virtual receive memory service, when applicable.
 */
void
mcl_rx_storage::close_vrmem	(class mcl_cb	*const mclcb)
{
#ifdef DEBUG
	if (mclcb) {
		TRACELVL(5, (mcl_stdout, "   mcl_rx_storage::close_vrmem:\n"))
	}
#endif
	if (this->vrmem_used) {
		delete this->vrmem;
		this->vrmem = NULL;
		this->vrmem_used = false;
	}
}
#endif
 

/**
 * Stores data of the DU either in RAM or in the vrmem file,
 * whichever is the most appropriate.
 */
mcl_error_status
mcl_rx_storage::store_data (class mcl_cb	*const mclcb,
			    du_t		*du)
{
	TRACELVL(5, (mcl_stdout,
		"-> mcl_rx_storage::store_data: du=x%x, seq=%d, len=%d\n",
		(INT32)du, du->seq, du->len))
#ifdef VIRTUAL_RX_MEM
	if (this->vrmem_desired) {
		if (this->vrmem_used == false) {
			this->initialize_vrmem();
		}
		if (du->vrm_info.status == STORAGE_STATUS_INVALID) {
			/* new fresh DU that must be stored for the 1st time */
			if (this->vrmem->must_store_in_vrfile(mclcb, du->len)) {
				if (this->vrmem->store_in_vrfile(mclcb, du) == MCL_ERROR) {
					return MCL_ERROR;
				}
			} else {
				this->vrmem->register_in_vrbuf(mclcb, du);
				/* update stats */
				mclcb->stats.buf_space += du->len;
				if (mclcb->stats.buf_space > mclcb->stats.max_buf_space) {
					mclcb->stats.max_buf_space =
								mclcb->stats.buf_space;
				}
			}
		} else {
			/* DU already stored once by the system */
			if (this->vrmem->in_vrfile(mclcb, du) &&
			    this->vrmem->in_vrbuf(mclcb, du)) {
				/*
				 * vrbuf copy is newer than vrfile copy,
				 * so update the permanent repository
				 * (similar to data with "dirty bit" set)
				 */
				if (this->vrmem->update_vrfile_from_dirty_vrbuf
						(mclcb, du) == MCL_ERROR) {
					return MCL_ERROR;
				}
			}
		}
	}
#else  /*  !VIRTUAL_RX_MEM */
	/* nothing to do, data is kept in memory and nothing is registered */
	/* update stats */
	mclcb->stats.buf_space += du->len;
	if (mclcb->stats.buf_space > mclcb->stats.max_buf_space) {
		mclcb->stats.max_buf_space = mclcb->stats.buf_space;
	}
#endif /*  VIRTUAL_RX_MEM */
	TRACELVL(5, (mcl_stdout, "<- mcl_rx_storage::store_data: du status=%d\n",
		du->vrm_info.status))
	return MCL_OK;
}


/**
 * Copies data in the buffer given in parameter, either
 * from a source data buffer, or from the vrmem file, whichever
 * is appropriate.
 */
mcl_error_status
mcl_rx_storage::get_data (class mcl_cb		*const mclcb,
			  du_t			*du)
{
	TRACELVL(5, (mcl_stdout,
		"-> mcl_rx_storage::get_data: du=x%x, seq=%d, status=%d, len=%d\n",
		(INT32)du, du->seq, du->vrm_info.status, du->len))
#ifdef VIRTUAL_RX_MEM
	if (this->vrmem_used) {
		ASSERT(du->vrm_info.status != STORAGE_STATUS_INVALID);
		if (this->vrmem->in_vrbuf(mclcb, du) == false) {
			UINT16	symbol_len;	// full size symbol length

			ASSERT(du->pkt == NULL);
			ASSERT(du->data == NULL);
			/* allocate destination buffer.
			 * Always allocate the full symbol size (needed
			 * by LDPC decoder, to avoid errors when adding
			 * the last (shorter) packet of a block).
			 */
			if (du->block) {
				ASSERT(du->block->adu);
				symbol_len = du->block->adu->symbol_len;
			} else {
				symbol_len = du->len;
			}
			mcl_rx_pkt	*pkt = new mcl_rx_pkt (symbol_len);
			ASSERT(pkt);
			pkt->pkt_len = du->len;
			du->pkt = pkt;
			du->data = pkt->get_buf();
			if (symbol_len > du->len) {
				memset(du->data, 0, symbol_len);
			}
			/* not in memory, so retrieve it from vrfile */
			if (this->vrmem->get_from_vrfile(mclcb, du) == MCL_ERROR) {
				PRINT_ERR((mcl_stderr, "mcl_rx_storage::get_data: ERROR, null buffer\n"))
				return MCL_ERROR;
			}
			/* update stats */
			mclcb->stats.buf_space += symbol_len;
		}
	}
#endif /* VIRTUAL_RX_MEM */
	TRACELVL(5, (mcl_stdout, "<- mcl_rx_storage::get_data:\n"))
	return MCL_OK;
}


/**
 * Unregisters data of the DU and frees the associated data buffer.
 */
mcl_error_status
mcl_rx_storage::free_data (class mcl_cb	*const mclcb,
			   du_t		*du)
{
	TRACELVL(5, (mcl_stdout,
		"-> mcl_rx_storage::free_data: du seq=%d, status=%d\n",
		du->seq, du->vrm_info.status))
#ifdef VIRTUAL_RX_MEM
	/* test first if it is in vrbuf... */
	if (this->vrmem->in_vrbuf(mclcb, du) == true) {
		this->vrmem->remove_from_vrbuf(mclcb, du);
		/* update stats */
		mclcb->stats.buf_space -= du->len;
	}
	/* test now if it is also in vrfile (in this order)... */
	if (this->vrmem->in_vrfile(mclcb, du) == true) {
                this->vrmem->remove_from_vrfile(mclcb, du);
        }
#else
	if (du->pkt) {
		delete du->pkt;
		du->pkt = NULL;
	}
	du->data = NULL;
	du->len = 0;
#endif
	return MCL_OK;
}


/**
 * Initialize the Virtual receive memory service.
 */
mcl_error_status
mcl_rx_storage::initialize_vrmem ()
{
	ASSERT(this->vrmem_desired == true);
	ASSERT(this->vrmem_used == false);
	ASSERT(this->vrmem == NULL);
#ifdef VIRTUAL_RX_MEM
	this->vrmem_used = true;
	this->vrmem = new mcl_vrmem;	/* performs 1st level initialization */
#else
	this->vrmem_used = false;
	this->vrmem = NULL;
#endif
	return MCL_OK;
}


