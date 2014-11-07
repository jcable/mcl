/* $Id: mcl_cb.h,v 1.32 2005/05/23 11:11:44 roca Exp $ */
/*
 *  Copyright (c) 2003-2004 INRIA - All rights reserved
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

#ifndef MCL_CB_H  /* { */
#define MCL_CB_H

#include "mcl_fec.h"	/* cross-references */


/**
 * TRANSMISSION layer control block.
 * One entry for each layer used by the sender.
 */
typedef struct txlay {
	INT32		layer;		/* tx layer (starts at 0) */
	double		cumul_du_per_tick;/* target cumulative tx rate per tick */
	double		du_per_tick;	/* target tx rate per tick */
	double		remaining_du_per_tick; /* number of DUs not sent */
					/* during previous tick, that should */
					/* have been sent though to match */
					/* target rate */
	class mcl_socket *socket;	/* mcast group where data is sent */
	mcl_tx_tab	*tx_tab;	/* tx plannification table for */
					/* normal priority data */
	mcl_tx_tab	*tx_tab_high;	/* tx plannification table for high */
					/* priority data*/
	INT32		tx_high_timer;	/* timer for high priority data, in
					   ticks. Tx is done when going to 0. */
	bool		tx_high_flush;	/* flush (i.e. send) the entire content
					   of tx_high immediatly. */
#if 0
//#ifdef RLC
	char		wait_sp;	/* boolean: 1 = no tx on lay i before */
					/* sending the first SP on layer i-1 */
	char		wait_after_sp_count;/* wait this # ticks after wait=0 */
#endif
} txlay_t;


/**
 * RECEPTION control block.
 * Only one entry at the receiver, no matter the number of layers.
 */
typedef struct {
	class mcl_socket *socket_head;	/* head of mcast group tab */
	fd_set		fds;		/* set of fd, used by select */
	int		nfds;		/* highest-numbered fd + 1 */
	int		n_fd;		/* total nb of fd (ie sockets) */
	du_t		*du_head;	/* head of the orphan DU list */
#ifdef FEC
	du_t		*dufec_head;	/* head of the DU_FEC list */
#endif
} rxlay_t;


/**
 * Possible session modes.
 */
enum mcl_tx_rx_mode {
	MCL_INVALID,
	MCL_IS_A_SENDER_ONLY,
	MCL_IS_A_RECEIVER_ONLY
};

/**
 * Control Block Class for the MCL_ALC session.
 * Contains all the variables, classes, member functions etc. required for
 * a session.
 */
class mcl_cb {
 
public:

	/****** Public Members ************************************************/
	/**
	 * Session control block constructor.
	 * Perform pre-initialization of the session.
	 * Called by mcl_open().
	 * At completion, the session is NOT fully initialized, but
	 * sufficiently to enable most tasks to be performed (e.g all
	 * threads are created).
	 * @param mode	determines if this is a sender or receiver
	 */
	mcl_cb (mcl_tx_rx_mode mode);

	/** Destructor */
	~mcl_cb ();

	/**
	 * Finish the initialization of the session if not already done.
	 * This end of init is done only once.
	 * @param nb_lay	number of layers
	 * @return		Completion status (MCL_OK or MCL_ERROR).
	 */
#ifdef SVSOA_RECV
	mcl_error_status  finish_session_init_if_needed (int nb_lay);
#else
	mcl_error_status  finish_session_init_if_needed ();
#endif

	/**
	 * True once the session if fully initialized.
	 * @return	boolean
	 */
	bool		is_fully_initialized ();

	/**
	 * Free everything.
	 * This is done at the very last stage, at the end of session close
	 * or abort, just before calling the destructor.
	 */
	void		free_everything ();

	/**
	 * Returns the local MCL session id.
	 * The session identifier is set once, at session creation.
	 * @return	session id
	 */
	INT32		get_id();

	/**
	 * Get the tx/rx mode for this session.
	 * @return	mode
	 */
	mcl_tx_rx_mode	get_mode ();

	/**
	 * Is this session essentially a sending session.
	 * @return	returns true when the MCL session has been opened
	 *		in "w" and "wr" modes, false otherwise.
	 */
	bool		is_a_sender ();

	/**
	 * Is this session essentially a receiving session.
	 * @return	returns true when the MCL session has been opened
	 *		in "r" and "rw" modes, false otherwise.
	 */
	bool		is_a_receiver ();

	/**
	 * Set/unset the FLUTE compatibility mode.
	 * @param flute	boolean: true in FLUTE compatibility mode
	 *			 (default: false)
	 */
	void		set_flute_mode (bool	onoff);

	/**
	 * True in FLUTE compatibility mode.
	 * @return	true in FLUTE compatibility mode, false otherwise.
	 */
	bool		is_flute_compatible ();

