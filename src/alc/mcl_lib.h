/* $Id: mcl_lib.h,v 1.2 2005/01/11 15:40:16 moi Exp $ */
/*
 *  Copyright (c) 1999-2004 INRIA - All rights reserved
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
 * Private versions of some public functions. 
 * Differ from the fact that they can be called while being already locked.
 */

/**
 * Internal MCLv3 control function.
 * Assumes the caller is already locked.
 * Uses the same parameters and return value as mcl_ctl().
 */
extern int	mcl_ctl2	(class mcl_cb		*const mclcb,
				 INT32			optname,
				 void			*optvalue,
				 INT32			optlen);

#if 0
/**
 * Internal MCLv3 sendto function.
 * Assumes the caller is already locked.
 * Uses the same parameters and return value as mcl_sendto().
 */
extern int	mcl_sendto2 	(class mcl_cb		*const mclcb,
				 const void		*data, 
				 int			len,
				 const struct sockaddr	*saddr,
				 int			saddr_len);
#endif

/**
 * Internal MCLv3 sendmsg function.
 * Assumes the caller is already locked.
 * Uses the same parameters and return value as mcl_sendmsg().
 */
extern INT64	mcl_sendmsg2	(class mcl_cb		*const mclcb,
				 struct mcl_msghdr	*msg,
				 mcl_msgflag		flags);


