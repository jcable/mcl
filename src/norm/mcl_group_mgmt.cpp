/* $Id: mcl_group_mgmt.cpp,v 1.2 2004/01/30 16:27:42 roca Exp $ */
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
 * group management class
 */

#include "mcl_includes.h"


mcl_group_mgmt::mcl_group_mgmt ()
{
	head = tail = NULL;
	cur_member = NULL;
	memset(member_tab, 0, sizeof(member_tab));
	nb_of_members = 0;
}


mcl_group_mgmt::~mcl_group_mgmt ()
{
	mcl_node	*m;

	set_member_iter_to_head();
	while ((m = get_next_member()) != NULL) {
		remove_member(m);
		delete m;
	}
	head = tail = NULL;
	cur_member = NULL;
	memset(member_tab, 0, sizeof(member_tab));
	nb_of_members = 0;
}


// add a member to list of members known by MCL-NORM
mcl_error_status
mcl_group_mgmt::add_member (mcl_node *const mem)
{
	return MCL_OK;
}


// remove a member to list of members known by MCL-NORM
mcl_error_status
mcl_group_mgmt::remove_member (mcl_node *const mem)
{
	mcl_node	*m;

	if (!(m = find_member_by_id(mem->get_id()))) {
		// error, not found!
		return MCL_ERROR;
	}
	return MCL_OK;

}


mcl_node *
mcl_group_mgmt::find_member_by_id (int id)
{
	return NULL;
}


INT32
mcl_group_mgmt::get_member_stats (mcl_node *const mem)
{
	return 0;
}


mcl_node *
mcl_group_mgmt::get_worst_member (void)
{
	return NULL;
}