	/**
	 * Set the session lock.
	 * There is one lock per MCL session. Their total number is equal
	 * to the number of concurrent sessions.
	 */
	void		lock ();

	/**
	 * Try to set the session lock.
	 * There is one lock per MCL session. Their total number is equal
	 * to the number of concurrent sessions.
	 * @return	return EBUSY if not possible, 0 if ok
	 */
	INT32		trylock ();

	/**
	 * Release the session lock.
	 */
	void		unlock ();

	/**
	 * Set the maximum datagram size (in bytes) and adjusts the payload
	 * size in consequence.
	 * A datagram includes the payload (if any) plus the ALC/LCT header.
	 * @return	Completion status (MCL_OK or MCL_ERROR).
	 */
	mcl_error_status	set_max_datagram_size (INT32 sz);

	/**
	 * Get the maximum datagram size (in bytes).
	 * A datagram includes the payload (if any) plus the ALC/LCT header.
	 * @return	maximum datagram size in bytes
	 */
	INT32			get_max_datagram_size () const;

	/**
	 * Set the payload size (in bytes).
	 * Only the data payload of the ALC/LCT datagram is considered here.
	 * @return	Completion status (MCL_OK or MCL_ERROR).
	 */
	mcl_error_status	set_payload_size (INT32 sz);

	/**
	 * Get the payload size (in bytes).
	 * Only the data payload of the ALC/LCT datagram is considered here.
	 * Can be set independantly of the max_datagram_size but cannot
	 * be larger than this latter.
	 * @return	payload size in bytes
	 */
	INT32			get_payload_size () const;

	/**
	 * Set the verbosity level.
	 * @param level
	 * @return	Completion status (MCL_OK or MCL_ERROR).
	 */
	mcl_error_status	set_verbosity (INT32 level);

	/**
	 * Get the verbosity level.
	 * @return	verbosity level
	 */
	INT32			get_verbosity ();

	/**
	 * Set the statistic level.
	 * @param level	0 means no stats at all, 2 means all statistics,
	 *		1 means only major statistics (e.g. at session end).
	 * @return	Completion status (MCL_OK or MCL_ERROR).
	 */
	mcl_error_status	set_stats_level (INT32 level);

	/**
	 * Get the statistic level.
	 * @return	statistic level
	 */
	INT32			get_stats_level ();


	/****** Public Attributes *********************************************/
	/*
	 * All the classes/structs required by a session follow...
	 */
	/*
	 * SHARED BY SENDERS AND RECEIVERS
	 */
	class mcl_fsm		fsm;
	class mcl_periodic_proc	periodic_proc;	// For all periodic tasks
	class mcl_fec		fec;		// FEC encoding/decoding class
						// for all FEC flavors and the
						// associated parameters
	stats_t			stats;		// tx and rx statistics
	//class mcl_ses_channel	ses_channel;	// session channel
	//class mcl_alc_pkt_mgmt alc_pkt_mgmt;

	rlccb_t			rlccb;		// RLC control block
	flids_cb_t		flids_cb;	// FLIDS control block 

#ifdef METAOBJECT_USED
	class mcl_meta_object_layer	*meta_obj_layer; // Meta object mgmt
#endif

	/*
	 * SENDER SPECIFIC
	 */
	class mcl_tx		tx;		// Transmission class
	//class mcl_group_mgmt	group_mgmt;
	//class mcl_tx_ctrl	tx_ctrl;	// Performs control tasks
	//class mcl_tx_window	tx_window;	// Store ADUs/DUs to send
	class mcl_tx_flute	tx_flute;	// FLUTE specific tx functions
	//class mcl_tx_storage	tx_storage;	// Virtual tx memory management

	/*
	 * RECEIVER SPECIFIC
	 */
	class mcl_rx		rx;		// Reception class
	//class mcl_rx_thread	rx_thread;	// Reception thread
	//class mcl_rx_ctrl	rx_ctrl;	// Performs control tasks
	class mcl_rx_window	rx_window;	// Store received ADUs 
	class mcl_rx_flute	rx_flute;	// FLUTE specific rx functions
	class mcl_rx_storage	rx_storage;	// Virtual rx memory management


	/*
	 * And some additional public variables...
	 */

	/*
	 * The number of layers, nb_layers, can vary in [1, max_nb_layers].
	 * The maximum nb of layers, max_nb_layers, can vary in
	 *	[1, MAX_NB_TX_LAYERS].
	 * All table allocations are done with MAX_NB_TX_LAYERS.
	 */
	INT32		nb_layers;	/** Current number of tx layers. */
	INT32		max_nb_layers;	/** Maximum number of tx layers. */

