/* $Id: mcl_alc_hdr.h,v 1.4 2005/01/11 13:12:26 roca Exp $ */
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


#ifndef MCL_ALC_HDR_H  /* { */
#define MCL_ALC_HDR_H


#define ALC_VERSION	0


/**
 * Initializes the ALC header.
 * Assumes that the memory block has been allocated by the caller
 * who MUST ensure that it will be large enough to hold all the
 * possible fields!
 * @param mclcb
 * @param lct_hdr	pointer to the LCT header
 * @param hdr_infos	pointer to the control struct containing raw
 * 			used during ALC header creation
 * @return		returns the size of the ALC header (bytes), or
 *			-1 if error.
 */

extern int	alc_hdr_create	(class mcl_cb		*mclcb,
				 fixed_lct_hdr_t	*lct_hdr,
				 hdr_infos_t		*hdr_infos);


/**
 * Parses the ALC header of the received packet.
 * @param mclcb
 * @param lct_hdr	pointer to the LCT header
 * @param hdr_infos	pointer to the control struct containing raw
 * 			used during ALC header creation
 * @return		returns the size of the ALC header (bytes), or
 *			-1 if error.
 */
extern int	alc_hdr_parse	(class mcl_cb		*mclcb,
				 fixed_lct_hdr_t	*lct_hdr,
				 hdr_infos_t		*hdr_infos,
				 INT32			plen);


#endif /* }  MCL_ALC_HDR_HALC */
