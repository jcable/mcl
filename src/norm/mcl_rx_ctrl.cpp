/* $Id: mcl_rx_ctrl.cpp,v 1.2 2004/01/30 16:27:43 roca Exp $ */
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
 */
mcl_rx_ctrl::mcl_rx_ctrl()
{
	this->NACK_tevent_head = NULL;
}


/**
 * Process an incoming NORM_CMD packet.
 * @param mclcb
 * @param pkt		pointer to packet
 * @param saddr		source address for this packet
 * @param chdr_infos	pointer to the common_infos struct
 */
mcl_error_status
mcl_rx_ctrl::process_cmd_pkt (mcl_cb			*const mclcb,
			      class mcl_rx_pkt		*pkt,
			      mcl_addr			*saddr,
			      mcl_common_hdr_infos_t	*chdr_infos)
{
	mcl_data_hdr_infos_t	dhdr_infos; // infos of data hdr, host format
	//INT32			hlen;	// total header length
	//INT32			len;	// payload length (without headers)

	TRACELVL(5, (mcl_stdout, "-> mcl_rx_ctrl::process_cmd_pkt:\n"))
	/*
	 * process the NORM DATA header
	 */
	memset(&dhdr_infos, 0, sizeof(mcl_data_hdr_infos_t));
	if (mclcb->norm_pkt_mgmt.parse_cmd_hdr(mclcb, pkt, &dhdr_infos)
	    == MCL_ERROR) {
		PRINT_ERR((mcl_stderr,
		"mcl_rx_ctrl::process_cmd_pkt: ERROR, bad NORM header\n"))
		goto bad_hdr;
	}

	switch (dhdr_infos.cmd_flavor) {

	case NORM_CMD_FLUSH: {
		mcl_rx_ctrl_NACK_tevent	*tevent;
		const INT32		START_NACK_PROCESS_MAX_TIMEOUT = 10;
						// timer duration in msec
		tevent = new mcl_rx_ctrl_NACK_tevent(dhdr_infos.idf_adu,
						     dhdr_infos.idf_block,
						     saddr);
		/*
		 * remove FLUSH duplicates that can exist because of the
		 * ROBUST factor!
		 */
		if (tevent->find_in_list(mclcb, &(this->NACK_tevent_head))) {
			/* FLUSH already received, because of the ROBUST
			 * factor */
			TRACELVL(5, (mcl_stdout,
			"   mcl_rx_ctrl::process_cmd_pkt: FLUSH ignored (already rx)\n"))
			delete tevent;
		} else {
			/* new FLUSH
			 * insert it in the tevent list and set a timer for
			 * delayed FLUSH processing */
			PRINT_LVL(1, (mcl_stdout,
				"\tFLUSH recvd for blk %d/%d\n",
				dhdr_infos.idf_adu, dhdr_infos.idf_block))
			tevent->insert_in_list(mclcb, &(this->NACK_tevent_head));
			mclcb->timer.set_timer(START_NACK_PROCESS_MAX_TIMEOUT,
					(mcl_timer_handler *)this,
					(INT32)mclcb, (INT32)tevent);
		}
		break;
		}

	case NORM_CMD_NO_NEW_OBJECT:
		TRACELVL(5, (mcl_stdout,
			"   mcl_rx_ctrl::process_cmd_pkt: NO_NEW_OBJECT\n"))
		if (mclcb->fsm.no_new_adu(mclcb)) {
			/* this is a duplicated announce; ignore */
			break;
		}
		if (mclcb->get_verbosity() >= 2)
			mcl_print_recvd_du(mclcb, false, MCL_SIG_NONEWADU,
					&dhdr_infos);
		mclcb->rx.set_highest_adu_seq_of_session(dhdr_infos.max_idf_adu);
		mclcb->fsm.update_rx_state(mclcb, REVENT_NO_NEW_ADU);
	
		break;

	case NORM_CMD_CLOSE:
		TRACELVL(5, (mcl_stdout,
			"   mcl_rx_ctrl::process_cmd_pkt: CLOSE\n"))
		if (mclcb->fsm.close_already_rx(mclcb)) {
			/* this is a duplicated announce; ignore */
			break;
		}
		if (mclcb->get_verbosity() >= 2)
			mcl_print_recvd_du(mclcb, false, MCL_SIG_CLOSE,
					&dhdr_infos);
		mclcb->fsm.update_rx_state(mclcb, REVENT_CLOSE_RECV);
		ASSERT(mclcb->fsm.no_new_adu(mclcb));
		if (mclcb->get_stats_level() >= 1) {
			mclcb->stats.print_rx_stats(mclcb);
			mclcb->stats.print_final_stats(mclcb);
		}
		break;

	default:
		PRINT_ERR((mcl_stderr,
			"mcl_rx_ctrl::process_cmd_pkt: ERROR, unknown type %d\n",
			dhdr_infos.cmd_flavor))
#ifdef DEBUG
		mcl_exit(-1);
#else
		goto bad_hdr;
#endif
	}

	TRACELVL(5, (mcl_stdout, "<- mcl_rx_ctrl::process_cmd_pkt:\n"))
	return MCL_OK;


bad_hdr:
	mclcb->stats.bad_hdr++;
	mclcb->stats.errors++;
	delete pkt;		// no du object yet, so free pkt explicitely
	TRACELVL(5, (mcl_stdout,
		"<- mcl_rx_ctrl::process_cmd_pkt: ERROR, bad hdr\n"))
	return MCL_ERROR;

#ifdef NOT_YET
bad:
	TRACELVL(5, (mcl_stdout,
		"<- mcl_rx_ctrl::process_cmd_pkt: ERROR\n"))
	return MCL_ERROR;
#endif
}


