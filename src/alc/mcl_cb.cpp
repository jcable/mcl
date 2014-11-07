/* $Id: mcl_cb.cpp,v 1.34 2005/05/24 15:43:22 roca Exp $ */
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

/* the MCL control block table is defined here... */
mcl_cb	*mclcb_tab[MAX_NB_MCLCB];

/*
 * One time initialization of the mclcb_tab.
 * => See header file for more informations.
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



/*
 * Session control block constructor.
 * => See header file for more informations.
 */
mcl_cb::mcl_cb (mcl_tx_rx_mode	mode)
{
	INT32	i;
	INT32	prof;	/* temp */
#if 0
	/*
	 * Tx rate scale (how many DUs to send on each layer at each time tick)
	 * Use the power of 2 scale of tx rate (default), or a custom scale.
	 * XXX: adjust the nb of preinitialized elements according to
	 * MAX_NB_TX_LAYERS, otherwise it generates a warning!
	 */
	INT32	du_per_tick_scale[MAX_NB_TX_LAYERS]; // predefined tx rate scale
#endif


	mcl_global_init();	// global MCL initialization, if not already
				// performed. Otherwise it does nothing.

	this->fully_initialized = false;	// not yet...
	/*
	 * get a unique identifier in the file descriptor space
	 */
	if ((this->id = (INT32)dup(0)) < 0) {
		PRINT_ERR((mcl_stderr,
		"mcl_cb::mcl_cb: ERROR cannot create a new session identifier\n"))
		goto bad;
	}
	if (this->id >= MAX_NB_MCLCB) {
		PRINT_ERR((mcl_stderr,
		"mcl_cb::mcl_cb: ERROR too many opened session, MCL file descriptor space full\n"))
		PRINT_ERR((mcl_stderr,
		"Recompile the MCL-ALC lib with a higher MAX_NB_MCLCB (src/alc/mcl_profile.h)\n"))
		goto bad;
	}
	/*
	 * store the new mclcb in the tab
	 */
	ASSERT(mclcb_tab[this->id] == NULL);
	mclcb_tab[this->id] = this;

	/*
	 * various mode initialization
	 */
	this->tx_rx_mode = mode;
	this->flute_mode = false;
	this->verbosity_level = 0;
	//this->verbosity_level = 5;		// for tests... set to maximum
	this->stats_level = 0;
	//this->stats_level = 2;		// for tests... set to maximum

	/*
	 * FSM initialization
	 */
	switch (this->tx_rx_mode) {
	case MCL_IS_A_SENDER_ONLY:
		this->fsm.update_tx_state(this, TEVENT_OPEN_CALLED);
		// can't call init_sender() here because of param parsing
		break;
	case MCL_IS_A_RECEIVER_ONLY:
		this->fsm.update_rx_state(this, REVENT_OPEN_CALLED);
		// can't call init_receiver() here because of param parsing
		break;
	default:
		PRINT_ERR((mcl_stderr,
		"mcl_cb::mcl_cb: ERROR unknown mode %d\n", this->tx_rx_mode))
		goto bad;
	}
	/*
	 * global session lock... there's only one!
	 */
	mcl_init_lock(&(this->session_lock));

	/*
	 * addr initialization
	 */
	this->addr.reset();		/* call constructor manually */
	this->addr.set_port(5665);
	this->mcast_if_addr = NULL;	/* nothing by default */
	this->mcast_if_name = NULL;	/* nothing by default */
	//this->rx.src_addr.reset();	/* call constructor manually */
	//this->rx.check_src_addr = false;
	for (i = 0; i < MAX_MC_GROUP; i++) {
		this->socket_tab[i].addr.reset(); /* call constructor man. */
	}
	this->ttl = TTL;
	this->demux_label = 0;	// default TSI for filtering incoming pkts
	/*
	 * choose a default tx profile. This will initialize at least the:
	 * 	single_layer_mode
	 * 	max_nb_layers
	 * 	congestion_control
	 * 	max_datagram_size
	 * 	payload_size (as a side effect of setting max_datagram_size)
	 * variables.
	 * the default tx profile can be changed in mcl_profile.h, and
	 * will anyway be overwritten by the FCAST/FLUTE application...
	 */
	this->congestion_control = INVALID_CC; /* let set_tx_profile choose */
	prof = DFLT_TX_PROFILE;
	if (mcl_set_tx_profile(this, prof) < 0) {
		PRINT_ERR((mcl_stderr,
			"mcl_alloc_mclcb: ERROR: mcl_set_tx_profile failed\n"))
		mcl_exit(-1);
	}
	//this->rx.never_leave_base_layer = 1;
	/*
	 * FEC initialization
	 */
	this->fec.initialize();

	/* by default init nb_layer to 1; updated later in mcl_init_layer_nb */
	this->nb_layers = 1;	/* only 1 tx layer to start... */
	this->mcl_max_group = MAX_MC_GROUP;
	this->nb_tx = 1;	/* by default single tx for each DU */
	
	/*
	 * choose a packet scheduler and an object scheduler
	 */
#if defined(LCT_SCHED1)
	this->scheduler = MCL_SCHED_LCT1;
#else
	Error, you must choose a scheduler! Define one in file mcl_profile.h.
#endif
	/* by default mix everything */
	this->adu_scheduling = MCL_SCHED_MIXED_ORDER;

#ifdef ANTICIPATED_TX_FOR_PUSH
	this->anticipated_tx_for_push = true;
#endif

#ifdef POSTPONE_FEC_DECODING
	/* it is dangerous to set fec_decoding to 1 by default. Prefer 0! */
	this->postpone_fec_decoding = true;
#else
	this->postpone_fec_decoding = false;
#endif

	this->tx_mem_cleanup_count = TX_MEM_CLEANUP_PERIOD;
	this->stats_time_count = STATS_PERIOD;

	/*
	 * clear stats
	 */
	memset((void *)&this->stats, 0, sizeof(this->stats));

	/* some more inits */
	this->findadu_cache = NULL;	/* nothing by default */
	this->lastadu_cache = NULL;
	this->ready_data = 0;
#ifdef METAOBJECT_USED
	this->meta_obj_layer = new mcl_meta_object_layer(this);
#endif
	this->sig_tab = NULL;
	this->psig_next = NULL;	

	// ok, return
	return;

bad:
	PRINT_ERR((mcl_stderr, "mcl_cb::mcl_cb: failed\n"))
	exit (-1);
}


