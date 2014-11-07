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

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "SDP_Utility.h"







/* The array to contain pointers to descriptions for all standardized types: */
static char *_type_description_table[256];

/*
 * Has the array been filled with pointers to strings containing descriptions
 * yet?
 */
static int _type_table_initialized = 0;

/* The function used to fill the array: */
static void _InitializeFieldTypeArray(void);

int SDP_IsKnownFieldType(char type)
{
	SDP_Assert(type >= 0);

	_InitializeFieldTypeArray();
	if (_type_description_table[(unsigned char) type])
		return 1;
	else
		return 0;
}





char *SDP_GetFieldTypeDescription(char type)
{
	SDP_Assert(type >= 0);

	_InitializeFieldTypeArray();

	return _type_description_table[(unsigned char) type];
}





void SDP_AssertionFailed(
	const char *format,
	...)
{
	va_list args;

	va_start(args, format);
	vfprintf(stderr, format, args);
	va_end(args);

	exit(0);
}





static void _InitializeFieldTypeArray(void)
{
	if (_type_table_initialized)
		return;

	/* Initialze each type to a NULL pointer... */
	memset(_type_description_table, 0, sizeof(_type_description_table));

	/* 
	 * ...then replace the NULL pointers with pointers to strings
	 * describing the types we recognize:
	 */
	_type_description_table['v'] = "protocol version field";
	_type_description_table['o'] = "owner/origin field";
	_type_description_table['s'] = "session name field";
	_type_description_table['i'] = "session or media description field";
	_type_description_table['u'] = "URI field";
	_type_description_table['e'] = "email address field";
	_type_description_table['p'] = "phone number field";
	_type_description_table['c'] = "session or media connection data field";
	_type_description_table['b'] = "session or media bandwidth "
	                               "information field";
	_type_description_table['t'] = "session start/stop time field";
	_type_description_table['r'] = "repeat interval field";
	_type_description_table['z'] = "timezone adjustments field";
	_type_description_table['e'] = "encryption information field";
	_type_description_table['a'] = "session or media attribute field";
	_type_description_table['m'] = "media description field";

	_type_table_initialized = 1;
}