/**
 * timer callback method.
 * @param arg1		mclcb
 * @param arg2		pointer to an rx_ctrl_timer_event_t struct
 */
void
mcl_rx_ctrl::timer_callback (INT32	arg1,
			     INT32	arg2)
{
	mcl_cb			*mclcb;	// session of this event
	mcl_rx_ctrl_tevent	*tevent;// timer event

	/*
	 * WARNING, there is a risk that the mclcb context has disappeared
	 * when this function is called. Should check the validity of the
	 * mclcb arg quickly...
	 */
	mclcb = (mcl_cb*)arg1;
	ASSERT(mclcb);
	mclcb->lock();
	tevent = (mcl_rx_ctrl_tevent *)arg2;
	ASSERT(tevent);
	TRACELVL(5, (mcl_stdout, "-> mcl_rx_ctrl::timer_callback: %s\n",
		tevent->get_type_string()))
	/*
	 * Timer processing...
	 */
	switch (tevent->type) {
	case RX_CTRL_TEVENT_START_NACK_PROCESS: {
		mcl_rx_ctrl_NACK_tevent		*te = (mcl_rx_ctrl_NACK_tevent*)tevent;
		/* remove the tevent from the list first */
		if ((te->find_in_list(mclcb, &(this->NACK_tevent_head))) != NULL) {
			ASSERT(te == te->find_in_list(mclcb, &(this->NACK_tevent_head)));
			te->remove_from_list(mclcb, &(this->NACK_tevent_head));
		}
		/* and then perform tevent processing */
		te->process(mclcb);
		delete te;
		break;
		}

	case RX_CTRL_TEVENT_START_ACK_PROCESS:
		break;
		delete tevent;

	default:
		ASSERT(0);
	}
	TRACELVL(5, (mcl_stdout, "<- mcl_rx_ctrl::timer_callback:\n"))
	mclcb->unlock();
	return;

#if 0
bad:
	TRACELVL(5, (mcl_stdout, "<- mcl_rx_ctrl::timer_callback: ERROR\n"))
	mclcb->unlock();
	return;
#endif
}


#if 0
/**
 * Default constructor
 */
mcl_rx_ctrl_tevent::mcl_rx_ctrl_tevent (mcl_rx_ctrl_tevent_types	t)
{
	this->type = t;
}
#endif


/**
 * Default destructor
 */
mcl_rx_ctrl_tevent::~mcl_rx_ctrl_tevent ()
{
}


#ifdef DEBUG
/**
 * Return the tevent type as a static string.
 */ 
char *
mcl_rx_ctrl_tevent::get_type_string () const
{
	// WARNING: must be compliant with mcl_rx_ctrl_tevent_types definition!
	static char	*type_strings[] = {
		"RX_CTRL_TEVENT_INVALID",
		"RX_CTRL_TEVENT_START_NACK_PROCESS",
		"RX_CTRL_TEVENT_START_ACK_PROCESS"
	};

	return type_strings[this->type];
}
#endif // DEBUG


