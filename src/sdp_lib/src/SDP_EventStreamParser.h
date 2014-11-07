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

#ifndef SDP_EVENT_STREAM_PARSER_INCLUDED
#define SDP_EVENT_STREAM_PARSER_INCLUDED

#ifdef __cplusplus
extern "C" {
#endif

#include <sys/types.h>
#include "SDP_Error.h"
#include "SDP_StreamTokenizer.h"
#include "SDP_Utility.h"





/*
 * We declar this out here first, instead of just wrapping the actual structure
 * declaration in a typedef because the function pointer typedefs below need
 * to take this struct as an argument, but the struct needs to contain function
 * pointers of those types (it's a chicken and egg thing):
 */
struct _SDP_Parser;

/* The various stream event handlers; */
typedef int (*SDP_StartHandler)(
	struct _SDP_Parser *   parser,
	void *                 user_data
);
typedef int (*SDP_StartDescriptionHandler)(
	struct _SDP_Parser *   parser,
	int                    current_description_number,
	void *                 user_data
);
typedef int (*SDP_FieldHandler)(
	struct _SDP_Parser *   parser,
	char                   type,
	const char *           value,
	void *                 user_data
);
typedef int (*SDP_EndDescriptionHandler)(
	struct _SDP_Parser *   parser,
	int                    current_description_number,
	void *                 user_data
);
typedef void (*SDP_EndHandler)(
	struct _SDP_Parser *   parser,
	int                    result,
	void *                 user_data
);

typedef struct _SDP_Parser {
	/*
	 * The state of the parser (SDP_STOPPED == doing nothing,
	 * SDP_RUNNING == reading lines and waiting for the first "v=" field to
	 * sink its teeth into, SDP_PARSING_SESSION_DESCRIPTION == parsing
	 * SDP fields):
	 */
	enum {
		SDP_STOPPED,
		SDP_RUNNING,
		SDP_PARSING_DESCRIPTION
	} state;

	/*
	 * A pointer to the tokenizer we'll use to chop up the stream into
	 * individual newline-delimited SDP fields:
	 */
	SDP_StreamTokenizer *stream_tokenizer;

	/* The number of the line being parsed at the moment */
	int current_line_number;

	/*
	 * The number of the current description being parsed (1 for the first
	 * description, 2 for the second...):
	 */
	int current_description_number;

	/* The type character of the field being parsed at the moment: */
	char current_field_type;

	/*
	 * A pointer to a string containing the current SDP field value
	 * (everything after the "=" sign):
	 */
	char *current_field;

	/*
	 * For a particular description, this array stores the number of times
	 * each field has been encountered. It's indexed using type characters
	 * (e.g, parser->fields_seen['o'] > 1). Everytime a "v" field is
	 * encountered, designating the start of another description, this
	 * array is reset:
	 */
	int fields_seen[256];

	/* The handlers for the event stream parser: */
	SDP_StartHandler            start_handler;
	SDP_StartDescriptionHandler start_description_handler;
	SDP_FieldHandler            field_handler;
	SDP_EndDescriptionHandler   end_description_handler;
	SDP_EndHandler              end_handler;

	/*
	 * A pointer to *something* the user wants us to pass to the first five
	 * event handlers when we call them:
	 */
	void *user_data;
} SDP_Parser;



extern SDP_Parser *SDP_NewParser(void);

extern int SDP_EventStreamParse(
	SDP_Parser *   parser,
	const char *   string
);
extern int SDP_EventStreamParseFile(
	SDP_Parser *   parser,
	const char *   filename
);

extern void SDP_SetStartHandler(
	SDP_Parser *       parser,
	SDP_StartHandler   handler
);
extern void SDP_SetStartDescriptionHandler(
	SDP_Parser *                  parser,
	SDP_StartDescriptionHandler   handler
);
extern void SDP_SetFieldHandler(
	SDP_Parser *       parser,
	SDP_FieldHandler   handler
);
extern void SDP_SetEndDescriptionHandler(
	SDP_Parser *                parser,
	SDP_EndDescriptionHandler   handler
);
extern void SDP_SetEndHandler(
	SDP_Parser *     parser,
	SDP_EndHandler   handler
);
extern void SDP_SetUserData(
	SDP_Parser *   parser,
	void *         user_data
);

extern SDP_StartHandler SDP_GetStartHandler(
	SDP_Parser *parser
);
extern SDP_StartDescriptionHandler SDP_GetStartDescriptionHandler(
	SDP_Parser *parser
);
extern SDP_FieldHandler SDP_GetFieldHandler(
	SDP_Parser *parser
);
extern SDP_EndDescriptionHandler SDP_GetEndDescriptionHandler(
	SDP_Parser *parser
);
extern SDP_EndHandler SDP_GetEndHandler(
	SDP_Parser *parser
);
extern void *SDP_GetUserData(SDP_Parser *parser);

extern int SDP_GetCurrentLineNumber(SDP_Parser *parser);
extern int SDP_GetCurrentDescriptionNumber(SDP_Parser *parser);
extern char SDP_GetCurrentFieldType(SDP_Parser *parser);
extern char *SDP_GetCurrentField(SDP_Parser *parser);
extern int SDP_FieldEncountered(
	SDP_Parser *   parser,
	char           field_type
);

extern void SDP_DestroyParser(SDP_Parser *parser);





#ifdef __cplusplus
}
#endif

#endif
