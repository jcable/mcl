/* $Id: mcl_timer.cpp,v 1.2 2004/01/30 16:27:43 roca Exp $ */
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
//----------------------------------------------------------------------------

#include "mcl_includes.h"


//
// Static class members.
//

bool		mcl_timer::initialized	= false;
INT32		mcl_timer::array_size	= 0;
INT32		mcl_timer::scan_size	= 0;
bool*		mcl_timer::in_use	= NULL;
struct timeval*	mcl_timer::exp_time	= NULL;
mcl_timer_handler**	mcl_timer::object	= NULL;
INT32*		mcl_timer::argument1	= NULL;
INT32*		mcl_timer::argument2	= NULL;
mcl_THREAD_ATTR_TYPE	mcl_timer::timer_attr;
mcl_THREAD_TYPE	mcl_timer::timer_thread;
mcl_MUTEX_TYPE	mcl_timer::timer_mutex	= mcl_MUTEX_CONST_INIT;
bool		mcl_timer::timer_mutex_init	= false;


//============================================================================
mcl_error_status
mcl_timer::set_timer(UINT32 msec, mcl_timer_handler* obj, INT32 arg1,
                               INT32 arg2)
{
  char*           mn = "set_timer";
  struct timeval  tv;
  INT32           i;
  
  //
  // Synchronize the threads.
  //
  
  if (!mcl_timer::timer_mutex_init)
  {
    mcl_MUTEX_STATIC_INIT(&mcl_timer::timer_mutex);
    mcl_timer::timer_mutex_init = true;
  }
  
  mcl_MUTEX_LOCK(&mcl_timer::timer_mutex);
  
  //
  // If the timer class has not been initialized yet, call start().
  //
  
  if (!mcl_timer::initialized)
  {
    mcl_timer::start();
    
    if (!mcl_timer::initialized)
    {
      log(mn, "Timer thread start failure.");
      mcl_MUTEX_UNLOCK(&mcl_timer::timer_mutex);
      return(MCL_ERROR);
    }
  }
  
  //
  // Add the timer information to the first open array element.
  //
  
  for (i = 0; i < mcl_timer::array_size; i++)
  {
    if (!mcl_timer::in_use[i])
    {
      
#ifdef GETTIMEOFDAY_NULL_ARG
      if (gettimeofday(&tv, NULL) != 0)
#else
      if (gettimeofday(&tv) != 0)
#endif
      {
        log(mn, "Unable to get current time.");
        mcl_MUTEX_UNLOCK(&mcl_timer::timer_mutex);
        return(MCL_ERROR);
      }
      
      tv.tv_sec  += (msec / 1000);
      tv.tv_usec += ((msec % 1000) * 1000);
      
      tv.tv_sec  += (tv.tv_usec / 1000000);
      tv.tv_usec  = (tv.tv_usec % 1000000);
      
      mcl_timer::in_use[i]     = true;
      mcl_timer::exp_time[i]   = tv;
      mcl_timer::object[i]    = obj;
      mcl_timer::argument1[i] = arg1;
      mcl_timer::argument2[i] = arg2;
      
      //
      // Update the scan size.
      //
      
      if (i >= mcl_timer::scan_size)
      {
        mcl_timer::scan_size = (i + 1);
      }
      
      mcl_MUTEX_UNLOCK(&mcl_timer::timer_mutex);
      return(MCL_OK);
    }
  }
  
  log(mn, "No room in timer array.");
  mcl_MUTEX_UNLOCK(&mcl_timer::timer_mutex);
  return(MCL_ERROR);
}