/**
 * Insert a tevent in an unordered list.
 * Called with the appropriate list head parameter.
 * @param mclcb
 * @param head  pointer to the list head pointer
 */
mcl_error_status
mcl_rx_ctrl_tevent::insert_in_list (mcl_cb		*const mclcb,
				    mcl_rx_ctrl_tevent	**head)
{
	ASSERT(head);
	TRACELVL(5, (mcl_stdout, "-> mcl_rx_ctrl_tevent::insert_in_list:\n"))
	if (!(*head)) {
		/*
		 * first tevent in list
		 */
		*head = this;
		this->next = this->prev = this;
		TRACELVL(5, (mcl_stdout, "<- mcl_rx_ctrl_tevent::insert_in_list: ok, inserted in empty list\n"))
	} else {
		/*
		 * insert in list tail.
		 */
		this->next = *head;
		this->prev = (*head)->prev;
		(*head)->prev->next = this;
		(*head)->prev = this;
		TRACELVL(5, (mcl_stdout, "<- mcl_rx_ctrl_tevent::insert_in_list:\n"))
	}
	return MCL_OK;
}


/**
 * Remove a tevent from an unordered list.
 * Called with the appropriate list head parameter.
 * @param mclcb
 * @param head  pointer to the list head pointer
 */
mcl_error_status 
mcl_rx_ctrl_tevent::remove_from_list (mcl_cb			*const mclcb,
				      mcl_rx_ctrl_tevent	**head)
{
	mcl_rx_ctrl_tevent	*p, *n;		// temp prev and next pointers

	ASSERT(head);
	TRACELVL(5, (mcl_stdout, "-> mcl_rx_ctrl_tevent::remove_from_list:\n"))
	ASSERT(*head);
	p = this->prev;
	n = this->next;
	if (p == this) {	/* only one tevent in list */
		ASSERT(n == this);
		*head = NULL;	/* list is now empty */
	} else {
		p->next = n;
		n->prev = p;
		if (*head == this) {
			*head = n;	/* adu was the first in list */
		}
	}
	TRACELVL(5, (mcl_stdout, "<- mcl_rx_ctrl_tevent::remove_from_list:\n"))
	return MCL_OK;
}


/**
 * Find a NACK_tevent in an unordered list with a type-dependant criteria.
 * @param mclcb
 * @param head  pointer to the list head pointer
 * @return      returns a pointer to the tevent if found, NULL otherwise
 */
mcl_rx_ctrl_NACK_tevent *
mcl_rx_ctrl_NACK_tevent::find_in_list (mcl_cb			*const mclcb,
					mcl_rx_ctrl_tevent	**head)
{
	mcl_rx_ctrl_NACK_tevent		*tevent;
	mcl_rx_ctrl_NACK_tevent		*nhead;

	TRACELVL(5, (mcl_stdout, "-> mcl_rx_ctrl_tevent::find_in_list:\n"))
	ASSERT(head);
	if (!(nhead = (mcl_rx_ctrl_NACK_tevent*)(*head)))  {
		TRACELVL(5, (mcl_stdout,
			"<- mcl_rx_ctrl_tevent::find_in_list: empty list\n"))
		return NULL;
	}
	/*
	 * start from the list tail
	 */
	tevent = (mcl_rx_ctrl_NACK_tevent*)(nhead->prev);
	while (1) {
		ASSERT(tevent);
		if ((tevent->adu_id == this->adu_id) &&
		    (tevent->block_id == this->block_id)) {
			/* found */
			TRACELVL(5, (mcl_stdout,
				"<- mcl_rx_ctrl_tevent::find_in_list: found\n"))
			return tevent;
		} 
		if (tevent == nhead) {
			/* we have cycled or new adu cannot be in list */
			TRACELVL(5, (mcl_stdout,
				"<- mcl_rx_ctrl_tevent::find_in_list: new\n"))
			return NULL;
		}
		tevent = (mcl_rx_ctrl_NACK_tevent*)tevent->prev;
	}
}


/**
 * Default constructor.
 */
mcl_rx_ctrl_NACK_tevent::mcl_rx_ctrl_NACK_tevent (UINT32	adu_idf,
						  UINT32	block_idf,
						  mcl_addr	*saddr)
{
	this->type = RX_CTRL_TEVENT_START_NACK_PROCESS;
	this->prev = this->next = NULL;
	this->adu_id = adu_idf;
	this->block_id = block_idf;
	this->src_addr = *saddr;
}


