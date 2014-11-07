/* $Id: mcl_rx_thread.cpp,v 1.3 2004/01/30 16:27:43 roca Exp $ */
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

mcl_rx_thread::mcl_rx_thread ()
{
	rx_thread_id = 0;
}


mcl_rx_thread::~mcl_rx_thread ()
{
}


void
mcl_rx_thread::start (mcl_cb	*const mclcb)
{
	/*
	 * create the timer thread...
	 */
#ifdef WIN32
	if (CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE) mcl_rx_thread::rx_thread,
			 NULL, 0, (LPDWORD)mclcb) == NULL) {
		perror("mcl_rx_thread::start: CreateThread");
		mcl_exit(1);
	}
#else
	if (pthread_create((pthread_t*)&(this->rx_thread_id),
				NULL,
				mcl_rx_thread::rx_thread,
				(void*)mclcb) != 0) {
		perror("mcl_rx_thread::start: pthread_create");
		mcl_exit(1);
	}
#endif
}


/**
 * Reception thread.
 * This thread polls data regularly on the various mcast groups...
 */
void *
mcl_rx_thread::rx_thread (void	*arg)
{
	mcl_cb	 *mclcb = (mcl_cb*)arg;

	/*
	 * we don't sleep here but in the recv_pkt() func.
	 * the unlock will be done there...
	 */
	ASSERT(mclcb != NULL);
	TRACELVL(5, (mcl_stdout, "-> RxThread:\n"))
#ifndef WIN32
	pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
	/*
	 * cancellation is deferred till next check point , i.e. points
	 * in code where we know everything is in a stable state
	 */
	pthread_setcanceltype(PTHREAD_CANCEL_DEFERRED, NULL);
	pthread_testcancel();
#endif
  	mclcb->lock();
	while (1) {
		if (mclcb->fsm.is_closed(mclcb)) {
			/* everything is finished */
			break;
		}
		TRACELVL(6, (mcl_stdout,
			"mcl_rx_thread::rx_thread: ses_id=%d, time_count=%d, state %s\n",
			mclcb->get_id(), mcl_time_count,
			mclcb->fsm.print_rx_state(mclcb)))
		if (mclcb->is_a_receiver() &&
		    mclcb->fsm.no_new_undup_du(mclcb)) {
			/*
			 * we received all DUs...
			 * we now wait that the application finishes to read
			 * all data and issues a final mcl_close().
			 */
			TRACELVL(6, (mcl_stdout,
			"mcl_rx_thread::rx_thread: waiting final mcl_close() call\n"))
			mclcb->unlock();
#ifndef WIN32
			pthread_testcancel();
#endif
			mcl_usleep(DFLT_POLLING_PERIOD);
#ifndef WIN32
			pthread_testcancel();
#endif
			mclcb->lock();
			TRACELVL(6, (mcl_stdout,
			"mcl_rx_thread::rx_thread: waiting final mcl_close() call\n"))
		} else {
			/*
			 * get new packets...
			 * this call is blocking, waiting new packets to arrive
			 */
			if (mclcb->ses_channel.recv_pkt(mclcb) == MCL_ERROR) {
				PRINT_ERR((mcl_stderr, "mcl_rx_thread::rx_thread: recv_pkt failed\n"))
				mcl_exit(-1);

			}
		}
	}
	TRACELVL(5, (mcl_stdout, "<- RxThread:\n"))
	mclcb->unlock();
	arg = 0;
#ifdef WIN32
	ExitThread(0);
#else
	pthread_exit(arg);
#endif
	return arg;	/* unused */
}

