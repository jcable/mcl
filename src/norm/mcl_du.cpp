/* $Id: mcl_du.cpp,v 1.3 2004/01/30 16:27:42 roca Exp $ */
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
 * default destructor
 */
mcl_du::~mcl_du ()
{
	if (this->pkt) {
		/*
		 * RECEIVER
		 */
		// this is a DU recvd from net, buffer is
		// in the du->pkt and free'd there
		delete this->pkt;
		this->pkt = NULL;
		this->data = 0;
	} else {
		/*
		 * SENDER
		 */
		if (this->is_fec) {
			free(this->data);
			this->data = NULL;
		}
		// else do nothing since all DUs point to the same buffer
		// allocated as a whole during ADU submission.
	}
}


#if 0
/**
 * overloaded destructor.
 * Called when the du->block->adu chain is not fully initialized.
 * @param type	differentiate between TX and RX ADU
 */
mcl_du::~mcl_du (mcl_tx_or_rx	type)
{
	if (type == MCL_TX) {
		/*
		 * SENDER
		 */
		if (this->is_fec) {
			free(this->data);
			this->data = NULL;
		}
		// else do nothing since DU objects and data have been
		// allocated as a whole during submission.
	} else {
		/*
		 * RECEIVER
		 */
		if (this->pkt) {
			// this is a DU recvd from net, buffer is
			// in the du->pkt and free'd there
			delete this->pkt;
			this->pkt = NULL;
			this->data = 0;
		} else if (this->data) {
			// this is a reconstructed data DU
			free(this->data);
		} else {
			ASSERT(0);	// error!!!
		}
	}
}
#endif // 0


/**
 * Insert a du in an ordered list according to the sequence number.
 * Called with the appropriate list head parameter.
 * Insert the du at its location (i.e. to comply
 * with the increasing seq # property) in the list.
 * @param mclcb
 * @param head	pointer to the list head pointer
 * @return	completion status: MCL_OK if inserted, or MCL_ERROR
 * 		if already present in list.
 */
mcl_error_status
mcl_du::insert_in_list (mcl_cb		*const mclcb,
			mcl_du		**head)
{
	mcl_du		*pdu, *ndu;	/* insert du between prev_du, next_du */

	ASSERT(head);
	ASSERT(this->next == NULL && this->prev == NULL); /* single DU to add */
	TRACELVL(5, (mcl_stdout, "-> mcl_du::insert_in_list: du=x%x\n", (int)this))
	/*
	 * find last DU in list, i.e. with highest seq nb
	 */
	if (*head == NULL || (*head)->prev == NULL) {
		/* first DU in empty list */
		*head = this;
		this->next = this->prev = this;
		TRACELVL(5, (mcl_stdout, "<- mcl_du::insert_in_list: first\n"))
		return MCL_OK;
	}
	pdu = (*head)->prev;
	ndu = pdu->next;
	/*
	 * find the proper location of du in the list to make
	 * sure seq numbers are always increasing
	 */
	while (this->seq < pdu->seq) {
		ndu = pdu;
		pdu = pdu->prev;
		if (ndu == *head) {
			/* we have cycled ! du is the new list head */
			*head = this;
			break;
		}
	}
	if (this->seq == pdu->seq) {
		/* DU already received */
		TRACELVL(5, (mcl_stdout, "<- mcl_du::insert_in_list: already rcvd\n"))
		return MCL_ERROR;
	} else {
		/* new DU */
		pdu->next = this;
		this->prev = pdu;
		this->next = ndu;
		ndu->prev = this;
		TRACELVL(5, (mcl_stdout, "<- mcl_du::insert_in_list: new\n"))
		return MCL_OK;
	}
}