/**
 * Default destructor.
 */
mcl_rx_ctrl_NACK_tevent::~mcl_rx_ctrl_NACK_tevent ()
{
}


/**
 * Perform NACK_tevent specific processing.
 */
void
mcl_rx_ctrl_NACK_tevent::process (mcl_cb	*const mclcb)
{
	mcl_adu			*adu;
	mcl_block		*blk;
	INT32			missing;	// nb of missing DUs
	norm_common_hdr_t	*hdr;		// norm header
	INT32			hlen;		// norm header length
	mcl_addr		sa;		// remote address to which NACK
						// must be sent

	TRACELVL(5, (mcl_stdout, "-> mcl_rx_ctrl_NACK_tevent::process:\n"))
	/* find the adu first */
	if (!(adu = mclcb->rx_window.find_adu(mclcb, this->adu_id))) {
		/*
		 * for some reason this ADU is not in rx window...
		 * maybe all packets of this ADU have been lost, so ask for
		 * a complete retransmission
		 */
		TRACELVL(5, (mcl_stdout,
			"   mcl_rx_ctrl_NACK_tevent::process: WARNING, adu %d not found in rx window\n",
			this->adu_id))
#if 0
		mclcb->stats.errors++;
		PRINT_ERR((mcl_stderr,
			"mcl_rx_ctrl_NACK_tevent::process: WARNING, adu %d not found in rx window\n",
			this->adu_id))
		TRACELVL(5, (mcl_stdout,
			"<- mcl_rx_ctrl_NACK_tevent::process: WARNING, adu not found in rx window\n"))
		return;
#endif
		sa = this->src_addr;
		missing = 1;	// be conservative first!
	} else {
		/*
		 * normal case where the ADU is known...
		 */
		sa = adu->addr;
		if (adu->check_if_completed(mclcb)) {
			/* ADU already completed/decoded/delivered, no need
			 * to check further */
			TRACELVL(5, (mcl_stdout, "<- mcl_rx_ctrl_NACK_tevent::process: adu already completed\n"))
			return;
		}
		/* find the block */
		if (!(blk = adu->find_block(mclcb, this->block_id))) {
			/*
			 * for some reason this block is not in rx window
			 * whereas the ADU is know.
			 * This is a serious error, exit all!
			 */
			//TRACELVL(5, (mcl_stdout, "<- mcl_rx_ctrl_NACK_tevent::process: block not found in rx window, ask for complete retx\n"))
			PRINT_ERR((mcl_stdout, "mcl_rx_ctrl_NACK_tevent::process: ERROR, block not found in rx window\n"))
			mcl_exit(-1);
		}
		/* and calculate how many DUs are missing */
		missing = blk->get_number_of_missing_dus(mclcb);
		if (missing == 0) {
			/* nothing to do */
			TRACELVL(5, (mcl_stdout,
				"<- mcl_rx_ctrl_NACK_tevent::process: block fully rx\n"))
			return;
		}
	}
	/*
	 * some DUs are still missing
	 * create and send a NACK packet...
	 */
	if ((hlen = mclcb->norm_pkt_mgmt.create_nack_hdr(mclcb, &hdr,
				this->adu_id, this->block_id, missing)) <= 0) {
		PRINT_ERR((mcl_stderr,
			"mcl_rx_ctrl_NACK_tevent::process: norm_pkt_mgmt.create_nak_hdr() failed\n"))
		ASSERT(0);      // in debug mode, stop immediately...
		return;
	}
	/*
	 * send the NACK specifically to the source unicast address
	 */
	mclcb->ses_channel.send_pkt(mclcb, sa, hdr, hlen, NULL);
	PRINT_LVL(1, (mcl_stdout, "\tNACK sent for blk %d/%d, %d missing\n",
				this->adu_id, this->block_id, missing))
	TRACELVL(5, (mcl_stdout, "<- mcl_rx_ctrl_NACK_tevent::process: NACK sent\n"))
	return;

#if 0
ask_for_complete_retx:
	PRINT_ERR((mcl_stderr, "XXX: ERROR, TODO: ask for complete retx\n"))
	TRACELVL(5, (mcl_stdout, "<- mcl_rx_ctrl_NACK_tevent::process:\n"))
	ASSERT(0);      // in debug mode, stop immediately...
	return;
#endif
}

