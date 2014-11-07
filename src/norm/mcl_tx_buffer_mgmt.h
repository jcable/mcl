/* $Id: mcl_tx_buffer_mgmt.h,v 1.2 2004/01/30 16:27:43 roca Exp $ */
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

#ifndef MCL_TX_BUFFER_MGMT_H
#define MCL_TX_BUFFER_MGMT_H

typedef struct {
	ulong	cur_buf_space;
	ulong	max_buf_space;
} tx_buffer_stats_t;


// main class
class tx_buffer_mgmt {
public:
	void tx_buffer_mgmt ();
	void ~tx_buffer_mgmt ();
	// returns < 0 if error, pointer to new buffer if ok
	char	*alloc (int size);
	// returns < 0 if error, 0 if ok
	int	free (char *buf);
	// statistics
	ulong	get_cur_buf_space_stats (void);
	ulong	get_max_buf_space_stats (void);

private:
	int	k;
	int	n;
	mcl_thread_t	fec_encoding_thread;	
	tx_buffer_stats_t	stats;
}

#endif // MCL_TX_BUFFER_MGMT_H
