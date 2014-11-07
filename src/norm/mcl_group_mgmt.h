/* $Id: mcl_group_mgmt.h,v 1.2 2004/01/30 16:27:42 roca Exp $ */
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

#ifndef MCL_GROUP_MGMT_H
#define MCL_GROUP_MGMT_H

#include "mcl_node.h"


/**
 * Group management class.
 * Only considers the group members, i.e. the receivers of a multicast group.
 */
class mcl_group_mgmt {
public:
	mcl_group_mgmt ();
	~mcl_group_mgmt ();

	/****** Member (i.e. receiver) list management ************************/

	/**
	 * Add a member to list of members known by MCL-NORM.
	 * @param mem new member to add
	 * @return Completion status (MCL_OK or MCL_ERROR).
	 */
	mcl_error_status	add_member (mcl_node *const mem);

	/**
	 * Remove a member to list of members known by MCL-NORM.
	 * @param mem member to remove
	 * @return Completion status (MCL_OK or MCL_ERROR).
	 */
	mcl_error_status	remove_member (mcl_node *const mem);

	/*
	 * Iteration functions on the member list
	 */

	/** Return the current number of members in the list. */
	INT32	get_nb_of_members (void);

	/**
	 * Find the member with this id.
	 * This function also resets the current member pointer to this one.
	 * @returns a member if found, or NULL if error
	 */
	mcl_node	*find_member_by_id (int id);

	/**
	 * Reset the member list iteration to head.
	 * Next call to get_next_member will return the first node in list
	 * (if any), and following calls the node after.
	 */
	void		set_member_iter_to_head (void);

	/**
	 * Reset the member list iteration to tail.
	 * Next call to get_prev_member will return the last node in list
	 * (if any), and following calls the node before.
	 */
	void		set_member_iter_to_tail (void);

	/**
	 * Iteration, from current position.
	 * Be carefull not to mix get_next_member() and get_prev_member()
	 * in the same iteration (will not work!).
	 * returns a member, or NULL if nothing or at the end of list
	 */
	mcl_node	*get_next_member (void);

	/**
	 * Iteration, from current position.
	 * Be carefull not to mix get_next_member() and get_prev_member()
	 * in the same iteration (will not work!).
	 * returns a member, or NULL if nothing or at the end of list
	 */
	mcl_node	*get_prev_member (void);

	/****** Statistics ****************************************************/

	/**
	 * Get the statistics for a given member. 
	 */
	INT32		get_member_stats (mcl_node *const mem);

	/**
	 * Get the member having the worst statisitics (only one).
	 * @returns a member, or NULL if no member in list
	 */
	mcl_node	*get_worst_member (void);

private:
	/*
	 * iterations
	 */
	mcl_node	*head, *tail;	// doubly linked list of members, no
					// cycle: tail->next==head->prev==NULL
	mcl_node	*cur_member;	// current position in iteration.
					// this is the member that will returned
					// by get_next/prev_member()
	static const INT32 MAX_NB_OF_MEMBERS = 1024;
	mcl_node	*member_tab[MAX_NB_OF_MEMBERS];
					// table for direct id to member access
	INT32	nb_of_members;		// number of members in the list
};


//------------------------------------------------------------------------------
// Inlines for all classes follow
//------------------------------------------------------------------------------

inline	INT32
mcl_group_mgmt::get_nb_of_members (void)
{
	return this->nb_of_members;
}

inline void
mcl_group_mgmt::set_member_iter_to_head (void)
{
	this->cur_member = this->head;
}

inline void
mcl_group_mgmt::set_member_iter_to_tail (void)
{
	this->cur_member = this->tail;
}

inline mcl_node	*
mcl_group_mgmt::get_next_member (void)
{
	mcl_node	*n = this->cur_member;

	if (n) {
		this->cur_member = this->cur_member->get_next();
		return n;
	} else {
		return NULL;
	}
}

inline mcl_node	*
mcl_group_mgmt::get_prev_member (void)
{
	mcl_node	*n = this->cur_member;

	if (n) {
		this->cur_member = this->cur_member->get_prev();
		return n;
	} else {
		return NULL;
	}
}


#endif // MCL_GROUP_MGMT_H