	/** Array of ctrl blocks for the sending side, one per layer. */
	txlay_t		txlay_tab[MAX_NB_TX_LAYERS];

	/* Control block for receiving side, only one, no matter the number
	 * of layers. */
	rxlay_t		rxlvl;

#ifdef WIN32
	BOOL test_cancel;	/* used as a pthread_testcancel() alternative */
#endif


	/**********************************************************************/
	/*** Stuff below is here for historical reasons, will be removed... ***/

	/*
	 * mode selection variables
	 */
	char ucast_mcast_mode;	/* UNI_TX MCAST_TX UNI_RX MCAST_RX */
	bool single_layer_mode;	/* optimize for single layer mode */
	mcl_congestion_control_scheme congestion_control;
				/* congestion control mode: */
				/*	NO_CC: no congestion control */
				/*	RLC_CC: RLC */
				/*	FLID_SL_CC: FLID-SL */
				/* Required for FLUTE interoperability tests */

	/*
	 * time related variables
	 */
	int last_periodic_proc_tc;/* time_count for last do_periodic_proc call*/
	mcl_itime_t last_periodic_proc_it;/* idem with itime */
	int tx_mem_cleanup_count; /* tx memory cleanup function call period */
	int stats_time_count;	/* for periodic stats print */

	/*
	 * COMMON TX/RX VARIABLES
	 */
	int demux_label;	/* LCT demux label, or tx session ID (TSI) */
	int cc_id;
	char *mcast_if_name;	/* name of mcast interface to use or NULL */
	class mcl_addr *mcast_if_addr;/* addr of mcast intf to use or NULL */
	class mcl_addr addr;	/* unicast/mcast address and port on which */
				/* to rx (client) or tx (source). This is the */
				/* ALC session addr/port. With mcast, it uses */
				/* the ranges: */
				/*   [addr; addr+max_nb_layers[ for addr, and */
				/*   [port; port+max_nb_layers[ for port nb */
	UINT16 ttl;		/* default TTL used with mcast */
	/*
	 * TX SPECIFIC VARIABLES
	 */
	char delivery_mode;	/* which delivery mode? on-demand/push/... */
	double rate_on_base_layer;	/* tx rate in packets/s on base layer */
	int nb_tx;		/* # of desired tx for each DU */
	float remaining_tx_tick_nb;/* fractional tick_nb in do_periodic_proc */
	sig_tab_t *sig_tab;	/* used by mcl_sig.c */
	int skip;		/* used by mcl_sig.c */
	sig_tab_t *psig_next;	/* used by mcl_sig.c */
	int mcl_sig_pending;	/* is there any SIG msg pending ? counter */

#ifdef ANTICIPATED_TX_FOR_PUSH
	char anticipated_tx_for_push; /* optimization for tx in PUSH mode */
#endif
#ifdef VIRTUAL_TX_MEM
	bool vtm_used;		/* boolean: 1 to use virtual tx memory service*/
	bool vtm_initialized;	/* boolean: 1 if vtm_cb is valid */
	vtm_cb_t vtm_cb;	/* vtm control block for that session */
#endif
	int scheduler;		/* shed to use for next UpdateTxPlanning call */
	int adu_scheduling;	/* SEQUENTIAL, MIXED, PARTIALLY_MIXED, RANDOM */

	/*
	 * RECEIVER SPECIFIC VARIABLES
	 */
	int ready_data;		/* # completed ADU not yet returned to appli */
#ifdef FEC
	bool postpone_fec_decoding; /* decode during reception of at the end? */
				/* only valid with RSE, doesn't apply to LDPC */
#endif
	
	/*
	 * various additional state/context
	 */
#ifdef SIMUL_LOSSES
	bool simul_losses_state;
#endif
	int mcl_max_group;	/* nb of mcast groups */
	class mcl_socket socket_tab[MAX_MC_GROUP]; /* array of mcast group contexts */
	mcl_thread_t rx_thread;	/* rx thread idf */
	/*
	 * various working variables
	 */
	adu_t	*findadu_cache;		/* adu cache used by FindADU function */
 	adu_t	*lastadu_cache;		/* adu cache to point to the last adu 
					 * returned/or announced to appli */
 

private:
	/****** Private Members ***********************************************/
	/**
	 * Finish the initialization of the session.
	 * This function is called by once finish_session_init_if_needed()
	 * after that all the appropriate params have been set up with
	 * mcl_ctl().
	 * It initializes everything (context, sockets, threads, etc.).
	 * @param nb_lay	number of layers
	 * @return		Completion status (MCL_OK or MCL_ERROR).
	 */
#ifdef SVSOA_RECV
	mcl_error_status	finish_session_init (int nb_lay);
#else
	mcl_error_status	finish_session_init ();
#endif