/**
 * Destructor.
 * => See header file for more informations.
 */
mcl_cb::~mcl_cb ()
{
	mclcb_tab[this->id] = NULL;
	this->id = 0;
	this->fully_initialized = false;
}


/**
 * Free everything.
 * => See header file for more informations.
 */
void
mcl_cb::free_everything ()
{
#ifdef DEBUG
	mcl_cb	*mclcb = this;		// required by TRACELVL
#endif

	/* free ADUs and their buffers */
	this->tx.free_all_adu(this);
#if 0
	/* close all the opened sockets */
	this->ses_channel.sock_close(this);
#endif

	TRACELVL(5, (mcl_stdout, "-> mcl_free_mclcb:\n"))
	/*
	 * close all the remaining sockets;
	 * especially required in case of a sender
	 */
	mcl_free_all_layers(this);
	/*
	 * no sig service any more
	 */
	if (this->is_a_sender()) {
		mcl_sig_close(this);
	}
	/*
	 * free all remaining data/adu/tx tab...
	 */
	if (this->is_a_sender()) {
		INT32	lay;
		txlay_t	*tl;
		for (lay = 0, tl = this->txlay_tab; lay < this->max_nb_layers; lay++, tl++) {
			delete tl->tx_tab;
			delete tl->tx_tab_high;
		}
	}
	
#ifdef METAOBJECT_USED	
	/* close meta object mgmnt */
	delete this->meta_obj_layer;
	this->meta_obj_layer = NULL;
#endif
	
	/*
	 * close congestion control services
	 */
#ifdef RLC	 
	if (this->is_a_receiver() && this->congestion_control == RLC_CC) {
		rlc_end_session(this);
	}
#endif
#ifdef FLIDS
	if (this->is_a_receiver() && this->congestion_control == FLID_SL_CC) {
		FLIDs_EndSession(this);
	}
#endif	
	TRACELVL(5, (mcl_stdout, "<- mcl_free_mclcb: ok\n"))
}


