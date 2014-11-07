/* $Id: mcl_timer.h,v 1.2 2004/01/30 16:27:43 roca Exp $ */
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

/*
 * contains code derived from:
 */
//============================================================================
// Copyright (c) 1997-2001 TASC, Inc.
// Copyright (c) 1997-2001 University of Massachusetts at Amherst
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions
// are met:
// 
// 1. Redistributions of source code must retain the above copyright
//    notice, this list of conditions and the following disclaimer.
// 2. Redistributions in binary form must reproduce the above copyright
//    notice, this list of conditions and the following disclaimer in the
//    documentation and/or other materials provided with the distribution.
// 3. All advertising materials mentioning features or use of this software
//    must display the following acknowledgement:
//      This product includes software developed by TASC, Inc. and the
//      University of Massachusetts at Amherst.
// 4. The names of TASC, Inc. and the University of Massachusetts at Amherst
//    may not be used to endorse or promote products derived from this
//    software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE CONTRIBUTORS ``AS IS'' AND ANY EXPRESS
// OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
// WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
// DISCLAIMED.  IN NO EVENT SHALL CONTRIBUTORS BE LIABLE FOR ANY DIRECT,
// INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
// (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
// SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
// HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
// STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING
// IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
// POSSIBILITY OF SUCH DAMAGE.

#ifndef MCL_TIMER_H
#define MCL_TIMER_H

#include "mcl_timer_handler.h"

/**
 * Class that provides static methods that allow events to be timed, resulting
 * in callbacks.  This class runs as a seperate thread and is itself thread-
 * safe.  Timers may be set when inside of timer callbacks.
 * @version  $Revision: 1.2 $
 * @author   Mark Keaton
 */
class mcl_timer {
  
public:
  
  /**
   * Create a timer for an event.  When at least the amount of time specified
   * elapses, the TimerCallback() method of the specified object is called
   * passing the specified arguments.
   * @param msec The number of milliseconds to time.
   * @param obj The callback object.
   * @param arg1 The first callback argument.
   * @param arg2 The second callback argument.
   * @return Completion status (MCL_OK or MCL_ERROR).
   */
  static mcl_error_status set_timer(UINT32 msec, mcl_timer_handler* obj,
		  		INT32 arg1, INT32 arg2);
 
  /**
   * Cancels all timers that are currently set for the specified callback
   * object.
   * @param obj The callback object.
   */
  static void cancel_timer(mcl_timer_handler* obj);
 
  /**
   * Cancels all timers that are currently set for the specified callback
   * object and first argument.
   * @param obj The callback object.
   * @param arg1 The first callback argument.
   */
  static void cancel_timer(mcl_timer_handler* obj, INT32 arg1);

#if 0
   /**
   * Cancels all timers that are currently set for the specified callback
   * object and arguments.
   * @param obj The callback object.
   * @param arg1 The first callback argument.
   * @param arg2 The second callback argument.
   */
  static void cancel_timer(mcl_timer_handler* obj, INT32 arg1, INT32 arg2);
#endif

  /**
   * Cancels all timers that are currently set for the specified callback
   * object and arguments and return a status.
   * @param obj The callback object.
   * @param arg1 The first callback argument.
   * @param arg2 The second callback argument.
   * @return	completion status: MCL_OK if at least one instance of
   * 		the timer has been found and cancelled, MCL_ERROR otherwise
   */
  static mcl_error_status	cancel_timer(mcl_timer_handler* obj,
		  			     INT32 arg1, INT32 arg2);
 
  /**
   * Naps for at least the specified number of milliseconds, usually a bit
   * more.  Callbacks will not be blocked while performing the nap.
   * @param msec The number of milliseconds to nap for.
   */
  static void nap(UINT32 msec);
 
  /**
   * Yields the processor to any other thread waiting.
   */
  static void yield_processor();
 
  /**
   * This method should be called before the application exits.  This method
   * will kill the timer thread, disabling all timers at the current time. 
   * Deleting all RMF objects and calling this method should allow the
   * application to exit gracefully.  Use this call with care.
   */
  static void kill_thread();
 

private:
 
  /**
   * Allocates the timer arrays and starts the timer thread.
   */
  static void start();
  
  /**
   * The timer thread run method for generating callbacks.
   * @param arg The mcl_Timer object address cast as a void*.
   */
  static void *run(void *arg);
  
  /**
   * Convenience method for logging output.
   * @param mn Method name.
   * @param s Message string.
   */
  static void log(char *mn, char *s);
  
  /**
   * The initialization flag.
   */
  static bool  initialized;
  
  /**
   * The timer array size.
   */
  static INT32  array_size;
  
  /**
   * The timer array scan size.
   */
  static INT32  scan_size;
  
  /**
   * The in use flag array.
   */
  static bool	*in_use;
  
  /**
   * The expiration time array.
   */
  static struct timeval	*exp_time;
  
  /**
   * The event handler callback object array.
   */
  static mcl_timer_handler	**object;
  
  /**
   * The event handler callback argument1 array.
   */
  static INT32	*argument1;
  
  /**
   * The event handler callback argument2 array.
   */
  static INT32	*argument2;
  
  /**
   * The timer thread attributes.
   */
  static mcl_THREAD_ATTR_TYPE  timer_attr;
  
  /**
   * The timer thread for callbacks.
   */
  static mcl_THREAD_TYPE  timer_thread;
  
  /**
   * The timer mutex lock.
   */
  static mcl_mutex_t  timer_mutex;
  
  /**
   * A flag to record if the timer mutex lock has been initialized.
   */
  static bool  timer_mutex_init;
  
};


//----------------------------------------------------------------------------
// Inlines for all classes follow
//----------------------------------------------------------------------------

#endif // !MCL_TIMER_H