//============================================================================
void mcl_timer::cancel_timer(mcl_timer_handler* obj)
{
  char*  mn = "cancel_timer";
  INT32  i;
  
  //
  // Synchronize the threads.
  //
  
  if (!mcl_timer::timer_mutex_init)
  {
    mcl_MUTEX_STATIC_INIT(&mcl_timer::timer_mutex);
    mcl_timer::timer_mutex_init = true;
  }
  
  mcl_MUTEX_LOCK(&mcl_timer::timer_mutex);
  
  //
  // If the timer class has not been initialized yet, call start().
  //
  
  if (!mcl_timer::initialized)
  {
    mcl_timer::start();
    
    if (!mcl_timer::initialized)
    {
      log(mn, "Timer thread start failure.");
      mcl_MUTEX_UNLOCK(&mcl_timer::timer_mutex);
      return;
    }
  }
  
  //
  // Find all of the timer entries for the callback object and delete them.
  //
  
  for (i = 0; i < mcl_timer::array_size; i++)
  {
    if (mcl_timer::in_use[i])
    {
      if (mcl_timer::object[i] == obj)
      {
        mcl_timer::in_use[i] = false;
      }
    }
  }
  
  //
  // Update the scan size.
  //
  
  for (i = (mcl_timer::array_size - 1); i >= 0; i--)
  {
    if (mcl_timer::in_use[i])
    {
      mcl_timer::scan_size = (i + 1);
      mcl_MUTEX_UNLOCK(&mcl_timer::timer_mutex);
      return;
    }
  }
  
  mcl_timer::scan_size = 0;
  mcl_MUTEX_UNLOCK(&mcl_timer::timer_mutex);
}

//============================================================================
void mcl_timer::cancel_timer(mcl_timer_handler* obj, INT32 arg1)
{
  char*  mn = "cancel_timer";
  INT32  i;
  
  //
  // Synchronize the threads.
  //
  
  if (!mcl_timer::timer_mutex_init)
  {
    mcl_MUTEX_STATIC_INIT(&mcl_timer::timer_mutex);
    mcl_timer::timer_mutex_init = true;
  }
  
  mcl_MUTEX_LOCK(&mcl_timer::timer_mutex);
  
  //
  // If the timer class has not been initialized yet, call start().
  //
  
  if (!mcl_timer::initialized)
  {
    mcl_timer::start();
    
    if (!mcl_timer::initialized)
    {
      log(mn, "Timer thread start failure.");
      mcl_MUTEX_UNLOCK(&mcl_timer::timer_mutex);
      return;
    }
  }
  
  //
  // Find all of the timer entries for the callback object and argument and
  // delete them.
  //
  
  for (i = 0; i < mcl_timer::array_size; i++)
  {
    if (mcl_timer::in_use[i])
    {
      if ((mcl_timer::object[i] == obj) &&
          (mcl_timer::argument1[i] == arg1))
      {
        mcl_timer::in_use[i] = false;
      }
    }
  }
  
  //
  // Update the scan size.
  //
  
  for (i = (mcl_timer::array_size - 1); i >= 0; i--)
  {
    if (mcl_timer::in_use[i])
    {
      mcl_timer::scan_size = (i + 1);
      mcl_MUTEX_UNLOCK(&mcl_timer::timer_mutex);
      return;
    }
  }
  
  mcl_timer::scan_size = 0;
  mcl_MUTEX_UNLOCK(&mcl_timer::timer_mutex);
}

