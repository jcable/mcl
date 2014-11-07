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
 *------------------------------------------------------------------------------
 *
 * This file contains routines that implement the lower-level SDP event stream
 * parser. See SinisterSdpParser.html for documentation.
 * 
 ******************************************************************************/

#include <ctype.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include "SDP_EventStreamParser.h"
#include "SDP_StreamTokenizer.h"
#include "SDP_Utility.h"







/* This does the actual parsing: */
static int _ParseStream(SDP_Parser *parser);

/* These routines invoke user's registered handlers: */
static int _InvokeStartHandler(SDP_Parser *parser);
static int _InvokeStartDescriptionHandler(SDP_Parser *parser);
static int _InvokeFieldHandler(SDP_Parser *parser);
static int _InvokeEndDescriptionHandler(SDP_Parser *parser);
static void _InvokeEndHandler(
	SDP_Parser *   parser,
	int            rv
);





SDP_Parser *SDP_NewParser(void)
{
	SDP_Parser *parser = (SDP_Parser *) SDP_Allocate(sizeof(SDP_Parser));
	if (parser == NULL)
	{
		SDP_RaiseFatalError(
			SDP_ERR_OUT_OF_MEMORY,
			"Couldn't allocate memory to create a new SDP "
			"parser: %s.",
			SDP_OS_ERROR_STRING
		);
		return NULL;
	}

	memset(parser, 0, sizeof(SDP_Parser));

	/* Set the default state and initialize the token buffer too: */
	parser->state            = SDP_STOPPED;
	parser->stream_tokenizer = SDP_NewStreamTokenizer();
	if (parser->stream_tokenizer == NULL)
	{
		SDP_RaiseFatalError(
			SDP_ERR_OUT_OF_MEMORY,
			"Couldn't allocate memory for the SDP parser's "
			"tokenizer: %s.",
			SDP_OS_ERROR_STRING
		);
		return NULL;
	}

	return parser;
}





int SDP_EventStreamParse(
	SDP_Parser *   parser,
	const char *   string)
{
	SDP_AssertNotNull(parser);
	SDP_AssertNotNull(string);

	SDP_UseStringAsStream(parser->stream_tokenizer, string);

	return _ParseStream(parser);
}





int SDP_EventStreamParseFile(
	SDP_Parser *   parser,
	const char *   filename)
{
	FILE *file;
	int rv;

	SDP_AssertNotNull(parser);
	SDP_AssertNotNull(filename);

	file = fopen(filename, "r");
	if (file == NULL)
	{
		SDP_RaiseFatalError(
			SDP_ERR_FILE_OPEN_FAILED,
			"Couldn't open file \"%s\" to parse it as a stream: "
			"%s.",
			filename,
			SDP_OS_ERROR_STRING
		);
		return SDP_FAILURE;
	}

	SDP_UseFileAsStream(parser->stream_tokenizer, file);

	rv = _ParseStream(parser);

	fclose(file);

	return rv;
}





/*******************************************************************************
 *
 * 	Name
 * 		_ParseStream(parser)
 * 
 * 	Purpose
 * 		This function parses the actual stream, invoking registered
 * 		event handlers when the events occur. A stream must be set for
 * 		the internal "stream_tokenizer" struct using one of the
 * 		SDP_Use*AsStream() functions before calling this function.
 * 
 * 	Parameters
 * 		parser - A pointer to an SDP_Parser struct.
 * 
 */

