/* $Id: mcl_periodic_proc.h,v 1.2 2004/01/30 16:27:43 roca Exp $ */
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

#ifndef MCL_PERIODIC_PROC_H
#define MCL_PERIODIC_PROC_H


/**
 * This class gathers additional variables and member functions that are
 * specific to each MCL session. This class is included by mcl_cb class.
 */
class mcl_periodic_proc {

public:
	/****** Public Members ************************************************/
 
	/**
	 * Default constructor.
	 */
	mcl_periodic_proc ();

	/**
	 * Default destructor.
	 */
	~mcl_periodic_proc ();

	/**
	 * Scan all the MCL sessions.
	 * Called periodically, unlocked, by the mcl_periodic_timer thread.
	 * There is only one instance of this function since it is
	 * independant of the MCL session.
	 */
	static void	scan_all_sessions (void);

	/**
	 * Calls the various functions that perform periodic processing for
	 * a given session.
	 * Called by scan_all_sessions() for each valid mcl_cb, and also
	 * called (locked), by mcl_send(to)() to anticipate data
	 * transmissions while doing FEC encoding.
	 * @param mclcb	session considered
	 */
	void	do_periodic_proc (mcl_cb *mclcb);


	/****** Public Attributes *********************************************/

	/** Time_count of last do_periodic_proc call. */
	int	last_periodic_proc_tc;
	/** itimer of last do_periodic_proc call. */
	mcl_itime_t	last_periodic_proc_it;
	/** Fractional tick_nb in do_periodic_proc. */
	float	remaining_tx_tick_nb;

#if 0
	/** tx memory cleanup function call counter. */
	int tx_mem_cleanup_count;
	/** periodic stats print counter. */
	int stats_time_count;
#endif // 0

private:
	/****** Private Members ***********************************************/

	/****** Private Attributes ********************************************/

};


//------------------------------------------------------------------------------
// Inlines for all classes follow
//------------------------------------------------------------------------------


#endif // !MCL_PERIODIC_PROC_H
