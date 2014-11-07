/* $Id: mcl.h,v 1.5 2004/06/15 15:53:27 roca Exp $ */
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

#ifndef MCL_H
#define MCL_H

/****** general purpose enumerations ******************************************/

/**
 * Used to differentiate between sending and receiving variants when needed.
 */
enum mcl_tx_or_rx {
	MCL_TX,
	MCL_RX
};


/****** prototypes ************************************************************/

extern void	mcl_moreabout (void);
extern void	mcl_exit (int n);
extern void	mcl_global_init (void);
extern void	mcl_stdout_stderr_init (void);


/****** global variables shared by all sessions *******************************/

/* MCL version of standard streams */
extern FILE	*mcl_stdout;
extern FILE	*mcl_stderr;

extern const UINT32	mcl_iss;
extern UINT32		mcl_time_count;

extern mcl_thread_t	mcl_timer_thread_id;

extern char mcl_tmp_dir_name[MAX_FILE_NAME_LEN];

extern class mcl_periodic_timer	mcl_periodic_timer_obj;


#endif // MCL_H
