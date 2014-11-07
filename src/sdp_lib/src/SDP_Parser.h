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

#ifndef SDP_PARSER_INCLUDED
#define SDP_PARSER_INCLUDED

#ifdef __cplusplus
extern "C" {
#endif

#include "SDP_Error.h"
#include "SDP_EventStreamParser.h"
#include "SDP_Description.h"
#include "SDP_Generator.h"
#include "SDP_LinkedList.h"
#include "SDP_Utility.h"



extern SDP_Description *SDP_Parse(
	SDP_Parser *   parser,
	const char *   string
);
extern SDP_Description *SDP_ParseFile(
	SDP_Parser *   parser,
	const char *   filename
);



#ifdef __cplusplus
}
#endif

#endif
