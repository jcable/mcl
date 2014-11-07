/* $Id: mcl_itime.h,v 1.2 2004/01/30 16:27:42 roca Exp $ */
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
 * Internal MCL time representation is in milli-seconds (ms)
 * Being stored in a signed 32 bit int, it will cycle after 596h31
 */
typedef INT32	mcl_itime_t;


/* define for optimized time management functions */
#define ITIME_FUNCTION_DEFINES

#ifdef ITIME_FUNCTION_DEFINES

#define mcl_usec2it(from)	((INT32)((from) / 1000))
#define	mcl_it2usec(from)	((from) * 1000)
#define	mcl_it2sec(from)	((float)(from) * 0.001)
#define	mcl_itime_sub(val,decr)	((val) - (decr))
#define	mcl_itime_add(val,incr)	((val) + (incr));
#define mcl_tv2it(tv)		((tv).tv_sec * 1000 + (INT32)((float)(tv).tv_usec * 0.001))

#else

extern mcl_itime_t	mcl_usec2it	(int from);
extern int		mcl_it2usec	(mcl_itime_t from);
extern float		mcl_it2sec	(mcl_itime_t from);
extern mcl_itime_t	mcl_itime_sub	(mcl_itime_t val, mcl_itime_t decr);
extern mcl_itime_t	mcl_itime_add	(mcl_itime_t val, mcl_itime_t incr);

#endif /* !ITIME_FUNCTION_DEFINES */

extern mcl_itime_t	mcl_get_itime	(void);
extern struct timeval	mcl_get_tvtime	(void);

extern struct timeval	mcl_norm_tvtime(struct timeval val);

