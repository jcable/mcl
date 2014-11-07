/* $Id: mcl_node.cpp,v 1.2 2004/01/30 16:27:42 roca Exp $ */
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


mcl_node::mcl_node ()
{
	/* reset all stats for this node and private attributes */
	memset(this, 0, sizeof(*this));
}


mcl_node::~mcl_node ()
{
}


mcl_error_status
mcl_node::set_id (const INT32	new_id)
{
#ifdef DEBUG
	// TODO: cross all the list to check that no other node has the
	// same id...
#endif
	this->id = new_id;
	return MCL_OK;
}

