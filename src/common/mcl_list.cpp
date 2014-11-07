/* $Id: mcl_list.cpp,v 1.3 2004/01/30 16:27:42 roca Exp $ */
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

#ifdef ALC
#include "../alc/mcl_includes.h"

#elif defined(NORM)

#include "../norm/mcl_includes.h"
#endif // RM_PROTOCOL


#if 0
/**
 * Default constructor.
 */
template <class T>
mcl_seq_list<T>::mcl_seq_list ()
{
	this->m_head = NULL;
	this->m_size = 0;
	this->m_find_cache = NULL;
}


/**
 * Default destructor.
 */
template <class T>
mcl_seq_list<T>::~mcl_seq_list ()
{
	// remove everything and call delete for each element
	this->remove_all(this->m_delete_callback);
}


/**
 * Insert an element in an ordered list according to its sequence
 * number.
 * See header file for more information.
 */
template <class T>
mcl_error_status
mcl_seq_list<T>::insert (//mcl_cb		*const mclcb,
		     T			*elem)
{
	T	*pe, *ne;	// insert elem between prev/next elem

	ASSERT(elem->next == NULL && elem->prev == NULL); // single elem to add
	TRACELVL(5, (mcl_stdout, "-> mcl_seq_list::insert: du=x%x\n", (int)this))
	/*
	 * find last element in list, i.e. with highest seq nb
	 */
	if (this->m_head == NULL || (this->m_head)->prev == NULL) {
		/* first element in empty list */
		this->m_head = elem;
		this->m_size = 1;
		elem->next = elem->prev = elem;
		TRACELVL(5, (mcl_stdout, "<- mcl_seq_list::insert: first\n"))
		return MCL_OK;
	}
	pe = this->m_head->prev;
	ne = pe->next;
	/*
	 * find the proper location of elem in the list to make
	 * sure seq numbers are always increasing
	 */
	while (elem->seq < pe->seq) {
		ne = pe;
		pe = pe->prev;
		if (ne == this->m_head) {
			/* we have cycled ! elem is the new list head */
			this->m_head = elem;
			break;
		}
	}
	if (elem->seq == pe->seq) {
		/* elem already received */
		TRACELVL(5, (mcl_stdout, "<- mcl_seq_list::insert: ERROR, already in list\n"))
		return MCL_ERROR;
	} else {
		/* elem DU */
		pe->next = elem;
		elem->prev = pe;
		elem->next = ne;
		ne->prev = elem;
		this->m_size++;
		TRACELVL(5, (mcl_stdout, "<- mcl_seq_list::insert: new\n"))
		return MCL_OK;
	}
}


/**
 *
 * See header file for more information.
 */
template <class T>
mcl_error_status
mcl_seq_list<T>::remove (T	*elem)
{
	TRACELVL(5, (mcl_stdout, "-> mcl_seq_list::remove: elem->seq=%d\n", elem->seq))
	if (this->find(elem->seq) == NULL) {
		TRACELVL(5, (mcl_stdout, "<- mcl_seq_list::remove: ERROR, not found in list\n"))
		return MCL_ERROR;
	}
	ASSERT(elem->prev != NULL && elem->next != NULL);
	elem->prev->next = elem->next;
	elem->next->prev = elem->prev;
	if (this->m_head == elem) {
		// elem->next since seq is kept in increasing order
		this->m_head = elem->next;
	}
	this->m_find_cache = NULL; // invalidate cache which always points
				   // to elem (find() is called just above)
	this->m_size--;
	return MCL_OK;
}


/**
 *
 * See header file for more information.
 */
template <class T>
mcl_error_status
mcl_seq_list<T>::remove_all (void (*func) (T*))
{
	T	*e, *ne;	// current elem and next elem

	TRACELVL(5, (mcl_stdout, "-> mcl_seq_list::remove_all\n"))
	e = this->m_head;
	if (e == NULL) { 
		ASSERT(this->m_size == 0);
		TRACELVL(5, (mcl_stdout, "<- mcl_seq_list::remove_all: empty list\n"))
		return MCL_OK;
	}
	do {
		ne = e->next;
		if (func) {
			func(e);
		}
		e = ne;
#ifdef DEBUG
		this->m_size--;
#endif
	} while (e != this->m_head);
	ASSERT(this->m_size == 0);
	this->m_size = 0;
	this->m_head = NULL;
	this->m_find_cache = NULL;
	TRACELVL(5, (mcl_stdout, "<- mcl_seq_list::remove_all: ok\n"))
	return MCL_OK;
}



/**
 *
 * See header file for more information.
 */
template <class T>
T *
mcl_seq_list<T>::find (INT32	s)
{
	T	*e;

	TRACELVL(5, (mcl_stdout, "-> mcl_seq_list::find: seq=%d\n", s))
	e = this->m_head;
	if (e == NULL) { 
		ASSERT(this->m_size == 0);
		TRACELVL(5, (mcl_stdout, "<- mcl_seq_list::find: empty list\n"))
		return NULL;
	}
	if (this->m_find_cache && this->m_find_cache->seq == s) {
		TRACELVL(5, (mcl_stdout, "<- mcl_seq_list::find: found in cache\n"))
		return this->m_find_cache;
	}
	/*
	 * start from the highest seq number
	 */
	e = e->prev;
	while (1) {
		ASSERT(e);
		if (e->seq == s) {
			/* found */
			this->m_find_cache = e;
			TRACELVL(5, (mcl_stdout, "<- mcl_seq_list::find: found\n"))
			return e;
		} 
		if (e == this->m_head || e->seq < s) {
			/* we have cycled or new element cannot be in list */
			TRACELVL(5, (mcl_stdout, "<- mcl_seq_list::find: new\n"))
			return NULL;
		}
		e = e->prev;
	}
}


/**
 *
 * See header file for more information.
 */
template <class T>
void
mcl_seq_list<T>::print_all (void)
{
	T	*e;	// current elem

	TRACELVL(5, (mcl_stdout, "-> mcl_seq_list::print_all\n"))
	e = this->m_head;
	if (e == NULL) { 
		ASSERT(this->m_size == 0);
		TRACELVL(5, (mcl_stdout, "<- mcl_seq_list::print_all: empty list\n"))
		return;
	}
	do {
		e->print();
		e = e->next;
	} while (e != this->m_head);
	TRACELVL(5, (mcl_stdout, "<- mcl_seq_list::print_all: ok\n"))
}
#endif // 0