static int _ParseStream(SDP_Parser *parser)
{
	/* The current token: */
	char *token;

	/* The type of the field being parsed: */
	int type_char;

	/*
	 * After we break out of the loop below, if something went wrong, then
	 * this will be false:
	 */
	int rv = SDP_SUCCESS;


	SDP_AssertNotNull(parser);

	/*
	 * Before we start parsing, we need to invoke any specified start
	 * handler:
	 */
	rv = _InvokeStartHandler(parser);
	if (SDP_FAILED(rv))
		return SDP_FAILURE;

	/* Now parse each newline-delimited token from the specified stream: */
	parser->state               = SDP_RUNNING;
	parser->current_line_number = 0;
	while (1)
	{
		++parser->current_line_number;

		/* Reset these two for the next token: */
		parser->current_field_type = '\0';
		parser->current_field      = NULL;

		/* Get the next CR, LF, or CRLF delimited token: */
		rv = SDP_GetNextToken(parser->stream_tokenizer, &token);
		if (SDP_FAILED(rv))
		{
			SDP_RaiseFatalError(
				SDP_ERR_OUT_OF_MEMORY,
				"Couldn't allocate memory to get the next "
				"line from the stream: %s.",
				SDP_OS_ERROR_STRING
			);
			break;
		}

		/* No more tokens? */
		if (token == NULL)
			break;

		/* 
		 * Check for an empty token, a newline that was followed right
		 * away by another newline:
		 */
		if (*token == '\0')
		{
			if (parser->state == SDP_PARSING_DESCRIPTION)
			{
				/*
				 * If it's a blank line and we're already
				 * parsing a description, then take it as the
				 * end of the description:
				 */
				rv = _InvokeEndDescriptionHandler(parser);

				/* We're outside of any description, so... */
				parser->state = SDP_RUNNING;

				if (SDP_SUCCEEDED(rv))
					continue;
				else
					break;
			}
			else
			{
				/*
				 * Keep going to till we find a non-blank
				 * line:
				 */
				continue;
			}
		}
	
		/* Make sure the field is in the valid "x=value" format: */
		if (!isalpha(*token) || *(token + 1) != '=')
		{
			rv = SDP_RaiseNonFatalError(
				SDP_ERR_MALFORMED_LINE,
				"Line %d is malformed; it doesn't contain a "
				"proper \"x=value\" sequence, and must be "
				"discarded.",
				parser->current_line_number
			);
			if (SDP_SUCCEEDED(rv))
				continue;
			else
				break;
		}

		/* Get the type character as an int so we can test it: */
		type_char = *token;

		/* Make sure its in range: */
		if (type_char < 0 || type_char > 255)
		{
			rv = SDP_RaiseNonFatalError(
				SDP_ERR_INVALID_TYPE_CHARACTER,
				"Line %d contains a type character with an " 
				"ordinal value \"%d\", which is out of range; "
				"isn't 8-bit ISO Latin-1.",
				parser->current_line_number,
				type_char
			);
			if (SDP_SUCCEEDED(rv))
				continue;
			else
				break;
		}

		/*
		 * Cast the type character, just to be safe, cause we're gonna
		 * use it to subscript an array later on:
		 */
		parser->current_field_type = (unsigned char) type_char;

		/* Move beyond the "x=" to get the field value: */
		parser->current_field = token += 2;

		/*
		 * The standard doesn't allow for custom fields; it defines a
		 * dozen or so fields and their associated type characters, and
		 * allows for extensions via "a" fields:
		 */
		if (!SDP_IsKnownFieldType(parser->current_field_type))
		{
			rv = SDP_RaiseNonFatalError(
				SDP_ERR_INVALID_TYPE_CHARACTER,
				"Line %d contains a \"%c\" field. The standard "
				"has defined no such field, and mandates user-"
				"defined extensions to be created using \"a\" "
				"fields instead; it has been discarded.",
				parser->current_line_number,
				parser->current_field_type
			);
			if (SDP_SUCCEEDED(rv))
				continue;
			else
				break;
		}

		/* A "v" field signifies the start of an SDP description */
		if (parser->current_field_type == 'v')
		{
			if (parser->state == SDP_PARSING_DESCRIPTION)
			{
				/*
				 * If we've already parsed a description, then
				 * the "v" field signals the end of the current
				 * description and the start of a new one:
				 */
				rv = _InvokeEndDescriptionHandler(parser);
				if (SDP_FAILED(rv))
					break;

				/*
				 * We need to reset the list of fields we've
				 * seen, since we're starting on a new session
				 * description with new fields:
				 */
				memset(
					parser->fields_seen,
					0,
					sizeof(parser->fields_seen)
				);
			}
			else
			{
				parser->state = SDP_PARSING_DESCRIPTION;
			}

			++parser->current_description_number;

			/*
			 * Invoke the start handler for the start of this new
			 * description:
			 */
			rv = _InvokeStartDescriptionHandler(parser);
			if (SDP_FAILED(rv))
				break;
		}
		else
		{
			/*
			 * Make sure that if it isn't a "v=" field, that we've
			 * *already* encountered one:
			 */
			if (!SDP_FieldEncountered(parser, 'v')
				|| parser->state != SDP_PARSING_DESCRIPTION)
			{
				rv = SDP_RaiseNonFatalError(
					SDP_ERR_FIELDS_OUT_OF_SEQUENCE,
					"Line %d contains a \"%c\" field, but "
					"no \"v\" field has been encountered "
					"yet; this doesn't seem to belong "
					"to any description and it has been "
					"discarded",
					parser->current_line_number,
					parser->current_field_type
				);
				if (SDP_SUCCEEDED(rv))
					continue;
				else
					break;
			}
		}

		/*
		 * Some fields only appear once in an entire session
		 * description (like "v" or "o"), whereas others can appear
		 * multiple times (like "a" or "t"):
		 */
		if (SDP_FieldEncountered(parser, parser->current_field_type)
			&& (parser->current_field_type == 'o'
				|| parser->current_field_type == 's'
				|| parser->current_field_type == 'u'
				|| parser->current_field_type == 'z'))
		{
			rv = SDP_RaiseNonFatalError(
				SDP_ERR_MULTIPLE_UNIQUE_FIELDS,
				"Line %d contains a \"%c\" field, but a "
				"\"%c\" field was already encountered. There "
				"should be only one \"%c\" field per-session "
				"description.",
				parser->current_line_number,
				parser->current_field_type,
				parser->current_field_type,
				parser->current_field_type
			);
			if (rv)
				continue;
			else
				break;
		}

		/*
		 * This looks bad, and the cast really isn't needed since we
		 * already casted it and made sure it was in range before that,
		 * but gcc warns under -Wall if we take it out:
		 */
		++parser->fields_seen[
			(unsigned char) parser->current_field_type
		];



		/*
		 * Now that we've extracted the field type and field value and
		 * we've performed some semblance of validation, try to invoke
		 * the user's field handler to handle the field:
		 */
		rv = _InvokeFieldHandler(parser);
		if (SDP_FAILED(rv))
			break;
	}

	/*
	 * Now try to invoke the end description handler one last time, unless
	 * some error occurred, and capture its return status:
	 */
	if (parser->state == SDP_PARSING_DESCRIPTION && SDP_SUCCEEDED(rv))
		rv = _InvokeEndDescriptionHandler(parser);

	/* Reset the state: */
	parser->state = SDP_STOPPED;

	/* Now the end handler, since we're done parsing: */
	_InvokeEndHandler(parser, rv);

	return rv;
}