#if 0
//============================================================================
void mcl_timer::cancel_timer(mcl_timer_handler* obj, INT32 arg1, INT32 arg2)
{
  char*  mn = "cancel_timer";
  INT32  i;
  
  //
  // Synchronize the threads.
  //
  
  if (!mcl_timer::timer_mutex_init)
  {
    mcl_MUTEX_STATIC_INIT(&mcl_timer::timer_mutex);
    mcl_timer::timer_mutex_init = true;
  }
  
  mcl_MUTEX_LOCK(&mcl_timer::timer_mutex);
  
  //
  // If the timer class has not been initialized yet, call start().
  //
  
  if (!mcl_timer::initialized)
  {
    mcl_timer::start();
    
    if (!mcl_timer::initialized)
    {
      log(mn, "Timer thread start failure.");
      mcl_MUTEX_UNLOCK(&mcl_timer::timer_mutex);
      return;
    }
  }
  
  //
  // Find all of the timer entries for the callback object and argument and
  // delete them.
  //
  
  for (i = 0; i < mcl_timer::array_size; i++)
  {
    if (mcl_timer::in_use[i])
    {
      if ((mcl_timer::object[i] == obj) &&
          (mcl_timer::argument1[i] == arg1) &&
          (mcl_timer::argument2[i] == arg2))
      {
        mcl_timer::in_use[i] = false;
      }
    }
  }
  
  //
  // Update the scan size.
  //
  
  for (i = (mcl_timer::array_size - 1); i >= 0; i--)
  {
    if (mcl_timer::in_use[i])
    {
      mcl_timer::scan_size = (i + 1);
      mcl_MUTEX_UNLOCK(&mcl_timer::timer_mutex);
      return;
    }
  }
  
  mcl_timer::scan_size = 0;
  mcl_MUTEX_UNLOCK(&mcl_timer::timer_mutex);
}
#endif

//============================================================================
mcl_error_status
mcl_timer::cancel_timer(mcl_timer_handler* obj, INT32 arg1, INT32 arg2)
{
  char*  mn = "cancel_timer";
  INT32  i;
  bool	found = false;	// indicates if at least one instance has been found
  
  //
  // Synchronize the threads.
  //
  
  if (!mcl_timer::timer_mutex_init)
  {
    mcl_MUTEX_STATIC_INIT(&mcl_timer::timer_mutex);
    mcl_timer::timer_mutex_init = true;
  }
  
  mcl_MUTEX_LOCK(&mcl_timer::timer_mutex);
  
  //
  // If the timer class has not been initialized yet, call start().
  //
  
  if (!mcl_timer::initialized)
  {
    mcl_timer::start();
    
    if (!mcl_timer::initialized)
    {
      log(mn, "Timer thread start failure.");
      mcl_MUTEX_UNLOCK(&mcl_timer::timer_mutex);
      return MCL_ERROR;
    }
  }
  
  //
  // Find all of the timer entries for the callback object and argument and
  // delete them.
  //
  
  for (i = 0; i < mcl_timer::array_size; i++)
  {
    if (mcl_timer::in_use[i])
    {
      if ((mcl_timer::object[i] == obj) &&
          (mcl_timer::argument1[i] == arg1) &&
          (mcl_timer::argument2[i] == arg2))
      {
        mcl_timer::in_use[i] = false;
	found = true;
      }
    }
  }
  
  //
  // Update the scan size.
  //
  
  for (i = (mcl_timer::array_size - 1); i >= 0; i--)
  {
    if (mcl_timer::in_use[i])
    {
      mcl_timer::scan_size = (i + 1);
      mcl_MUTEX_UNLOCK(&mcl_timer::timer_mutex);
      return (found ? MCL_OK : MCL_ERROR);
    }
  }
  
  mcl_timer::scan_size = 0;
  mcl_MUTEX_UNLOCK(&mcl_timer::timer_mutex);
  return (found ? MCL_OK : MCL_ERROR);
}

//============================================================================
void mcl_timer::nap(UINT32 msec)
{
  struct timeval  tv;
  
  //
  // If the nap time is less than one millisecond, return.
  //
  
  if (msec < 1)
  {
    return;
  }
  
  //
  // Use select() as a fairly portable way to sleep with subsecond precision.
  //
  tv.tv_sec  = (msec / 1000);
  tv.tv_usec = ((msec % 1000) * 1000);
  
  select(0, NULL, NULL, NULL, &tv);
}

//============================================================================
void mcl_timer::yield_processor()
{
  
  //
  // Yield the CPU.
  //
  
  mcl_THREAD_YIELD();
}

