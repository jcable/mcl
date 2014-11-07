/* $Id: mcl_node.h,v 1.2 2004/01/30 16:27:42 roca Exp $ */
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

#ifndef MCL_NODE_H
#define MCL_NODE_H

/** node stats, applicable to sources and receivers. */
typedef struct {
	UINT32	xxx;
} node_stats_t;

/** source specific stats. */
typedef struct {
	UINT32	xxx;
} source_stats_t;

/** receiver specific stats. */
typedef struct {
	UINT32	xxx;
} receiver_stats_t;


/**
 * Node class.
 */
class mcl_node {
public:
	mcl_node ();
	~mcl_node ();

	/**
	 * Retrieve the node id.
	 * @returns < 0 if error, node id > 0 if ok
	 */
	INT32	get_id (void);
	/**
	 * Set the node id.
	 * @param id new node id
	 * @return Completion status (MCL_OK or MCL_ERROR).
	 */
	mcl_error_status	set_id (const INT32 new_id);

	// link related functions
	mcl_node	*get_next (void);
	mcl_node	*get_prev (void);
	void		set_next (mcl_node *n);
	void		set_prev (mcl_node *p);

	// statistics
	node_stats_t	n_stats;	// node stats
	source_stats_t	s_stats;	// stats, may be 0 if not a source
	receiver_stats_t r_stats;	// stats, may be 0 if not a receiver

private:
	INT32		id;
	mcl_node	*prev, *next;	// for linked list in class group_mgmt
};


#if 0
/**
 * Member node class.
 * This is a derived class from the node class.
 */
class mcl_member : public mcl_node {
	mcl_member (void);
	~mcl_member (void);
};
#endif

//------------------------------------------------------------------------------
// Inlines for all classes follow
//------------------------------------------------------------------------------

inline INT32
mcl_node::get_id (void)
{
	return this->id;
}


inline mcl_node*
mcl_node::get_next (void)
{
	return this->next;
}

inline mcl_node*
mcl_node::get_prev (void)
{
	return this->prev;
}

inline void
mcl_node::set_next (mcl_node	*n)
{
	this->next = n;
}

inline void
mcl_node::set_prev (mcl_node	*p)
{
	this->prev = p;
}

#endif // MCL_NODE_H