void SDP_SetStartHandler(
	SDP_Parser *       parser,
	SDP_StartHandler   handler)
{
	SDP_AssertNotNull(parser);

	parser->start_handler = handler;
}





void SDP_SetStartDescriptionHandler(
	SDP_Parser *                  parser,
	SDP_StartDescriptionHandler   handler)
{
	SDP_AssertNotNull(parser);

	parser->start_description_handler = handler;
}





void SDP_SetFieldHandler(
	SDP_Parser *       parser,
	SDP_FieldHandler   handler)
{
	SDP_AssertNotNull(parser);

	parser->field_handler = handler;
}





void SDP_SetEndDescriptionHandler(
	SDP_Parser *                parser,
	SDP_EndDescriptionHandler   handler)
{
	SDP_AssertNotNull(parser);

	parser->end_description_handler = handler;
}





void SDP_SetEndHandler(
	SDP_Parser *     parser,
	SDP_EndHandler   handler)
{
	SDP_AssertNotNull(parser);

	parser->end_handler = handler;
}





void SDP_SetUserData(
	SDP_Parser *   parser,
	void *         user_data)
{
	SDP_AssertNotNull(parser);

	parser->user_data = user_data;
}





SDP_StartHandler SDP_GetStartHandler(SDP_Parser *parser)
{
	SDP_AssertNotNull(parser);

	return parser->start_handler;
}





SDP_StartDescriptionHandler SDP_GetStartDescriptionHandler(SDP_Parser *parser)
{
	SDP_AssertNotNull(parser);

	return parser->start_description_handler;
}





SDP_FieldHandler SDP_GetFieldHandler(SDP_Parser *parser)
{
	SDP_AssertNotNull(parser);

	return parser->field_handler;
}





SDP_EndDescriptionHandler SDP_GetEndDescriptionHandler(SDP_Parser *parser)
{
	SDP_AssertNotNull(parser);

	return parser->end_description_handler;
}





SDP_EndHandler SDP_GetEndHandler(SDP_Parser *parser)
{
	SDP_AssertNotNull(parser);

	return parser->end_handler;
}





void *SDP_GetUserData(SDP_Parser *parser)
{
	SDP_AssertNotNull(parser);

	return parser->user_data;
}





int SDP_GetCurrentLineNumber(SDP_Parser *parser)
{
	SDP_AssertNotNull(parser);

	return parser->current_line_number;
}





int SDP_GetCurrentDescriptionNumber(SDP_Parser *parser)
{
	SDP_AssertNotNull(parser);

	return parser->current_description_number;
}





char SDP_GetCurrentFieldType(SDP_Parser *parser)
{
	SDP_AssertNotNull(parser);
	
	return parser->current_field_type;
}





char *SDP_GetCurrentField(SDP_Parser *parser)
{
	SDP_AssertNotNull(parser);
	
	return parser->current_field;
}