/**
 *
 * => See header file for more informations.
 */
mcl_error_status
mcl_cb::set_verbosity (INT32	level)
{
	if (level < 0 || level > 6) {
		PRINT_ERR((mcl_stderr,
		"mcl_cb::set_verbosity: ERROR, level out of range (expected [0;6], got %d).\n", (int)level))
		return MCL_ERROR;
	}
	this->verbosity_level = (INT8)level;
	return MCL_OK;
}


/**
 *
 * => See header file for more informations.
 */
mcl_error_status
mcl_cb::set_stats_level (INT32	level)
{
	if (level < 0 || level > 2) {
		PRINT_ERR((mcl_stderr, "mcl_cb::set_stats_level: ERROR, level out of range (expected [0;2], got %d).\n", (int)level))
		return MCL_ERROR;
	}
	this->stats_level = (INT8)level;
	return MCL_OK;
}


/**
 * Finish the initialization of the session.
 * => See header file for more informations.
 */
mcl_error_status
#ifdef SVSOA_RECV
mcl_cb::finish_session_init (int nb_lay)
#else
mcl_cb::finish_session_init ()
#endif
{
#ifdef DEBUG
	mcl_cb	*mclcb = this;		// required by TRACELVL
#endif
	INT32	mode = 0;	/* init mode of sockets for each layer */

	TRACELVL(5, (mcl_stdout, "-> mcl_cb::finish_session_init:\n"))
	ASSERT((!this->fully_initialized))
	this->fully_initialized = true;

	/*
	 * perform socket initialization now...
	 */
	/* moved here from init_receiver which is called to late! */
	memset((void *)&this->rxlvl, 0, sizeof(this->rxlvl));
	/*
	 * set the init mode first...
	 */
	if (this->is_a_sender()) {
		if (this->addr.is_multicast_addr()) {
			mode = MODE_MCAST_TX;
		} else {
			mode = MODE_UNI_TX;
			// in unicast, use a single layer!
			this->max_nb_layers = 1;
			this->single_layer_mode = true;
		}
	} else {
		ASSERT(this->is_a_receiver());
		if (this->addr.is_multicast_addr()) {
			mode = MODE_MCAST_RX;
		} else {
			mode = MODE_UNI_RX;
			// in unicast, use a single layer!
			this->max_nb_layers = 1;
			this->single_layer_mode = true;
		}
	}
	this->ucast_mcast_mode = mode;	/* remember the ucast_mcast_mode */

#ifdef SVSOA_RECV	
	if (mcl_init_layer_nb(this, nb_lay) < 0)
#else
	if (mcl_init_layer_nb(this) < 0)
#endif
	{
		PRINT_ERR((mcl_stderr,
		"mcl_cb::finish_session_init: ERROR: mcl_init_layer_nb failed\n"))
		mcl_exit(1);
	}
	/*
	 * ... then create the socket endpoints,
	 */
	//if (this->ses_channel.sock_init(this) != MCL_OK)
	if (mcl_init_layer_sockets(this) != MCL_OK) {
		PRINT_ERR((mcl_stderr,
		"mcl_cb::finish_session_init: ERROR: socket initialization failed\n"))
		mcl_exit(1);
	}
	/*
	 * ... then create/initialize everything else
	 */
	if (this->is_a_sender()) {
		if (this->finish_init_as_a_sender() == MCL_ERROR) {
			goto bad;
		}
	} else {
		ASSERT(this->is_a_receiver());
		if (this->finish_init_as_a_receiver() == MCL_ERROR) {
			goto bad;
		}
	}
	TRACELVL(5, (mcl_stdout, "<- mcl_cb::finish_session_init:\n"))
	return MCL_OK;

bad:
	PRINT_ERR((mcl_stderr, "mcl_cb::finish_session_init: ERROR\n"))
	mcl_exit(-1);
	return MCL_ERROR;	// avoid warning
}


/**
 * Finishes the initialization of a sender.
 * => See header file for more informations.
 */
