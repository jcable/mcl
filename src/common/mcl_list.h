/* $Id: mcl_list.h,v 1.3 2004/01/30 16:27:42 roca Exp $ */
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

#ifndef MCL_LIST_H
#define MCL_LIST_H

#if 0 // {
/**
 * Ordered linked list class with sequential access to elements.
 *
 * Assumes that elements already have next/prev pointer members
 * AND a seq member value which will be used to sort elements.
 * A print() method is also required if it is desired to print the
 * list content with the print_all() method.
 *
 * Search operations follow a linear search algorithm, so this class
 * is limited to small lists.
 * This class does not allocate any data, hence its efficiency compared
 * to the variants based on containers.
 * Yet a limitation is that an element cannot belong simultaneously
 * to two or more lists since there is a single instance of prev/next
 * pointers.
 */
template <class T>
class mcl_seq_list
{

public:
	/**
	 * Default constructor.
	 */
	mcl_seq_list ();

	/**
	 * Default destructor.
	 * Remove and delete all elements of the list, calling the
	 * default destructor of these elements.
	 */
	~mcl_seq_list ();

	/**
	 * Insert an element in an ordered list according to its sequence
	 * number.
	 * Insert the element at its location (i.e. to comply
	 * with the increasing seq # property) in the list.
	 * @param mclcb
	 * @param elem	element to insert in list
	 * @return	completion status: MCL_OK if inserted, or MCL_ERROR
	 * 		if already present in list.
	 */
	mcl_error_status	insert	(T *elem);

	/**
	 * Remove an element from the list.
	 * This method does not try to free the element.
	 * @param mclcb
	 * @param elem  element to remove from the list
	 * @return      completion status: MCL_OK if removed, or MCL_ERROR
	 *              if not found in list or in case of error.
	 */
	mcl_error_status	remove	(T *elem);

	/**
	 * Remove all elements from the list.
	 * This method does not try to free the elements but instead
	 * calls the callback function provided for each element.
	 * @param mclcb
	 * @param func	callback function that needs to be called
	 *		for each element removed from the list, or NULL
	 *		if nothing shall be done
	 * @return      completion status: MCL_OK if removed, or MCL_ERROR
	 *              in case of error.
	 */
	mcl_error_status	remove_all (void (*func)(T*));

	/**
	 * Find the element with the given sequence number in an ordered list.
	 * @param mclcb
	 * @param seq	sequence number looked for
	 * @return	pointer to the element with that sequence number
	 *		if found, NULL if not found or in case of error.
	 */
	T			*find	(INT32 seq);

	/**
	 * Return the first element of list.
	 * @return	pointer to the first element of this sequentially
	 *		ordered list, or NULL if list is empty.
	 */
	T			*first (void);

	/**
	 * Return the last element of list.
	 * @return	pointer to the last element of this sequentially
	 *		ordered list, or NULL if list is empty.
	 */
	T			*last (void);

	/**
	 * Number of elements in list.
	 * @return	total number of elements in list
	 */
	INT32			size (void);
	
	/**
	 * Call the print method of each element in list.
	 * Traversal of list is done in increasing sequence number order.
	 */
	void			print_all (void);


private:
	T	*m_head;	// pointer to first element of list
	INT32	m_size;		// number of elements in list
	T	*m_find_cache;	// cache used by the find method

	static void m_delete_callback (T *elem);// used by default destructor
						// to delete each element
};


/**
 * Ordered table class.
 *
 * Assumes that elements already have a seq member value which is
 * also used as the index in the tablE.
 * A print() method is also required if it is desired to print the
 * list content with the print_all() method.
 *
 * This class does not allocate any data, hence its efficiency compared
 * to the variants based on containers.
 * Yet a limitation is that an element cannot belong simultaneously
 * to two or more lists since there is a single instance of prev/next
 * pointers.
 * Another limitation is that the table cannot be extended once
 * created.
 */
template <class T>
class mcl_seq_table
{

public:
	/**
	 * Constructor.
	 * The table has a fixed size, initialized upon creation.
	 * No size modification will be possible later on.
	 * @param size	number of elements in the table.
	 */
	mcl_seq_table (INT32	size);

	/**
	 * Default destructor.
	 * Remove and delete all elements of the table, calling the
	 * default destructor of these elements.
	 */
	~mcl_seq_table ();

	/**
	 * Insert an element in an ordered table according to its sequence
	 * number.
	 * Insert the element at its location (i.e. to comply
	 * with the increasing seq # property) in the table.
	 * @param mclcb
	 * @param elem	element to insert in table
	 * @return	completion status: MCL_OK if inserted, or MCL_ERROR
	 * 		if already present in table.
	 */
	mcl_error_status	insert	(T *elem);

	/**
	 * Remove an element from the table.
	 * This method does not try to free the element.
	 * @param mclcb
	 * @param elem  element to remove from the table
	 * @return      completion status: MCL_OK if removed, or MCL_ERROR
	 *              if not found in table or in case of error.
	 */
	mcl_error_status	remove	(T *elem);

	/**
	 * Remove all elements from the table.
	 * This method does not try to free the elements but instead
	 * calls the callback function provided for each element.
	 * @param mclcb
	 * @param func	callback function that needs to be called
	 *		for each element removed from the table, or NULL
	 *		if nothing shall be done
	 * @return      completion status: MCL_OK if removed, or MCL_ERROR
	 *              in case of error.
	 */
	mcl_error_status	remove_all (void (*func)(T*));

	/**
	 * Find the element with the given sequence number in an ordered table.
	 * @param mclcb
	 * @param seq	sequence number looked for
	 * @return	pointer to the element with that sequence number
	 *		if found, NULL if not found or in case of error.
	 */
	T			*find	(INT32 seq);

	/**
	 * Return the first element of table.
	 * @return	pointer to the first element of this sequentially
	 *		ordered table, or NULL if table is empty.
	 */
	T			*first (void);

	/**
	 * Return the last element of table.
	 * @return	pointer to the last element of this sequentially
	 *		ordered table, or NULL if table is empty.
	 */
	T			*last (void);

	/**
	 * Number of elements in table.
	 * @return	total number of elements in table
	 */
	INT32			size (void);
	
	/**
	 * Call the print method of each element in table.
	 * Traversal of table is done in increasing sequence number order.
	 */
	void			print_all (void);


private:
	T	*m_head;	// pointer to first element of table
	INT32	m_size;		// number of elements in table

	static void m_delete_callback (T *elem);// used by default destructor
						// to delete each element
};


//------------------------------------------------------------------------------
// Inlines for all classes follow
//------------------------------------------------------------------------------


template <class T>
inline INT32
mcl_seq_list<T>::size ()
{
	return this->m_size;
}

template <class T>
inline void
mcl_seq_list<T>::m_delete_callback (T *elem)
{
#if 0
	elem->print();
#endif
	delete elem;
}

template <class T>
inline T *
mcl_seq_list<T>::first (void)
{
	return m_head;
}

template <class T>
inline T *
mcl_seq_list<T>::last (void)
{
	return m_head->prev;
}

#endif // } 0
#endif // MCL_LIST_H