int SDP_FieldEncountered(
	SDP_Parser *    parser,
	char            field_type)
{
	SDP_AssertNotNull(parser);
	SDP_Assert(field_type >= 0);

	return parser->fields_seen[(unsigned char) field_type];
}





void SDP_DestroyParser(SDP_Parser *parser)
{
	SDP_AssertNotNull(parser);

	if (parser->stream_tokenizer)
		SDP_DestroyStreamTokenizer(parser->stream_tokenizer);

	SDP_Destroy(parser);
}





/*******************************************************************************
 *
 * 	Name
 * 		_InvokeStartHandler(parser)
 * 
 * 	Purpose
 * 		This function invokes any registered start handler with a
 * 		pointer to the parser struct as the first argument and a
 * 		pointer to any specified user data as the second argument.
 *
 * 		It returns the return value of the user's handler, or just true
 * 		if no handler is registered.
 * 
 * 	Parameters
 * 		parser - A pointer to an SDP_Parser struct.
 * 
 */

static int _InvokeStartHandler(SDP_Parser *parser)
{
	if (parser->start_handler)
		return parser->start_handler(parser, parser->user_data);
	else
		return SDP_SUCCESS;
}





/*******************************************************************************
 *
 * 	Name
 * 		_InvokeStartDescriptionHandler(parser)
 * 
 * 	Purpose
 * 		This function invokes any registered start-of-description
 * 		handler with a pointer to the parser struct as the first
 * 		argument, the number of the description that just started as
 * 		the second argument, and a pointer to any specified user data
 * 		as the third argument.
 *
 * 		It returns the return value of the user's handler, or just true
 * 		if no handler is registered.
 * 
 * 	Parameters
 * 		parser - A pointer to an SDP_Parser struct.
 * 
 */

static int _InvokeStartDescriptionHandler(SDP_Parser *parser)
{
	if (parser->start_description_handler)
		return parser->start_description_handler(
			parser,
			parser->current_description_number,
			parser->user_data
		);
	else
		return SDP_SUCCESS;
}





/*******************************************************************************
 *
 * 	Name
 * 		_InvokeFieldHandler(parser)
 * 
 * 	Purpose
 * 		This function invokes any registered field handler with a
 * 		pointer to the parser struct as the first argument, the type
 * 		character of the field to handle as the seocnd argument, a
 * 		pointer to a string containing the value of the field to
 * 		handler as the third argument, and a pointer to any specified
 * 		user data as the fourth argument.
 * 
 * 		It returns the return value of the user's handler, or just true
 * 		if no handler is registered.
 * 
 * 	Parameters
 * 		parser - A pointer to an SDP_Parser struct.
 * 
 */

static int _InvokeFieldHandler(SDP_Parser *parser)
{
	if (parser->field_handler)
		return parser->field_handler(
			parser,
			parser->current_field_type,
			parser->current_field,
			parser->user_data
		);
	else
		return SDP_SUCCESS;
}





/*******************************************************************************
 *
 * 	Name
 * 		_InvokeEndDescriptionHandler(parser)
 * 
 * 	Purpose
 * 		This function invokes any registered end-of-description handler
 * 		with a pointer to the parser struct as the first argument, the
 * 		number of the description that just ended as the second
 * 		argument, and a pointer to any specified user data as the third
 * 		argument.
 *
 * 		It returns the return value of the user's handler, or just true
 * 		if no handler is registered.
 * 
 * 	Parameters
 * 		parser - A pointer to an SDP_Parser struct.
 * 
 */

static int _InvokeEndDescriptionHandler(SDP_Parser *parser)
{
	if (parser->end_description_handler)
		return parser->end_description_handler(
			parser,
			parser->current_description_number,
			parser->user_data
		);
	else
		return SDP_SUCCESS;
}





/*******************************************************************************
 *
 * 	Name
 * 		_InvokeEndHandler(parser, result)
 * 
 * 	Purpose
 * 		This function invokes any registered end handler with a pointer
 * 		to the parser struct as the first argument, the result of
 * 		parsing (success or failure) as the second argument, and a
 * 		pointer to any specified user data as the third argument.
 * 
 * 	Parameters
 * 		parser - A pointer to an SDP_Parser struct.
 * 		result - The result of _ParseStream; true or false, success or
 * 		         failure. This will be passed on to the user's end
 * 		         handler as the second argument.
 * 
 */

static void _InvokeEndHandler(
	SDP_Parser *   parser,
	int            result)
{
	if (parser->end_handler)
		parser->end_handler(parser, result, parser->user_data);
}
