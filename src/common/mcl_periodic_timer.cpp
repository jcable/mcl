/* $Id: mcl_periodic_timer.cpp,v 1.2 2004/01/30 16:27:42 roca Exp $ */
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

#ifdef ALC
#include "../alc/mcl_includes.h"

#elif defined(NORM)

#include "../norm/mcl_includes.h"
#endif // ALC|NORM


/****** mcl_periodic_timer class **********************************************/


/* static data members init */
bool		mcl_periodic_timer::no_time_to_sleep = false;
mcl_thread_t	mcl_periodic_timer::timer_thread_id = 0;


void
mcl_periodic_timer::start ()
{
#ifdef DEBUG
	static bool	initialized = false;

	ASSERT(initialized == false);	// check that it is called only once
	initialized = true;
#endif

	mcl_periodic_timer::no_time_to_sleep = false;
	/*
	 * create the timer thread...
	 */
#ifdef WIN32
	if (CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE) mcl_periodic_timer::timer_thread,
			 NULL, 0, (LPDWORD)&(mcl_periodic_timer::timer_thread_id)) == NULL) {
		perror("mcl_periodic_timer::start: CreateThread");
		mcl_exit(1);
	}
#else
	if (pthread_create((pthread_t*)&(mcl_periodic_timer::timer_thread_id),
				NULL, mcl_periodic_timer::timer_thread,
				(void*)NULL) != 0) {
		perror("mcl_periodic_timer::start: pthread_create");
		mcl_exit(1);
	}
#endif
}


/**
 * Timer thread, with a 1/MCL_TIMER_PERIOD fixed frequency
 * The mcl_time_count global variable, which reflects the nb of "ticks" 
 * elapsed since the begining, is updated here.
 */
void *
mcl_periodic_timer::timer_thread (void *arg)
{
	//INT32		ses;		// session id
	//mcl_cb		*mclcb;		// session control block
	mcl_itime_t	start_itime;	// to calculate exact time to sleep
	mcl_itime_t	end_itime;	// to calculate exact time to sleep
	INT32		dt;		// time diff in microseconds
	INT32		dt2sleep;	// sleep that nb of microseconds
	mcl_itime_t	timer_thread_last_wakeup;
					// last time timer_thread woke up
	float		timer_thread_rem_tc; // fractional part of time_count
	float		true_tc;	// elapsed time_count as a float
	INT32		tc;		// integral part of true_tc

	start_itime = mcl_get_itime();
	timer_thread_last_wakeup = start_itime;
	timer_thread_rem_tc = 0.0;

	while (1) {
		/*
		 * this sleep determines the timer granularity...
		 * it takes into account the processing time spent in the
		 * various calls to mcl_do_periodic_proc()
		 */
		end_itime = mcl_get_itime();
		dt = mcl_it2usec(mcl_itime_sub(end_itime, start_itime));
		// don't sleep less than 0.1ms
		dt2sleep = max(100, (int)MCL_PERIODIC_TIMER_PERIOD - dt);
		dt2sleep = min(MCL_PERIODIC_TIMER_PERIOD, dt2sleep);
		if (dt2sleep == 100) {
			no_time_to_sleep = true;
//#ifdef DEBUG
#if 0
			PRINT_ERR((mcl_stderr, "mcl_periodic_timer::timer_thread: WARNING, no time to sleep\n"))
#endif
		} else {
			no_time_to_sleep = false;
		}
		// now let's sleep...
		mcl_usleep(dt2sleep);
		start_itime = mcl_get_itime();

		/*
		 * update the global MCL time (in time ticks), the
		 * mcl_time_count var. There is one tick every
		 * MCL_TIMER_PERIOD micro-seconds
		 */
		true_tc = (float)mcl_it2usec(mcl_itime_sub(start_itime, timer_thread_last_wakeup)) / (float)MCL_PERIODIC_TIMER_PERIOD + (float)timer_thread_rem_tc;
		tc = (int)(true_tc);
		mcl_time_count += tc;
		//printf("THREAD:: start_itime=%d, timer_thread_last_wakeup=%d, timer_thread_rem_tc=%f, true_tc=%f, mcl_time_count=%d\n", start_itime, timer_thread_last_wakeup, timer_thread_rem_tc, true_tc, mcl_time_count);
		timer_thread_last_wakeup = start_itime;
		timer_thread_rem_tc = true_tc - tc;

		/*
		 * scan all the sessions...
		 * this is where the real work is done for each mcl_cb.
		 */
		mcl_periodic_proc::scan_all_sessions();
	}
	arg = 0;
#ifdef WIN32
	ExitThread(0);
#else
	pthread_exit(arg);
#endif
	return arg;	/* unused */
}

