/* $Id: mcl_rx.h,v 1.1 2005/05/23 14:35:54 roca Exp $ */
/*
 *  Copyright (c) 2005 INRIA - All rights reserved
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

#ifndef MCL_RX_H
#define MCL_RX_H


/**
 * This Class controls many aspects related to transmissions.
 */
class mcl_rx {
 
public:
	/****** Public Members ************************************************/
	/**
	 * Default constructor.
	 */
	mcl_rx ();

	/**
	 * Default destructor.
	 */
	~mcl_rx ();


	/****** Public Attributes *********************************************/

#ifdef SSM
	/**
	 * True if we are using SSM multicast in this session.
	 * In that case, src_addr is mandatory...
	 */
	bool	ssm;
#endif

	/**
	 * Unicast address of the source.
	 * Used by the TSI/saddr check of incoming pkts.
	 * This address is also used in SSM mode during SSM multicast join.
	 * Port is not specified.
	 */
	class mcl_addr	src_addr;

	/* true if TSI/saddr is done and src_addr set */
	bool		check_src_addr;


	/* required by MCL_WAIT_EVENT_CLOSED */
	bool never_leave_base_layer;

private:
	/****** Private Members ***********************************************/

	/****** Private Attributes ********************************************/
};


//------------------------------------------------------------------------------
// Inlines for all classes follow
//------------------------------------------------------------------------------

#endif // !MCL_RX_H

