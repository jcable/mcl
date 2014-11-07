/* $Id: mcl_cb.cpp,v 1.3 2004/01/30 16:27:42 roca Exp $ */
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

/*
 * mcl_cb class implementation
 */

mcl_cb	*mclcb_tab[MAX_NB_MCLCB];


/**
 * One time initialization of the mclcb_tab.
 */
void
mcl_init_mclcb_tab (void)
{
#ifdef DEBUG
	static bool	init = false;

	ASSERT(init == false);		// check that it is called only once
	init = true;
#endif
	/* the session control block table */
	memset(mclcb_tab, 0, sizeof(mclcb_tab));
}


/**
 * Session control block constructor.
 */
mcl_cb::mcl_cb (mcl_tx_rx_mode	mode)
{
	mcl_global_init();		// global MCL initialization (if not
					// already performed
	// initialize everything first...
	this->id = 0;
	this->fully_initialized = false;
	this->tx_rx_mode = MCL_INVALID;
	this->verbosity_level = 0;
	//this->verbosity_level = 6;	// for tests... set to maximum
	this->stats_level = 0;
	this->max_datagram_size = MAX_DATAGRAM_SIZE;
	this->payload_size = MAX_PAYLOAD_SIZE;

	// default tx rate init. Must be done after datagram_size init.
	//this->tx.set_bps_tx_rate(this, (float)10000000.0);
	this->tx.set_bps_tx_rate(this, (float)1000000.0);
	//this->tx.set_bps_tx_rate(this, (float)32000.0);

	// trick to get a unique identifier in the file descriptor space
	if ((this->id = (INT32)dup(0)) < 0) {
		PRINT_ERR((mcl_stderr,
		"mcl_cb::mcl_cb: ERROR cannot create a new session identifier\n"))
		goto bad;
	}
	if (this->id >= MAX_NB_MCLCB) {
		PRINT_ERR((mcl_stderr,
		"mcl_cb::mcl_cb: ERROR too many opens, MCL file descriptor space full\n"))
		goto bad;
	}
	// store the new mclcb in the tab
	ASSERT(mclcb_tab[this->id] == NULL);
	mclcb_tab[this->id] = this;

	// initialize the mode
	this->tx_rx_mode = mode;
	switch (mode) {
	case MCL_IS_A_SENDER_ONLY:
		this->fsm.update_tx_state(this, TEVENT_OPEN_CALLED);
		// can't call init_sender() here because of param parsing
		break;
	case MCL_IS_A_RECEIVER_ONLY:
		this->fsm.update_rx_state(this, REVENT_OPEN_CALLED);
		// can't call init_receiver() here because of param parsing
		break;
	default:
		PRINT_ERR((mcl_stderr, "mcl_cb::mcl_cb: ERROR unknown mode %d\n",
			mode))
		goto bad;
	}

	// global session lock... there's only one!
	mcl_init_lock(&(this->session_lock));

	// ok, return
	return;

bad:
	PRINT_ERR((mcl_stderr, "mcl_cb::mcl_cb: failed\n"))
	exit (-1);
}


/**
 * Destructor.
 */
mcl_cb::~mcl_cb ()
{
	mclcb_tab[this->id] = NULL;
	this->id = 0;
	this->fully_initialized = false;
}


mcl_error_status
mcl_cb::set_verbosity (INT32	level)
{
	if (level < 0 || level > 6) {
		PRINT_ERR((mcl_stderr, "mcl_cb::set_verbosity: level out of range (expected [0;6], got %d).\n", (int)level))
		return MCL_ERROR;
	}
	this->verbosity_level = (INT8)level;
	return MCL_OK;
}


mcl_error_status
mcl_cb::set_stats_level (INT32	level)
{
	if (level < 0 || level > 2) {
		PRINT_ERR((mcl_stderr, "mcl_cb::set_verbosity: level out of range (expected [0;1], got %d).\n", (int)level))
		return MCL_ERROR;
	}
	this->stats_level = (INT8)level;
	return MCL_OK;
}


/**
 * Finish the initialization of the session.
 */
mcl_error_status
mcl_cb::finish_session_init ()
{
#ifdef DEBUG
	mcl_cb	*mclcb = this;		// required by TRACELVL
#endif

	TRACELVL(5, (mcl_stdout, "-> mcl_cb::finish_session_init:\n"))
	ASSERT((!this->fully_initialized))
	this->fully_initialized = true;
	/*
	 * perform socket initialization now...
	 */
	if (this->ses_channel.sock_init(this) != MCL_OK) {
		PRINT_ERR((mcl_stderr,
		"mcl_cb::finish_session_init: ERROR: socket initialization failed\n"))
		mcl_exit(1);
	}
	/*
	 * start the reception thread.
	 * required both by senders and receivers.
	 */
	this->rx_thread.start(this);

	TRACELVL(5, (mcl_stdout, "<- mcl_cb::finish_session_init:\n"))

	return MCL_OK;
}


void
mcl_cb::free_everything ()
{
	/* free ADUs and their buffers */
	this->tx.free_all_adu(this);
	/* close all the opened sockets */
	this->ses_channel.sock_close(this);
}


mcl_error_status
mcl_cb::set_max_datagram_size (INT32 sz)
{
	// sanity checks...
	if (sz < MIN_NORM_HDR_SIZE || sz > MAX_DATAGRAM_SIZE) {
		PRINT_ERR((mcl_stderr,
			"mcl_cb::set_max_datagram_size: size out of range (expected [%d;%d], got %d).\n",
			MIN_NORM_HDR_SIZE, MAX_DATAGRAM_SIZE, sz))
		return MCL_ERROR;
	}
	this->max_datagram_size = sz;
	this->payload_size = this->max_datagram_size - MAX_NORM_HDR_SIZE_FOR_DATA;
	return MCL_OK;
}


mcl_error_status
mcl_cb::set_payload_size (INT32 sz)
{
	// sanity checks...
	if (sz < 1 || sz > MAX_PAYLOAD_SIZE) {
		PRINT_ERR((mcl_stderr,
			"mcl_cb::set_payload_size: size out of range (expected [1;%d], got %d).\n",
			MAX_PAYLOAD_SIZE, sz))
		return MCL_ERROR;
	}
	if (sz >= this->max_datagram_size) {
		PRINT_ERR((mcl_stderr,
			"mcl_cb::set_payload_size: size %d cannot be larger than current max datagram size %d.\n",
			sz, this->max_datagram_size))
		return MCL_ERROR;
	}
	this->payload_size = sz;
	return MCL_OK;
}