//============================================================================
void mcl_timer::kill_thread()
{
  INT32  i;
  
  //
  // If a timer thread is running, cancel it and clear out the timer arrays.
  //
  
  if (mcl_timer::initialized)
  {
    
    //
    // Synchronize the threads.
    //
    
    if (!mcl_timer::timer_mutex_init)
    {
      mcl_MUTEX_STATIC_INIT(&mcl_timer::timer_mutex);
      mcl_timer::timer_mutex_init = true;
    }
    
    mcl_MUTEX_LOCK(&mcl_timer::timer_mutex);
    
    for (i = 0; i < MCL_TIMER_ARRAY_SIZE; i++)
    {
      mcl_timer::in_use[i]           = false;
      mcl_timer::exp_time[i].tv_sec  = 0;
      mcl_timer::exp_time[i].tv_usec = 0;
      mcl_timer::object[i]          = NULL;
      mcl_timer::argument1[i]       = 0;
      mcl_timer::argument2[i]       = 0;
    }
    
    mcl_THREAD_KILL(mcl_timer::timer_thread);
    
    mcl_timer::initialized = false;
    
    mcl_MUTEX_UNLOCK(&mcl_timer::timer_mutex);
  }
  
  mcl_THREAD_YIELD();
}

//============================================================================
void mcl_timer::start()
{
  char*  mn = "start";
  INT32  i;
  
  //
  // Allocate the arrays.
  //
  
  if (mcl_timer::in_use == NULL)
  {
    mcl_timer::in_use = new bool[MCL_TIMER_ARRAY_SIZE];
    
    if (mcl_timer::in_use == NULL)
    {
      log(mn, "Unable to allocate in_use array.");
      return;
    }
  }
  
  if (mcl_timer::exp_time == NULL)
  {
    mcl_timer::exp_time = new struct timeval[MCL_TIMER_ARRAY_SIZE];
    
    if (mcl_timer::exp_time == NULL)
    {
      log(mn, "Unable to allocate expiration time array.");
      return;
    }
  }
  
  if (mcl_timer::object == NULL)
  {
    mcl_timer::object = new mcl_timer_handler*[MCL_TIMER_ARRAY_SIZE];
    
    if (mcl_timer::object == NULL)
    {
      log(mn, "Unable to allocate object array.");
      return;
    }
  }
  
  if (mcl_timer::argument1 == NULL)
  {
    mcl_timer::argument1 = new INT32[MCL_TIMER_ARRAY_SIZE];
    
    if (mcl_timer::argument1 == NULL)
    {
      log(mn, "Unable to allocate fist argument array.");
      return;
    }
  }
  
  if (mcl_timer::argument2 == NULL)
  {
    mcl_timer::argument2 = new INT32[MCL_TIMER_ARRAY_SIZE];
    
    if (mcl_timer::argument2 == NULL)
    {
      log(mn, "Unable to allocate second argument array.");
      return;
    }
  }
  
  //
  // Initialize the sizes and array elements.
  //
  
  mcl_timer::array_size = MCL_TIMER_ARRAY_SIZE;
  mcl_timer::scan_size  = 0;
  
  for (i = 0; i < MCL_TIMER_ARRAY_SIZE; i++)
  {
    mcl_timer::in_use[i]           = false;
    mcl_timer::exp_time[i].tv_sec  = 0;
    mcl_timer::exp_time[i].tv_usec = 0;
    mcl_timer::object[i]          = NULL;
    mcl_timer::argument1[i]       = 0;
    mcl_timer::argument2[i]       = 0;
  }
  
  //
  // Create the timer thread.  Make it a detached thread.
  //
  
  if (mcl_THREAD_ATTR_INIT(&mcl_timer::timer_attr) != 0)
  {
    log(mn, "Thread attribute initialization error.");
    return;
  }
  
  if (mcl_THREAD_ATTR_SETDETACHSTATE(&mcl_timer::timer_attr,
                                     mcl_CONST_CREATE_DETACHED) != 0)
  {
    log(mn, "Thread set attribute detached state error.");
    return;
  }
  
  if (mcl_THREAD_CREATE(&mcl_timer::timer_thread, &mcl_timer::timer_attr,
                        mcl_timer::run, NULL) != 0)
  {
    log(mn, "Thread creation error.");
    return;
  }
  
  if (mcl_THREAD_ATTR_DESTROY(&mcl_timer::timer_attr) != 0)
  {
    log(mn, "Thread attribute destruction error.");
    return;
  }
  
  //
  // Mark the initialization as complete.
  //
  
  mcl_timer::initialized = true;
}