mcl_error_status
mcl_cb::finish_init_as_a_sender ()
{
	INT32	lay;
	txlay_t	*tl;
	double	sent_on_lower_layers;	/* nb of packets per tick */
					/* already sent on lower tx layers */
#ifdef DEBUG
	mcl_cb	*mclcb = this;		// required by TRACELVL
#endif
	double	du_per_tick_scale[MAX_NB_TX_LAYERS]; // predefined tx rate scale
	double	tps;			// ticks per second (also periodic
					// timer frequency).

	TRACELVL(5, (mcl_stdout, "-> mcl_cb::finish_init_as_a_sender: id=%d\n",
		this->get_id()))
	/* required below but... */
	ASSERT((this->mcl_max_group == MAX_NB_TX_LAYERS))
	/* init the txlay array first... */
	memset((void *)this->txlay_tab, 0, sizeof(this->txlay_tab));

	/*
	 * Calculate the transmission rate progression.
	 * Do not consider yet the rate on base layer...
	 */
#define MAX_NB_OF_DUS_SENT_PER_TICK_PER_LAYER	20
	mcl_calc_tx_scale(&du_per_tick_scale[0], 0,
			MAX_NB_OF_DUS_SENT_PER_TICK_PER_LAYER,
			MAX_NB_TX_LAYERS);
#undef MAX_NB_OF_DUS_SENT_PER_TICK_PER_LAYER
	/*
	 * initialize each txlay_t control block.
	 */
	sent_on_lower_layers = 0;
	tps = mcl_periodic_timer::get_frequency();
	for (lay = 0, tl = this->txlay_tab; lay < this->max_nb_layers; lay++, tl++) {
		tl->layer = lay;
		/* number of ticks per second */
		/* (in order to send important data every IMPORTANT_DATA_FREQUENCY (microseconds) on each layer)*/
		/* timers between layers are shifted */
		tl->tx_high_flush = false; /* not needed by default */
		tl->tx_high_timer = (int) floor((double) (IMPORTANT_DATA_FREQUENCY / MCL_TIMER_PERIOD) / (this->max_nb_layers - lay));	
		tl->tx_tab = new mcl_tx_tab();
		tl->tx_tab_high = new mcl_tx_tab();		
		tl->socket = &(this->socket_tab[lay]);
		tl->du_per_tick = (double)du_per_tick_scale[lay] *
					this->rate_on_base_layer / tps;
		tl->remaining_du_per_tick = 0.0;
		TRACELVL(5, (mcl_stdout,
		"   mcl_cb::finish_init_as_a_sender: layer %d: du_per_tick_scale=%f, rate_on_base_layer=%f, tps=%f\n",
			lay, du_per_tick_scale[lay],
			this->rate_on_base_layer, tps))
		/* take advantage of data sent on lower layer */
		tl->cumul_du_per_tick = sent_on_lower_layers + tl->du_per_tick;
		sent_on_lower_layers = tl->cumul_du_per_tick;
	}
	this->rx_thread = (mcl_thread_t)0;
	if (this->get_verbosity() >= 1 || this->get_stats_level() >= 1)
		mcl_print_tx_profile(this);
	TRACELVL(5, (mcl_stdout, "<- mcl_cb::finish_init_as_a_sender: OK\n"))
	return MCL_OK;
}


/**
 * Finishes the initialization of a receiver.
 * => See header file for more informations.
 */
mcl_error_status
mcl_cb::finish_init_as_a_receiver ()
{
#ifdef DEBUG
	mcl_cb	*mclcb = this;		// required by TRACELVL
#endif

	TRACELVL(5, (mcl_stdout,
	"-> mcl_cb::finish_init_as_a_receiver: id=%d\n", this->get_id()))
	this->rxlvl.socket_head = this->socket_tab;
	/*
	 * create the reception thread and lock
	 */
#ifdef WIN32
	if (CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE) mcl_rx_thread, (void*)this, 0, (LPDWORD)&this->rx_thread) == NULL) {
		perror("mcl_cb::finish_init_as_a_receiver: CreateThread");
		mcl_exit(1);
	}
