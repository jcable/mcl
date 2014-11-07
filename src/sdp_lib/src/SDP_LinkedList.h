/*******************************************************************************
 * 
 * Copyright 2004 by William G. Davis.
 *
 * This library is free software released under the terms of the GNU Lesser
 * General Public License (LGPL), the full terms of which can be found in the
 * "COPYING" file that comes with the distribution.
 *
 * This library is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.
 *
 ******************************************************************************/

#ifndef SDP_LINKEDLIST_INCLUDED
#define SDP_LINKEDLIST_INCLUDED

#ifdef __cplusplus
extern "C" {
#endif

#include "SDP_Utility.h"





typedef struct {
	/* Pointer to the first element in the linked list: */
	void *elements;

	/* The total number of elements in the linked list: */
	int total_elements;

	/* A pointer to the last element in the linked list: */
	void *last_element;
} SDP_LinkedList;



/* These macros are used as accessors for STD_LinkedList structs: */
#define SDP_GetListElements(_list_)  ((_list_).elements)
#define SDP_GetTotalElements(_list_) ((_list_).total_elements)
#define SDP_GetLastElement(_list_)   ((_list_).last_element)

/*
 * Generic next/previous linked list macros for structs that are chained
 * together to form lists:
 */
#define SDP_GetNext(_object_) \
	((_object_) ? (_object_)->next : NULL)
#define SDP_GetPrevious(_object_) \
	((_object_) ? (_object_)->previous : NULL)

#define SDP_SetNext(_object_, _next_) \
	((_object_) ? (_object_)->next = _next_ : NULL)
#define SDP_SetPrevious(_object_, _previous_) \
	((_object_) ? (_object_)->previous = _previous_ : NULL)

/*
 * This macro adds the struct "_new_object_" of type "_type_" to the
 * SDP_LinkedList struct "_list_":
 */
#define SDP_LINK_INTO_LIST(_list_, _new_object_, _type_)             \
	do {                                                         \
		if ((_list_).elements)                               \
		{                                                    \
			_type_ *last_element =                       \
				(_type_ *) (_list_).last_element;    \
                                                                     \
			SDP_SetNext(last_element, _new_object_);     \
			SDP_SetPrevious(_new_object_, last_element); \
		}                                                    \
		else                                                 \
		{                                                    \
			(_list_).elements = _new_object_;            \
		}                                                    \
                                                                     \
		(_list_).last_element = _new_object_;                \
		++(_list_).total_elements;                           \
	} while (0)

/*
 * This macro removes the struct "_object_" of type "_type_" from the
 * SDP_LinkedList struct "_list_":
 */
#define SDP_UNLINK_FROM_LIST(_list_, _object_, _type_)                     \
	do {                                                               \
		if ((_object_)->next && (_object_)->previous)              \
		{                                                          \
			(_object_)->next->previous = (_object_)->previous; \
			(_object_)->previous->next = (_object_)->next;     \
		}                                                          \
		else if ((_object_)->next)                                 \
		{                                                          \
			(_object_)->next->previous = NULL;                 \
			(_list_).elements = (_object_)->next;              \
		}                                                          \
		else if ((_object_)->previous)                             \
		{                                                          \
			(_object_)->previous->next = NULL;                 \
			(_list_).last_element = (_object_)->previous;      \
		}                                                          \
                                                                           \
		--(_list_).total_elements;                                 \
	} while (0)

/*
 * This macro destorys each struct of type "_type_" from the SDP_LinkedList
 * struct "_list_" using the destroy function "_function_":
 */
#define SDP_DESTROY_LIST(_list_, _type_, _function_)        \
	do {                                                \
		_type_ *object = (_list_).elements;         \
		                                            \
		while (object)                              \
		{                                           \
			_type_ *object_to_destroy = object; \
                                                            \
			object = object->next;              \
                                                            \
			_function_(object_to_destroy);      \
		}                                           \
                                                            \
		(_list_).elements       = NULL;             \
		(_list_).total_elements = 0;                \
		(_list_).last_element   = NULL;             \
	} while (0)





#ifdef __cplusplus
}
#endif

#endif