//============================================================================
mcl_VOID_METHOD mcl_timer::run(mcl_THREAD_ARG arg)
{
  struct timeval     tv;
  bool           forever;
  bool           retry;
  mcl_timer_handler* localObject;
  INT32              lastState;
  INT32              lastType;
  INT32              localArgument1;
  INT32              localArgument2;
  INT32              i;
  
  //
  // Set deferred cancelation for this thread.
  //
  
  mcl_THREAD_CANCEL_STATE(mcl_CONST_CANCEL_ENABLE, &lastState);
  mcl_THREAD_CANCEL_TYPE(mcl_CONST_CANCEL_DEFERRED, &lastType);
  
  //
  // Block the SIGINT signal in this thread.
  //
  
  mcl_SIGNAL_BLOCK_MASK(SIGINT);
  
  //
  // Loop forever.
  //
  
  forever = true;
  
  while (forever)
  {
    mcl_timer::yield_processor();
    
    //
    // Sleep for some amount of time using either sginap() or select().
    //
    
    mcl_THREAD_TEST_CANCEL();
    
#ifdef HAVE_SGINAP
    sginap((50 * CLK_TCK) / 1000);
#else
    tv.tv_sec  = 0;
    tv.tv_usec = 50000;
    
    select(0, NULL, NULL, NULL, &tv);
#endif
    
    mcl_THREAD_TEST_CANCEL();
    
    //
    // Get the current time and perform any callbacks that are due.  Be sure
    // to aquire the mutex lock whenever accessing the static members.
    // However, the mutex lock cannot be held when actually performing any
    // callbacks.  Also, the loop that checks for expired timers must be
    // restarted after each callback since a callback might set or cancel
    // timers.
    //
    
#ifdef GETTIMEOFDAY_NULL_ARG
    if (gettimeofday(&tv, NULL) == 0)
#else
    if (gettimeofday(&tv) == 0)
#endif
    {
      mcl_MUTEX_LOCK(&mcl_timer::timer_mutex);
      
      retry = true;
      
      while (retry)
      {
        retry = false;
        
        for (i = 0; i < mcl_timer::scan_size; i++)
        {
          if (mcl_timer::in_use[i])
          {
            if ((tv.tv_sec > mcl_timer::exp_time[i].tv_sec) ||
                ((tv.tv_sec == mcl_timer::exp_time[i].tv_sec) &&
                 (tv.tv_usec >= mcl_timer::exp_time[i].tv_usec)))
            {
              if (mcl_timer::object[i] != NULL)
              {
                localObject    = mcl_timer::object[i];
                localArgument1 = mcl_timer::argument1[i];
                localArgument2 = mcl_timer::argument2[i];
                
                mcl_timer::in_use[i] = false;
                
                mcl_MUTEX_UNLOCK(&mcl_timer::timer_mutex);
                localObject->timer_callback(localArgument1, localArgument2);
                mcl_MUTEX_LOCK(&mcl_timer::timer_mutex);
                
                retry = true;
                break;
              }
            }
          }
        }
      }
      
      mcl_MUTEX_UNLOCK(&mcl_timer::timer_mutex);
    }
  }
  
  return(NULL);
}

//============================================================================
void mcl_timer::log(char* mn, char* s)
{
  cerr << "[mcl_timer::" << mn << "] " << s << endl;
}