	/**
	 * Finishes the initialization of a sender.
	 * @return		Completion status (MCL_OK or MCL_ERROR).
	 */
	mcl_error_status	finish_init_as_a_sender ();

	/**
	 * Finishes the initialization of a receiver.
	 * @return		Completion status (MCL_OK or MCL_ERROR).
	 */
	mcl_error_status	finish_init_as_a_receiver ();

	/* 
	 * Tx rate (in DUs per tick) calculations.
	 * The exact transmission scale depends on the congestion control
	 * protocol used.
	 * With RLC: exponential scale (factor 2) up to "max" DUs/tick, and then
	 * linear scale of "max" DUs/tick
	 * With FLID-SL: exponential scale (factor 1.3).
	 * @param tab		table where to store the result.
	 * @param start		initial power of 2 (RLC) or 1.3 (FLIDS).
	 * @param max		with RLC, maximum number of DUs per tick.
	 * @param tab_len	number of entries in table to consider.
	 */
	void mcl_calc_tx_scale (double *tab, INT32 start, double max, INT32	tab_len);


	/****** Private Attributes ********************************************/
	bool		fully_initialized; // true when session is fully init'ed
					// ie. after finish_session_init()
	INT32		id;		// MCL session identifier
	mcl_tx_rx_mode	tx_rx_mode;	// is it a sender or a receiver?
	bool		flute_mode;	// true in FLUTE compatibility mode,
					// false otherwise
	INT8		verbosity_level;// debug trace level 
	INT8		stats_level;	// statistics level
	mcl_mutex_t	session_lock;	// global session lock
 
	INT32		max_datagram_size; // max datagram size for tx and rx
	INT32		payload_size;	// default payload size for tx and rx
				// the actual DU size used for a given ADU is
				// kept in the adu->symbol_len, and may change
				// for different ADUs (not yet supported!)
};


/**
 * Table containing the various control blocks of the sessions.
 * There can be at most MCLCB_MAX_ID simultaneous sessions in an MCL-ALC
 * instance.
 */
extern mcl_cb	*mclcb_tab[MAX_NB_MCLCB];

/** One time initialization function for the mclcb table. */
extern void mcl_init_mclcb_tab (void);


//------------------------------------------------------------------------------
// Inlines for all classes follow
//------------------------------------------------------------------------------


inline INT32
mcl_cb::get_id()
{
	return this->id;
}

inline mcl_tx_rx_mode
mcl_cb::get_mode ()
{
	return this->tx_rx_mode;
}

inline bool
mcl_cb::is_a_sender ()
{
	//return (this->tx_rx_mode == MCL_IS_A_SENDER_ONLY ||
	//	this->tx_rx_mode == MCL_IS_A_SENDER_AND_RECEIVER);
	return (this->tx_rx_mode == MCL_IS_A_SENDER_ONLY);
}

inline bool
mcl_cb::is_a_receiver ()
{
	//return (this->tx_rx_mode == MCL_IS_A_RECEIVER_ONLY ||
	//	this->tx_rx_mode == MCL_IS_A_RECEIVER_AND_SENDER);
	return (this->tx_rx_mode == MCL_IS_A_RECEIVER_ONLY);
}

inline void
mcl_cb::set_flute_mode (bool	onoff)
{
	this->flute_mode = onoff;
}

inline bool
mcl_cb::is_flute_compatible ()
{
	return this->flute_mode;
}

inline INT32
mcl_cb::get_verbosity ()
{
	return (INT32) this->verbosity_level;
}

inline INT32
mcl_cb::get_stats_level ()
{
	return (INT32) this->stats_level;
}

inline void
mcl_cb::lock ()
{
	mcl_lock(&(this->session_lock));
}

inline INT32
mcl_cb::trylock ()
{
	return mcl_trylock(&(this->session_lock));
}

inline void
mcl_cb::unlock ()
{
	mcl_unlock(&(this->session_lock));
}

inline mcl_error_status
#ifdef SVSOA_RECV
mcl_cb::finish_session_init_if_needed (int nb_lay)
#else
mcl_cb::finish_session_init_if_needed ()
#endif
{
	if (this->fully_initialized)
		return MCL_OK;
	else
#ifdef SVSOA_RECV
		return (this->finish_session_init(nb_lay));
#else	
		return (this->finish_session_init());
#endif
}

inline bool
mcl_cb::is_fully_initialized ()
{
	return this->fully_initialized;
}

inline INT32
mcl_cb::get_max_datagram_size () const
{
	return this->max_datagram_size;
}

inline INT32
mcl_cb::get_payload_size () const
{
	return this->payload_size;
}

#endif /* }  MCL_CB_H */
