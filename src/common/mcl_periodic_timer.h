/* $Id: mcl_periodic_timer.h,v 1.4 2005/05/17 12:36:59 roca Exp $ */
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

#ifndef MCL_PERIODIC_TIMER_H
#define MCL_PERIODIC_TIMER_H


/**
 * Period of the main periodic timer.
 * Used to trigger all the periodic processings (data transmissions
 * essentially, congestion control periodic processing, etc.)
 * This value (in microseconds) determines the internal MCL clock
 * granularity.
 * It is the sleep duration between two consecutive virtual time ticks.
 * All other timers are multiples of this period...
 */
const INT32	MCL_PERIODIC_TIMER_PERIOD = 10000;	// 10 ms => 100 Hz
//const INT32	MCL_PERIODIC_TIMER_PERIOD = 20000;	// 20 ms =>  50 Hz


/**
 * MCL timer static class for periodic processing.
 * There is only one instance of this class, allocated statically.
 *
 * Time is discretized into itime ticks (see src/common/mcl_itime.h|cpp).
 * This class provides a timer that is to be used whenever dealing with
 * periodic tasks. It is therefore different from the mcl_timer class
 * which is dedicated to one-time timer events.
 * For instance, mcl_periodic_timer is used for rate-controled transmissions.
 * It is made possible by a dedicated thread that awakes regularly and
 * calls the appropriate MCL functions that need periodic processing.
 * The sleep duration takes into account the time spent in these functions
 * and in case a large processing suspends temporarily the service, the
 * various processing functions will be called as many times as required
 * to compensate for the missed cycles. It guaranties that on average the
 * period of each individual periodic processing functions is achieved.
 * The mcl_time_count global variable, which reflects the nb of "ticks" 
 * elapsed since the begining, is updated here.
 */
class mcl_periodic_timer {
 
public:
	/****** Public Members ************************************************/
 
	/**
	 * Starts the periodic_timer service.
	 * Exits in case of error.
	 */
	static void	start ();

	/**
	 * Returns the periodic timer frequency in Hz.
	 */
	static double	get_frequency ();

	/**
	 * Returns the periodic timer period in seconds.
	 */
	static double	get_period ();

	/**
	 * Returns true if the activity is too high and transmission must be
	 * slown down.
	 */
	static bool	must_reduce_activity ();

	/****** Public Attributes *********************************************/
 

private:
	/****** Private Members ***********************************************/

	/**
	 * Timer thread, with a 1/MCL_TIMER_PERIOD fixed frequency.
	 * @param arg	unused
	 */
	static void	*timer_thread (void *arg);

	/**
	 * Set the periodic timer frequency
	 */
	static void	set_frequency (INT32 freq);

	/****** Private Attributes ********************************************/
	static bool		no_time_to_sleep;
				// true when the activity is far too high.
				// This is symptomatic of a problem, so
				// we must try to find a solution then...
	static mcl_thread_t	timer_thread_id; // idf returned at creation
};


//------------------------------------------------------------------------------
// Inlines for all classes follow
//------------------------------------------------------------------------------

inline double
mcl_periodic_timer::get_frequency ()
{
	return ((double)1000000.0 / (double)MCL_PERIODIC_TIMER_PERIOD);
}

inline double
mcl_periodic_timer::get_period ()
{
	return (double)0.000001 * (double)MCL_PERIODIC_TIMER_PERIOD;
}

inline bool
mcl_periodic_timer::must_reduce_activity ()
{
	return no_time_to_sleep;
}

#endif // !MCL_PERIODIC_TIMER_H

