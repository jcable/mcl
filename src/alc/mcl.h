/* $Id: mcl.h,v 1.7 2005/03/18 12:06:15 roca Exp $ */
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

#ifndef MCL_H  /* { */
#define MCL_H


/****** general purpose enumerations ******************************************/

/**
 * Used to differentiate between sending and receiving variants when needed.
 */
enum mcl_tx_or_rx {
	MCL_RX = 0,
	MCL_TX = 1
};

#define SENDER		MCL_TX
#define RECEIVER	MCL_RX


/****** constants: do not edit ************************************************/

/*
 * print stats every STATS_PERIOD usec at most
 */
#define STATS_PERIOD	5000000

/*
 * call the memory cleanup functin with this period (in time tick)
 */
#define TX_MEM_CLEANUP_PERIOD	50

#if 0
/*
 * sender or receiver; used as argument to some functions
 * do not change...
 */
#define SENDER		1
#define RECEIVER	0
#endif

/*
 * used by mcl_drop_layer; check or really drop ?
 */
#define		MCL_CHECK_ONLY		1
#define		MCL_DO_IT		0
#define		MCL_ALL_LAYERS		254	/* used by add/drop functions */
#define		MCL_HIGHEST_LAYER	255	/* used by add/drop functions */

/*
 * tx/rx modes
 * differentiate application mode and signaling mode (usually piggy-backed)
 */
#define	MODE_UNI_TX	0x01		/* can send in unicast */
#define	MODE_MCAST_TX	0x02		/* can send in multicast */
#define	MODE_UNI_RX	0x04		/* can receive in unicast */
#define	MODE_MCAST_RX	0x08		/* can receive in multicast */
					/* SIG currently only used by ODL but */
//#define MODE_SIG_UNI_TX	0x10	/* can send signaling in unicast */
//#define MODE_SIG_UNI_RX	0x20	/* can recv signaling in unicast */

/*
 * How many times should we transmit each kind of signalization information
 * (for increased reliability) ? Tx for ever if value is -1.
 * NB: for CLOSE the nb of tx must be > 0 (10 times seems ok)
 */
#define MAX_TX_EXT_NONEWADU	-1
#define MAX_TX_SIG_CLOSE	10


/****** prototypes ************************************************************/

extern void	mcl_moreabout	(void);
extern void	mcl_exit	(INT32 n);
extern void	mcl_global_init	(void);
extern void	mcl_stdout_stderr_init (void);


/****** global variables shared by all sessions *******************************/

/** MCL version of standard output stream. */
extern FILE	*mcl_stdout;
/** MCL version of standard error stream. */
extern FILE	*mcl_stderr;

/** Initial sequence number */
extern const UINT32	mcl_iss;
/**
 * Local time in usec.
 * Incremented by the period_proc thread.
 */
extern UINT32		mcl_time_count;

extern mcl_thread_t	mcl_timer_thread_id;

/** default temporary file location. */
extern char		mcl_tmp_dir_name[MAX_FILE_NAME_LEN];

extern class mcl_periodic_timer	mcl_periodic_timer_obj;


#endif /* }  MCL_H */

