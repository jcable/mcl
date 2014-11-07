/* $Id: mcl_itime.cpp,v 1.2 2004/01/30 16:27:42 roca Exp $ */
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


/****** mcl_itime functions ***************************************************/

/*
 * See mcl_time.h for internal MCL time representation
 */

#ifndef ITIME_FUNCTION_DEFINES

mcl_itime_t
mcl_usec2it (int	from)
{
	return((int)(from / 1000));
}


int
mcl_it2usec (mcl_itime_t	from)
{
	return(from * 1000);
}


float
mcl_it2sec (mcl_itime_t from)
{
	return((float)from * 0.001);
}


mcl_itime_t
mcl_itime_sub (mcl_itime_t	val,
		mcl_itime_t	decr)
{
	return(val - decr);
}


mcl_itime_t
mcl_itime_add (mcl_itime_t	val
		mcl_itime_t	incr)
{
	return(val + decr);
}


mcl_itime_t
mcl_tv2it (struct timeval	tv)
{
	return(tv.tv_sec * 1000 + (long)((float)tv.tv_usec * 0.001));
}

#endif /* !ITIME_FUNCTION_DEFINES */


/****** timeval functions *****************************************************/


/*
 * normalises a timeval struct
 * time in usec = sec * 1000000 + usec
 * normalised time is such that usec is in [0,999999]
 */
struct timeval
mcl_norm_tvtime(struct timeval val)
{	
	while (val.tv_usec < 0) {
		val.tv_usec += 1000000;
		val.tv_sec--;
	}
	while (val.tv_usec >= 1000000) {
		val.tv_usec -= 1000000;
		val.tv_sec++;
	}
	return val;
}


/****** get time functions ****************************************************/


static int time_initialized = 0;
static struct timeval	tv0;		/* initial timeval at initialization */


mcl_itime_t
mcl_get_itime ()
{
	return(mcl_tv2it(mcl_get_tvtime()));
}


struct timeval
mcl_get_tvtime ()
{
	struct timeval	tv;

	if (!time_initialized) {
		time_initialized = 1;
		mcl_gettimeofday(&tv0);
		/*it0 = mcl_tv2it(tv0);*/
	}
	mcl_gettimeofday(&tv);
	tv.tv_sec  -= tv0.tv_sec;
	tv.tv_usec -= tv0.tv_usec;
	return mcl_norm_tvtime(tv);
}