#elif defined(AIX) || defined(HPUX) || defined(IRIX) || defined(FREEBSD) || defined(SOLARIS)
	{
	pthread_attr_t		attr;
	size_t			stack_size;
	pthread_attr_init(&attr);
	/*
	 * get the current stack size, and if lower than a given threshold,
	 * increase it.
	 */
	if (pthread_attr_getstacksize(&attr, &stack_size) != 0) {
		perror("mcl_cb::finish_init_as_a_receiver: pthread_attr_getstacksize");
		mcl_exit(1);
	}
#define MIN_STACK_SIZE	200 * 4096
	TRACELVL(5, (mcl_stdout,
		"   mcl_cb::finish_init_as_a_receiver: default stack size=%d, make sure it's no smaller than %d\n", MIN_STACK_SIZE))
	stack_size = max(stack_size, MIN_STACK_SIZE);
	if (pthread_attr_setstacksize(&attr, stack_size) != 0) {
		perror("mcl_cb::finish_init_as_a_receiver: pthread_attr_setstacksize");
		mcl_exit(1);
	}
	if ((pthread_create((pthread_t*)&this->rx_thread, &attr,
			    mcl_rx_thread, (void*)this)) != 0) {
		perror("mcl_cb::finish_init_as_a_receiver: pthread_create");
		mcl_exit(1);
	}
	}
#else	/* linux */
	if ((pthread_create((pthread_t*)&this->rx_thread, NULL, mcl_rx_thread, (void*)this)) != 0) {
		perror("mcl_cb::finish_init_as_a_receiver: pthread_create");
		mcl_exit(1);
	}
#endif
	TRACELVL(5, (mcl_stdout, "<- mcl_cb::finish_init_as_a_receiver: OK\n"))
	return MCL_OK;
}


/**
 *
 * => See header file for more informations.
 */
mcl_error_status
mcl_cb::set_max_datagram_size (INT32 sz)
{
	// sanity checks...
	if (sz < MIN_ALC_HEADER_SIZE || sz > MAX_DATAGRAM_SIZE) {
		PRINT_ERR((mcl_stderr,
			"mcl_cb::set_max_datagram_size: size out of range (expected [%d;%d], got %d).\n",
			MIN_ALC_HEADER_SIZE, MAX_DATAGRAM_SIZE, sz))
		return MCL_ERROR;
	}
	this->max_datagram_size = sz;
	this->payload_size = this->max_datagram_size - MAX_ALC_HEADER_SIZE;
	return MCL_OK;
}


/**
 *
 * => See header file for more informations.
 */
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


#if 0
/**
 * Set the delivery mode.
 * => See header file for more informations.
 */
mcl_error_status
mcl_cb::set_delivery_mode (mcl_cb *const mclcb,
			   mcl_delivery_mode mode)
{
	TRACELVL(5, (mcl_stdout, "-> mcl_cb::set_delivery_mode: %s",
		(mode == MCL_IMMEDIATE_DELIVERY) ? "IMMEDIATE_DELIVERY" :
						   "ORDERED_DELIVERY"))
	this->delivery_mode = mode;
	return MCL_OK;
}
#endif


/**
 * Tx rate (in DUs per tick) calculations.
 * => See header file for more informations.
 */
void
mcl_cb::mcl_calc_tx_scale (double	*tab,
			   INT32	start,
			   double	max,
			   INT32	nb_entries)
{
	double		val;
	double		*last;	// last valid entry in table to consider.

	ASSERT(tab && start >= 0 && nb_entries > 0);
	last = &tab[nb_entries - 1];
#ifdef RLC
	if (this->congestion_control == RLC_CC) {
		val = 1 << start; 	// value for base layer.
		*tab++ = val;		// tab[0] == tab[1] for a progression 1 1 2...
		for (; tab <= last ; tab++) {
			/* taking the min() leads to a less aggressive RLC, taking
			 * val follows the initial exponential scheme of RLC. */
			//*tab = val;
			*tab =min(val, max);
			val *= 2;
		}
	}
#endif
#ifdef FLIDS
	if (this->congestion_control == FLID_SL_CC) {
		val =  pow(1.3, start); // value for base layer.
		*tab++ = val;		// tab[0] == tab[1] for a progression 1 1 2...
		for (INT32 i = 1; tab <= last ; i++, tab++) { 
			*tab = (pow(1.3, i) - pow(1.3, i - 1)) * val;
		}
	}
#endif
	if (this->congestion_control == NO_CC) {
		*tab = 1.0;
	}
}

