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
 * This file contains routines that parse individual SDP fields and fill
 * SDP_Description structs. It uses the event stream parser and defines its own
 * custom event handlers to do the additional parsing of each field. It uses
 * the SDP generator to output the description.
 *
 *------------------------------------------------------------------------------
 * 
 * See SinisterSdpParser.html for documentation.
 *
 ******************************************************************************/

#include <ctype.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include "SDP_Error.h"
#include "SDP_EventStreamParser.h"
#include "SDP_Generator.h"
#include "SDP_LinkedList.h"
#include "SDP_Parser.h"
#include "SDP_Str.h"
#include "SDP_Description.h"
#include "SDP_Utility.h"

/*
 * This is used to test an error string supplied as the first argument to
 * _CheckForMissingField() after calling _CheckForMissingField():
 */
#define ARE_REQUIRED_FIELDS_MISSING(_string_) \
	(((_string_) && *(_string_)) ? 1: 0)

/*
 * This converts string containing a Network Time Protocol timestamp to
 * a time_t:
 */
#define NTP_DIFFERENCE 2208988800UL
#define NETWORK_TIME_TO_OS_TIME(_ntp_time_value_)                  \
	(((_ntp_time_value_) >= NTP_DIFFERENCE)                    \
		? ((time_t) ((_ntp_time_value_) - NTP_DIFFERENCE)) \
		: ((time_t) (_ntp_time_value_)))







/* This struct is used to build up the SDP_Description structs: */ 
typedef struct {
	/* A pointer to the current event stream parser struct: */
	SDP_Parser *parser;

	/*
	 * We use our own custom handlers to build the descriptions, but before
	 * resgistering them, we save pointers to the user's event stream
	 * handlers and their user data, then when we're done, we set things
	 * back to the way they were:
	 */
	SDP_StartHandler              saved_start_handler;
	SDP_StartDescriptionHandler   saved_start_description_handler;
	SDP_FieldHandler              saved_field_handler;
	SDP_EndDescriptionHandler     saved_end_description_handler;
	SDP_EndHandler                saved_end_handler;
	void *                        saved_user_data;

	/*
	 * A pointer to the place to store the pointer to the resulting session
	 * description structs:
	 */
	SDP_Description **descriptions_destination;

	/* The linked list of session descriptions we're going to build: */
	SDP_LinkedList descriptions;

	/* The current state of the description builder: */
	enum {
		BUILDING_DESCRIPTION_STRUCTS,
		BUILDING_SESSION_TIME_STRUCTS,
		BUILDING_MEDIA_DESCRIPTION_STRUCTS
	} state;

	/*
	 * If we're parsing time fields at the moment, then this will contain
	 * the current time struct we're filling:
	 */
	SDP_SessionPlayTime *current_session_play_time;

	/*
	 * If we're parsing media description fields at the moment, then this
	 * will contain a pointer to the current media description struct we're
	 * filling:
	 */
	SDP_MediaDescription *current_media_description;
} SDP_DescriptionBuilder;





/* Registers our handlers with the event stream parser to build the structs: */
static int _RegisterCustomHandlers(
	SDP_Parser *         parser,
	SDP_Description **   descriptions_destination
);

/* The custom handlers used to build the SDP_Description description: */
static int _CustomStartDescriptionHandler(
	SDP_Parser *   parser,
	int            current_description,
	void *         user_data
);
static int _CustomFieldHandler(
	SDP_Parser *   parser,
	char           field_type,
	const char *   field_value,
	void *         user_data
);
static void _CustomEndHandler(
	SDP_Parser *   parser,
	int            result,
	void *         user_data
);

/*
 * This array stores pointers to functions to handle each specific type of SDP
 * field:
 */
static int (*_field_parsers_table[256])(
	SDP_Parser *               parser,
	SDP_DescriptionBuilder *   description_builder,
	SDP_Description *          description,
	const char *               field_value
);

/* Has the array been initialized yet? */
static int _field_parsers_table_initialized = 0;

/* This function initializes the function pointer table to parse each field: */
static void _InitializeFieldParsersTable(void);





SDP_Description *SDP_Parse(
	SDP_Parser *   parser,
	const char *   string)
{
	SDP_Description *descriptions;
	int rv;

	SDP_AssertNotNull(parser);
	SDP_AssertNotNull(string);

	rv = _RegisterCustomHandlers(parser, &descriptions);
	if (SDP_FAILED(rv))
		return NULL;

	SDP_EventStreamParse(parser, string);

	return descriptions;
}





SDP_Description *SDP_ParseFile(
	SDP_Parser *   parser,
	const char *   filename)
{
	SDP_Description *descriptions;
	int rv;

	SDP_AssertNotNull(parser);
	SDP_AssertNotNull(filename);

	rv = _RegisterCustomHandlers(parser, &descriptions);
	if (SDP_FAILED(rv))
		return NULL;

	SDP_EventStreamParseFile(parser, filename);

	return descriptions;
}





/*******************************************************************************
 *
 * 	Name
 * 		_RegisterCustomHandlers(parser, descriptions_destination)
 *
 * 	Purpose
 * 		This function registers handlers with the event stream parser
 * 		that extract values from each field and create and fill
 * 		SDP_Description structs with them.
 *
 * 		It creates a new SDP_DescriptionBuilder struct, saves away the
 * 		user's event handlers and user data in it, then overwrites them
 * 		in the parser struct with its own custom handlers that build up
 * 		the structs, and sets the description builder struct as the user
 * 		data so it gets passed to those registered handlers.
 *
 * 		It returns true if it succeeds, false if it fails (if it
 * 		couldn't allocate memory for the SDP_DescriptionBuilder struct).
 *
 * 	Parameters
 * 		parser                   - A pointer to a parser struct.
 * 		descriptions_destination - A pointer to an SDP_Description
 * 		                           pointer that will end up pointing
 * 		                           to the first struct in the list if
 * 		                           parsing was successful or NULL if it
 * 		                           was unsuccessful.
 * 
 */

static int _RegisterCustomHandlers(
	SDP_Parser *         parser,
	SDP_Description **   descriptions_destination)
{
	SDP_DescriptionBuilder *description_builder =
		(SDP_DescriptionBuilder *) SDP_Allocate(
			sizeof(SDP_DescriptionBuilder)
		);
	if (description_builder == NULL)
	{
		SDP_RaiseFatalError(
			SDP_ERR_OUT_OF_MEMORY,
			"Couldn't allocate memory for the SDP description "
			"builder: %s.",
			SDP_OS_ERROR_STRING
		);
		return SDP_FAILURE;
	}

	memset(description_builder, 0, sizeof(SDP_DescriptionBuilder));

	description_builder->parser = parser;

	description_builder->descriptions_destination =
		descriptions_destination;

	/* Save the user's event handlers... */
	description_builder->saved_start_handler             =
		SDP_GetStartHandler(parser);
	description_builder->saved_start_description_handler =
		SDP_GetStartDescriptionHandler(parser);
	description_builder->saved_field_handler             =
		SDP_GetFieldHandler(parser);
	description_builder->saved_end_description_handler   =
		SDP_GetEndDescriptionHandler(parser);
	description_builder->saved_end_handler               =
		SDP_GetEndHandler(parser);

	/* ...and their user data: */
	description_builder->saved_user_data = SDP_GetUserData(parser);

	/* ...so we can overwrite them: */
	SDP_SetStartHandler(parser, NULL);
	SDP_SetStartDescriptionHandler(parser, _CustomStartDescriptionHandler);
	SDP_SetFieldHandler(parser, _CustomFieldHandler);
	SDP_SetEndDescriptionHandler(parser, NULL);
	SDP_SetEndHandler(parser, _CustomEndHandler);

	/*
	 * We pass our builder struct to each of the registered hadlers as user
	 * data:
	 */
	SDP_SetUserData(parser, description_builder);

	return SDP_SUCCESS;
}





/*******************************************************************************
 *
 * 	Name
 * 		_CustomStartDescriptionHandler(
 * 			parser, current_description_number, user_data
 * 		);
 *
 * 	Purpose
 * 		This function is registered by _RegisterCustomHandlers() as the
 * 		start-of-description handler. It creates a new description
 * 		struct to handle the description, and links it into the linked
 * 		list of description structs.
 *
 * 		It returns true if it can create the struct, false if it can't.
 *
 * 	Parameters
 * 		Same as all other start-of-description handlers.
 * 
 */

static int _CustomStartDescriptionHandler(
	SDP_Parser *   parser,
	int            current_description_number,
	void *         user_data)
{
	SDP_DescriptionBuilder *description_builder;
	SDP_Description *new_description;

	/* 
	 * Allocate the new description struct and link it into the
	 * descriptions list:
	 */
	description_builder = (SDP_DescriptionBuilder *) user_data;
	new_description     = SDP_NewDescription();
	if (new_description == NULL)
		return SDP_FAILURE;

	SDP_LINK_INTO_LIST(
		description_builder->descriptions,
		new_description,
		SDP_Description
	);

	/* Set or reset the builder state: */
	description_builder->state = BUILDING_DESCRIPTION_STRUCTS;

	return SDP_SUCCESS;
}





/*******************************************************************************
 *
 * 	Name
 * 		_CustomFieldHandler(parser, field_type, field_value, user_data);
 *
 * 	Purpose
 * 		This function is registered by _RegisterCustomHandlers() as the
 * 		field handler. If it recognizes the field, it will invoke a
 * 		function to parse it and store its values in the current
 * 		SDP_Description struct being filled.
 *
 * 		It returns true if it parses the field successfully, false if
 * 		it can't.
 * 
 * 	Parameters
 * 		Same as all field handlers.
 * 
 */

static int _CustomFieldHandler(
	SDP_Parser *    parser,
	char            field_type,
	const char *    field_value,
	void *          user_data)
{
	SDP_DescriptionBuilder *description_builder;
	int type_char;

	/*
	 * Probably isn't needed, as the event stream parser makes sure this
	 * is an unsigned char, but it can't hurt:
	 */
	type_char = field_type;
	SDP_Assert(type_char > 0 && type_char < 255);

	/* Get our passed-around builder struct: */
	description_builder = (SDP_DescriptionBuilder *) user_data;

	/*
	 * Initialize the field parser function pointer table if we haven't
	 * done so yet:
	 */
	_InitializeFieldParsersTable();

	/*
	 * Now invoke the proper handler for this specific type of field
	 * (the casts are just to shut GCC up; field_type has already been
	 * checked to make sure it's in range (not negative, not greater than
	 * 255): */
	if (_field_parsers_table[(unsigned char) field_type])
		return (*_field_parsers_table[(unsigned char) field_type])(
			description_builder->parser,
			description_builder,
			SDP_GetLastElement(description_builder->descriptions),
			field_value
		);
	else
		return SDP_SUCCESS;
}





/*******************************************************************************
 *
 * 	Name
 * 		_CustomEndHandler(parser, result, user_data);
 *
 * 	Purpose
 * 		This function is registered by _RegisterCustomHandlers() as the
 * 		end handler. It is invoked when parsing stops, and is tasked
 * 		with unregistering the custom handlers and reregistering the
 * 		user's handlers and user data that were saved away in the
 * 		SDP_DescriptionBuilder struct.
 *
 * 		If parsing was successful, then it stores a pointer to the
 * 		first SDP_Description struct in the linked list to the place
 * 		specified by the second argument to _RegisterCustomHandlers(),
 * 		otherwise, it stores a NULL pointer there instead.
 * 
 * 	Parameters
 * 		Same as all other end handlers.
 * 
 */

static void _CustomEndHandler(
	SDP_Parser *   parser,
	int            result,
	void *         user_data)
{
	SDP_DescriptionBuilder *description_builder =
		(SDP_DescriptionBuilder *) user_data;

	/* Overwrite our handlers with the user's: */
	SDP_SetStartHandler(
		parser, description_builder->saved_start_handler
	);
	SDP_SetStartDescriptionHandler(
		parser, description_builder->saved_start_description_handler
	);
	SDP_SetFieldHandler(
		parser, description_builder->saved_field_handler
	);
	SDP_SetEndDescriptionHandler(
		parser, description_builder->saved_end_description_handler
	);
	SDP_SetEndHandler(
		parser, description_builder->saved_end_handler
	);

	/* ...and reset their user data: */
	SDP_SetUserData(parser, description_builder->saved_user_data);

	/*
	 * If we successfully parsed parsed the input and built up the
	 * descriptions, then we pass on a pointer to them. Otherwise, if
	 * parsing failed or something else bad happened (like if we ran out of
	 * memory), then we destory any malformed description and pass on a
	 * NULL pointer:
	 */
	if (SDP_SUCCEEDED(result))
	{
		*(description_builder->descriptions_destination) =
			SDP_GetListElements(description_builder->descriptions);
	}
	else
	{
		*(description_builder->descriptions_destination) = NULL;
		SDP_DestroyDescriptions(
			SDP_GetListElements(description_builder->descriptions)
		);
	}

	SDP_Destroy(description_builder);
}





/*******************************************************************************
 *
 * These functions handle specific SDP fields. They're called by the
 * description builder's custom field handler where appropriate.
 *
 ******************************************************************************/

/*
 * This is used by both _ParseEmailContactField() and
 * _ParsePhoneContactField(), as "e" and "p" fields both share more or less the
 * same format:
 */
#define ERR_MISSING_REQUIRED_FIELD           1
#define ERR_MISSING_ANGLE_BRACKET_TERMINATOR 2
#define ERR_MISSING_PARENTHESIS_TERMINATOR   3

static int _ParseContactField(
	char *    field_to_parse,
	char **   required_value,
	char **   optional_value,
	int *     error
);

/*
 * This is used to check if a required subfield is missing, and if it is,
 * concatenate its name to a string containing the names of all other missing
 * subfields separated by commas:
 */
static int _CheckForMissingField(
	char *         missing_fields,
	const char *   field,
	const char *   field_name
);

/*
 * This is used to convert a repeat time offset or zone adjustment offset from
 * the "\d+[smhd]" format to a number of seconds:
 */
static int _GetOffsetInSeconds(
	const char *   string,
	long *         offset_in_seconds,
	char *         error_string,
	size_t         error_string_size
);

/*******************************************************************************
 *
 * 	Name
 * 		_ParseProtocolVersionField(
 * 			parser, description_builder, description, field_value
 * 		);
 *
 * 	Purpose
 * 		This function tries to extract a protocol version number from
 * 		a "v" field and store it in the description struct.
 *
 * 		It returns true if it parses the field successfully, false
 * 		otherwise.
 * 
 */

static int _ParseProtocolVersionField(
	SDP_Parser *               parser,
	SDP_DescriptionBuilder *   description_builder,
	SDP_Description *          description,
	const char *               field_value)
{
	int protocol_version = strtol(field_value, (char **) NULL, 10);

	if (protocol_version == 0
		&& (SDP_OS_ERROR_CODE == EINVAL
			|| SDP_OS_ERROR_CODE == ERANGE))
	{
		return SDP_RaiseNonFatalError(
			SDP_ERR_MALFORMED_V_FIELD,
			"Line %d contains a malformed \"v\" %s and could not "
			"be parsed: %s",
			SDP_GetCurrentLineNumber(parser),
			SDP_GetFieldTypeDescription('v'),
			SDP_OS_ERROR_STRING
		);
	}

	SDP_SetProtocolVersion(description, protocol_version);

	return SDP_SUCCESS;
}





/*******************************************************************************
 *
 * 	Name
 * 		_ParseOwnerField(
 * 			parser, description_builder, description, field_value
 * 		);
 *
 * 	Purpose
 * 		This function tries to extract a the username, session ID,
 * 		session version, network type, address type, and address from
 * 		an "o" field and store them in the current description struct.
 *
 * 		It returns true if it parses the field successfully, false
 * 		otherwise.
 * 
 */

static int _ParseOwnerField(
	SDP_Parser *               parser,
	SDP_DescriptionBuilder *   description_builder,
	SDP_Description *          description,
	const char *               field_value)
{
	/* Used to copy and traverse the "o" field value: */
	char *copy_of_field;
	char *c;

	/* Pointers to each substring in the copy of the "o" field: */
	char *username;
	char *session_id;
	char *session_version;
	char *network_type;
	char *address_type;
	char *address;

	/*
	 * This stores the name of each missing subfield for the error message
	 * we need to generate if the field is malformed:
	 */
	char missing_fields[128] = "";

	int rv;



	/* Get a copy of the field to work with: */
	copy_of_field = SDP_StrDup(field_value);
	if (copy_of_field == NULL)
	{
		SDP_RaiseFatalError(
			SDP_ERR_OUT_OF_MEMORY,
			"Couldn't allocate memory to copy and parse \"o\" %s "
			"on line %d: %s.",
			SDP_GetFieldTypeDescription('o'),
			SDP_GetCurrentLineNumber(parser),
			SDP_OS_ERROR_STRING
		);
		return SDP_FAILURE;
	}



	/* Each of these subfields is required: */
	c = copy_of_field;
	username        = SDP_StrSep(&c, " ");
	session_id      = SDP_StrSep(&c, " ");
	session_version = SDP_StrSep(&c, " ");
	network_type    = SDP_StrSep(&c, " ");
	address_type    = SDP_StrSep(&c, " ");
	address         = SDP_StrSep(&c, " ");
	_CheckForMissingField(missing_fields, username, "username field");
	_CheckForMissingField(missing_fields, session_id, "session ID field");
	_CheckForMissingField(
		missing_fields, session_version, "session version field"
	);
	_CheckForMissingField(missing_fields, network_type, "network type");
	_CheckForMissingField(missing_fields, address_type, "address type");
	_CheckForMissingField(missing_fields, address, "address field");

	if (ARE_REQUIRED_FIELDS_MISSING(missing_fields))
	{
		rv = SDP_RaiseNonFatalError(
			SDP_ERR_MALFORMED_O_FIELD,
			"Line %d contains a malformed \"o\" %s and could not "
			"be parsed. It is missing the following required "
			"subfields: %s.",
			SDP_GetCurrentLineNumber(parser),
			SDP_GetFieldTypeDescription('o'),
			missing_fields
		);
		SDP_Destroy(copy_of_field);

		return rv;
	}



	/* Set the internal SDP_Owner struct: */
	rv = SDP_SetOwner(
		description,
		username,
		session_id,
		session_version,
		network_type,
		address_type,
		address
	);

	SDP_Destroy(copy_of_field);

	return rv;
}





/*******************************************************************************
 *
 * 	Name
 * 		_ParseSessionNameField(
 * 			parser, description_builder, description, field_value
 * 		);
 *
 * 	Purpose
 * 		Copies an "s" field and sets it to be the session name in the
 * 		SDP_Description struct being built.
 *
 * 		It returns true if it can copy the field, false otherwise.
 * 
 */

static int _ParseSessionNameField(
	SDP_Parser *               parser,
	SDP_DescriptionBuilder *   description_builder,
	SDP_Description *          description,
	const char *               field_value)
{
	return SDP_SetSessionName(description, field_value);
}





/*******************************************************************************
 *
 * 	Name
 * 		_ParseInformationField(
 * 			parser, description_builder, description, field_value
 * 		);
 *
 * 	Purpose
 * 		Copies "i" field value to be either the session information or
 * 		media information depending on what fields have already been
 * 		parsed and what struct is currently being built.
 *
 * 		It returns true if it can copy the field, false otherwise.
 * 
 */

static int _ParseInformationField(
	SDP_Parser *               parser,
	SDP_DescriptionBuilder *   description_builder,
	SDP_Description *          description,
	const char *               field_value)
{
	if (description_builder->state == BUILDING_MEDIA_DESCRIPTION_STRUCTS)
		return SDP_SetMediaInformation(
			description_builder->current_media_description,
			field_value
		);
	else
		return SDP_SetSessionInformation(description, field_value);
}





/*******************************************************************************
 *
 * 	Name
 * 		_ParseSessionNameField(
 * 			parser, description_builder, description, field_value
 * 		);
 *
 * 	Purpose
 * 		Copies a "u" field and sets it to be the URI of the session in
 * 		the SDP_Description struct currently being built.
 *
 * 		It returns true if it can copy the field, false otherwise.
 * 
 */

static int _ParseURIField(
	SDP_Parser *               parser,
	SDP_DescriptionBuilder *   description_builder,
	SDP_Description *          description,
	const char *               field_value)
{
	return SDP_SetURI(description, field_value);
}





/*******************************************************************************
 *
 * 	Name
 * 		_ParseEmailContactField(
 * 			parser, description_builder, description, field_value
 * 		);
 *
 * 	Purpose
 * 		Tries to parse an "e" field and extract the email address and
 * 		optional name of the person who can be reached at that address,
 * 		and then creates an SDP_EmailContact struct with those values
 * 		and adds it to the linked list of SDP_EmailContact structs in
 * 		the SDP_Description struct currently being built.
 *
 * 		It returns true if it can parse the field, false otherwise.
 * 
 */

static int _ParseEmailContactField(
	SDP_Parser *               parser,
	SDP_DescriptionBuilder *   description_builder,
	SDP_Description *          description,
	const char *               field_value)
{
	/* String used to chop up the "p" field into substrings: */
	char *copy_of_field;

	/* Pointers to the name and address substrings in copy_of_field: */
	char *name;
	char *address;

	/*
	 * Stores one of the above defined error codes for
	 * _ParseContactField():
	 */
	int error;

	int rv;



	/* Get a copy of the "e" field to manipulate: */
	copy_of_field = SDP_StrDup(field_value);
	if (copy_of_field == NULL)
	{
		SDP_RaiseFatalError(
			SDP_ERR_OUT_OF_MEMORY,
			"Couldn't allocate memory to copy and parse \"e\" %s "
			"on line %d: %s.",
			SDP_GetFieldTypeDescription('e'),
			SDP_GetCurrentLineNumber(parser),
			SDP_OS_ERROR_STRING
		);
		return SDP_FAILURE;
	}

	/* Extract the email address and name from it: */
	rv = _ParseContactField(copy_of_field, &address, &name, &error);
	if (SDP_FAILED(rv))
	{
		char *error_string;

		SDP_Destroy(copy_of_field);

		/* Generate a corresponding error message for the error code: */
		switch (error)
		{
			case ERR_MISSING_REQUIRED_FIELD:
				error_string = "It has no email address in it";
			case ERR_MISSING_ANGLE_BRACKET_TERMINATOR:
				error_string = "The address part is missing "
				               "the terminating \">\" angle "
					       "bracket";
			case ERR_MISSING_PARENTHESIS_TERMINATOR:
				error_string = "The name portion is missing "
				               "the terminating \")\" "
					       "parenthesis character";
			default:
				SDP_AssertionFailed(
					"Bad error code: %d.", error
				);
		}
		
		return SDP_RaiseNonFatalError(
			SDP_ERR_MALFORMED_E_FIELD,
			"Line %d contains a malformed \"e\" %s field. %s.",
			SDP_GetCurrentLineNumber(parser),
			SDP_GetFieldTypeDescription('e'),
			error_string
		);
	}



	rv = SDP_AddNewEmailContact(description, address, name);

	SDP_Destroy(copy_of_field);

	return rv;
}





/*******************************************************************************
 *
 * 	Name
 * 		_ParsePhoneContactField(
 * 			parser, description_builder, description, field_value
 * 		);
 *
 * 	Purpose
 * 		Tries to parse a "p" field and extract the phone number and
 * 		optional name of the person who can be reached at that number,
 * 		and then creates an SDP_PhoneContact struct with those values
 * 		and adds it to the linked list of SDP_PhoneContact structs in
 * 		the SDP_Description struct currently being built.
 *
 * 		It returns true if it can parse the field, false otherwise.
 * 
 */

static int _ParsePhoneContactField(
	SDP_Parser *               parser,
	SDP_DescriptionBuilder *   description_builder,
	SDP_Description *          description,
	const char *               field_value)
{
	/* A copy of the "p" field for us to chop up into substrings: */
	char *copy_of_field;

	/* The phone number and name substrings in copy_of_field: */
	char *number;
	char *name;

	/*
	 * Error code set by _ParseContactField() to one of the above defined
	 * error code constants:
	 */
	int error;

	int rv;



	/* Copy the "p" field: */
	copy_of_field = SDP_StrDup(field_value);
	if (copy_of_field == NULL)
	{
		SDP_RaiseFatalError(
			SDP_ERR_OUT_OF_MEMORY,
			"Couldn't allocate memory to copy and parse \"p\" %s "
			"on line %d: %s.",
			SDP_GetFieldTypeDescription('p'),
			SDP_GetCurrentLineNumber(parser),
			SDP_OS_ERROR_STRING
		);
		return SDP_FAILURE;
	}

	/* Extract the phone number and name from it: */
	rv = _ParseContactField(copy_of_field, &number, &name, &error);
	if (SDP_FAILED(rv))
	{
		char *error_string;

		SDP_Destroy(copy_of_field);

		/* Generate a corresponding error string for the error code: */
		switch (error)
		{
			case ERR_MISSING_REQUIRED_FIELD:
				error_string = "It has no phone number in it";
			case ERR_MISSING_ANGLE_BRACKET_TERMINATOR:
				error_string = "The number portion is missing "
				               "the terminating \">\" angle "
					       "bracket character";
			case ERR_MISSING_PARENTHESIS_TERMINATOR:
				error_string = "The name portion is missing "
				               "the terminating \")\" "
					       "parenthesis character";
			default:
				SDP_AssertionFailed(
					"Bad error code: %d.", error
				);
		}

		return SDP_RaiseNonFatalError(
			SDP_ERR_MALFORMED_P_FIELD,
			"Line %d contains a malformed \"p\" %s and could not "
			"be parsed. %s.",
			SDP_GetCurrentLineNumber(parser),
			SDP_GetFieldTypeDescription('p'),
			error_string
		);
	}



	rv = SDP_AddNewPhoneContact(description, number, name);

	SDP_Destroy(copy_of_field);

	return rv;
}





/*******************************************************************************
 *
 * 	Name
 * 		_ParseConnectionField(
 * 			parser, description_builder, description, field_value
 * 		);
 *
 * 	Purpose
 * 		Tries to parse a "c" field and extract the network type,
 * 		address type, and then the address with an optional Time To
 * 		Live value and count of other available addresses after it. If
 * 		a media description is currently being parsed, then the
 * 		resulting values become the connection information for the
 * 		current SDP_MediaDescription struct being built. Otherwise,
 * 		they become session-level connection information for the
 * 		current SDP_Description struct being built.
 *
 * 		It returns true if it can parse the field, false otherwise.
 * 
 */

static int _ParseConnectionField(
	SDP_Parser *               parser,
	SDP_DescriptionBuilder *   description_builder,
	SDP_Description *          description,
	const char *               field_value)
{
	/* These are used to copy and chop up the "c" field: */
	char *copy_of_field;
	char *c;
	char *subfield;

	/* The last two are optional: */
	char *network_type;
	char *address_type;
	char *address;
	int ttl             = 0;
	int total_addresses = 0;

	/* Stores the names of any missing required fields: */
	char missing_fields[64] = "";

	int rv;



	/* Copy the "c" field: */
	copy_of_field = SDP_StrDup(field_value);
	if (copy_of_field == NULL)
	{
		SDP_RaiseFatalError(
			SDP_ERR_OUT_OF_MEMORY,
			"Couldn't allocate memory to copy and parse \"c\" %s "
			"on line %d: %s.",
			SDP_GetFieldTypeDescription('c'),
			SDP_GetCurrentLineNumber(parser),
			SDP_OS_ERROR_STRING
		);
		return SDP_FAILURE;
	}



	/* Extract the first three required fields: */
	c = copy_of_field;
	network_type = SDP_StrSep(&c, " ");
	address_type = SDP_StrSep(&c, " ");
	address      = SDP_StrSep(&c, " /");
	_CheckForMissingField(missing_fields, network_type, "network type");
	_CheckForMissingField(missing_fields, address_type, "address type");
	_CheckForMissingField(missing_fields, address, "address");

	if (ARE_REQUIRED_FIELDS_MISSING(missing_fields))
	{
		SDP_Destroy(copy_of_field);

		return SDP_RaiseNonFatalError(
			SDP_ERR_MALFORMED_C_FIELD,
			"Line %d contains a malformed \"c\" %s and could not "
			"be parsed. It is missing the following required "
			"subfields: %s.",
			SDP_GetCurrentLineNumber(parser),
			SDP_GetFieldTypeDescription('c'),
			missing_fields
		);
	}

	/*
	 * The address may be followed by a "/" character and then a Time To
	 * Live value, and that may be followed by another "/" and a count of
	 * how many other addresses are available counting up from the base
	 * address:
	 */
	subfield = SDP_StrSep(&c, "/");
	if (subfield)
	{
		ttl = (int) strtol(subfield, (char **) NULL, 10);
		if (ttl == 0
			&& (SDP_OS_ERROR_CODE == EINVAL
				|| SDP_OS_ERROR_CODE == ERANGE))
		{
			SDP_Destroy(copy_of_field);

			return SDP_RaiseNonFatalError(
				SDP_ERR_MALFORMED_C_FIELD,
				"Line %d contains a malformed \"c\" %s and "
				"could not be parsed. The TTL value after the "
				"address is invalid: %s.",
				SDP_GetCurrentLineNumber(parser),
				SDP_GetFieldTypeDescription('c'),
				SDP_OS_ERROR_STRING
			);
		}
	}

	subfield = c;
	if (subfield)
	{
		total_addresses = (int) strtol(subfield, (char **) NULL, 10);
		if (total_addresses == 0
			&& (SDP_OS_ERROR_CODE == EINVAL
				|| SDP_OS_ERROR_CODE == ERANGE))
		{
			SDP_Destroy(copy_of_field);

			return SDP_RaiseNonFatalError(
				SDP_ERR_MALFORMED_C_FIELD,
				"Line %d contains a malformed \"c\" %s and "
				"could not be parsed. The total number of "
				"available addresses after the TTL value "
				"for the connection is invalid: %s.",
				SDP_GetCurrentLineNumber(parser),
				SDP_GetFieldTypeDescription('c'),
				SDP_OS_ERROR_STRING
			);
		}
	}



	if (description_builder->state == BUILDING_MEDIA_DESCRIPTION_STRUCTS)
		rv = SDP_SetMediaConnection(
			description_builder->current_media_description,
			network_type,
			address_type,
			address,
			ttl,
			total_addresses
		);
	else
		rv = SDP_SetConnection(
			description,
			network_type,
			address_type,
			address,
			ttl,
			total_addresses
		);

	SDP_Destroy(copy_of_field);

	return rv;
}





/*******************************************************************************
 *
 * 	Name
 * 		_ParseBandwidthField(
 * 			parser, description_builder, description, field_value
 * 		);
 *
 * 	Purpose
 * 		Tries to parse a "b" field and extract the bandwidth limit and
 * 		what it applies to. If a media description is currently being
 * 		parsed, then the resulting values apply only to the media and
 * 		get added to the current SDP_MediaDescription struct being
 * 		built. Otherwise, it applies to the whole session and becomes
 * 		the session bandwidth information for the current
 * 		SDP_Description struct being built.
 *
 * 		It returns true if it can parse the field, false otherwise.
 * 
 */

static int _ParseBandwidthField(
	SDP_Parser *               parser,
	SDP_DescriptionBuilder *   description_builder,
	SDP_Description *          description,
	const char *               field_value)
{
	/* Used to splitup the "b" field: */
	char *copy_of_field;
	char *separator;

	/*
	 * The two substrings in the "b" field, and the bandwidth value as a
	 * long int extracted from the second field:
	 */
	char *bandwidth_modifier;
	char *bandwidth_value;
	long available_bandwidth;

	int rv;



	/* Copy the "b" field: */
	copy_of_field = SDP_StrDup(field_value);
	if (copy_of_field == NULL)
	{
		SDP_RaiseFatalError(
			SDP_ERR_OUT_OF_MEMORY,
			"Couldn't allocate memory to copy and parse \"b\" %s "
			"on line %d: %s.",
			SDP_GetFieldTypeDescription('b'),
			SDP_GetCurrentLineNumber(parser),
			SDP_OS_ERROR_STRING
		);
		return SDP_FAILURE;
	}



	/* Extract the bandwidth modifier and value subfields: */
	separator = strchr(copy_of_field, ':');
	if (separator == NULL)
	{
		SDP_Destroy(copy_of_field);

		return SDP_RaiseNonFatalError(
			SDP_ERR_MALFORMED_B_FIELD,
			"Line %d contains a malformed \"b\" %s and could not "
			"be parsed.",
			SDP_GetCurrentLineNumber(parser),
			SDP_GetFieldTypeDescription('b')
		);
	}
	bandwidth_modifier = copy_of_field;
	bandwidth_value    = separator + 1;
	*separator         = '\0';



	/* Make sure the bandwidth modifier is valid: */
	if (strcmp(bandwidth_modifier, "CT") != 0
		&& strcmp(bandwidth_modifier, "AT") != 0
		&& strncmp(bandwidth_modifier, "X-", 2) != 0)
	{
		SDP_Destroy(copy_of_field);

		return SDP_RaiseNonFatalError(
			SDP_ERR_MALFORMED_B_FIELD,
			"Line %d contains a malformed \"b\" %s and could not "
			"be parsed. The conference total is not \"CT\" or "
			"\"AT\" and isn't in the valid \"X-...\" extension "
			"format.",
			SDP_GetCurrentLineNumber(parser),
			SDP_GetFieldTypeDescription('b')
		);
	}

	/* Try to extract the number from the bandwidth value subfield: */
	available_bandwidth = strtol(bandwidth_value, (char **) NULL, 10);
	if (available_bandwidth == 0
		&& (available_bandwidth == EINVAL
			|| available_bandwidth == ERANGE))
	{
		SDP_Destroy(copy_of_field);

		return SDP_RaiseNonFatalError(
			SDP_ERR_MALFORMED_B_FIELD,
			"Line %d contains a malformed \"b\" %s and could not "
			"be parsed. The available bandwidth value is invalid: "
			"%s.",
			SDP_GetCurrentLineNumber(parser),
			SDP_GetFieldTypeDescription('b'),
			SDP_OS_ERROR_STRING
		);
	}



	if (description_builder->state == BUILDING_MEDIA_DESCRIPTION_STRUCTS)
		rv = SDP_SetMediaBandwidth(
			description_builder->current_media_description,
			bandwidth_modifier,
			available_bandwidth
		);
	else
		rv = SDP_SetBandwidth(
			description,
			bandwidth_modifier,
			available_bandwidth
		);

	SDP_Destroy(copy_of_field);

	return rv;
}





/*******************************************************************************
 *
 * 	Name
 * 		_ParseSessionTimeField(
 * 			parser, description_builder, description, field_value
 * 		);
 *
 * 	Purpose
 * 		Tries to parse a "t" field and extract the starting time and
 * 		ending time of the session. It converts both values from NTP
 * 		format to POSIX time_t format, then creates a new
 * 		SDP_SessionPlayTime struct with those values and adds it to the
 * 		linked list of time structs in the current SDP_Description
 * 		struct being built.
 *
 * 		It returns true if it can parse the field, false otherwise.
 * 
 */

static int _ParseSessionTimeField(
	SDP_Parser *               parser,
	SDP_DescriptionBuilder *   description_builder,
	SDP_Description *          description,
	const char *               field_value)
{
	/* the raw start/end time values: */
	unsigned long raw_start_time;
	unsigned long raw_end_time;

	/*
	 * The start and end times of the session converted from NTP format to
	 * POSIX time_t format:
	 */
	time_t start_time;
	time_t end_time;

	int rv;



	/*
	 * Set the state for this time field and the "r" fields after it, and
	 * allocate a new session time struct to fill:
	 */
	description_builder->state = BUILDING_SESSION_TIME_STRUCTS;
	description_builder->current_session_play_time =
		SDP_NewSessionPlayTime();
	if (description_builder->current_session_play_time == NULL)
	{
		SDP_RaiseFatalError(
			SDP_ERR_OUT_OF_MEMORY,
			"Couldn't allocate memory to copy and parse \"t\" %s "
			"on line %d: %s.",
			SDP_GetFieldTypeDescription('t'),
			SDP_GetCurrentLineNumber(parser),
			SDP_OS_ERROR_STRING
		);
		return SDP_FAILURE;
	}

	/* Add it to the list of time structs: */
	SDP_AddSessionPlayTime(
		description, description_builder->current_session_play_time
	);



	/* Try to extract the session start time and stop time in NTP format: */
	rv = sscanf(field_value, "%lu %lu", &raw_start_time, &raw_end_time);
	if (rv == EOF)
		return SDP_RaiseNonFatalError(
			SDP_ERR_MALFORMED_T_FIELD,
			"Line %d contains a malformed \"t\" %s and could not "
			"be parsed. One or both of the NTP time values were "
			"missing or invalid: %s.",
			SDP_GetCurrentLineNumber(parser),
			SDP_GetFieldTypeDescription('t'),
			SDP_OS_ERROR_STRING
		);

	/* Convert both to time_t format: */
	start_time = NETWORK_TIME_TO_OS_TIME(raw_start_time);
	end_time   = NETWORK_TIME_TO_OS_TIME(raw_end_time);



	SDP_SetStartTime(
		description_builder->current_session_play_time, start_time
	);
	SDP_SetEndTime(
		description_builder->current_session_play_time, end_time
	);

	return SDP_SUCCESS;
}





/*******************************************************************************
 *
 * 	Name
 * 		_ParseRepeatTimeField(
 * 			parser, description_builder, description, field_value
 * 		);
 *
 * 	Purpose
 * 		Tries to parse an "r" field and extract the repeat interval,
 * 		active duration, and any repeat offsets, then creates a new
 * 		SDP_RepeatTime struct with those values and adds it to the
 * 		linked list of repeat time structs in the current
 * 		SDP_SessionPlayTime struct being built.
 *
 * 		It returns true if it can parse the field, false otherwise.
 * 
 */

static int _ParseRepeatTimeField(
	SDP_Parser *               parser,
	SDP_DescriptionBuilder *   description_builder,
	SDP_Description *          description,
	const char *               field_value)
{
	/* These are used to split up and parse the "r" field: */
	char *copy_of_field;
	char *c;
	char *subfield;

	unsigned long repeat_interval;
	unsigned long active_duration;
	unsigned long *repeat_offsets = NULL;
	int total_offsets             = 0;

	char error_string[64];

	int rv;



	/*
	 * Make sure we've got at least one "t" field behind us before trying
	 * to parse this "r" field:
	 */
	if (description_builder->state != BUILDING_SESSION_TIME_STRUCTS)
	{
		return SDP_RaiseNonFatalError(
			SDP_ERR_FIELDS_OUT_OF_SEQUENCE,
			"Line %d contains an \"r\" %s that's out of sequnce; "
			"\"r\" fields should only show up right after either "
			"a \"t\" field or another \"r\" field.",
			SDP_GetCurrentLineNumber(parser),
			SDP_GetFieldTypeDescription('r')
		);
	}

	/* Copy the "r" field to manipulate it: */
	copy_of_field = SDP_StrDup(field_value);
	if (copy_of_field == NULL)
	{
		SDP_RaiseFatalError(
			SDP_ERR_OUT_OF_MEMORY,
			"Couldn't allocate memory to copy and parse \"r\" %s "
			"on line %d: %s.",
			SDP_GetFieldTypeDescription('r'),
			SDP_GetCurrentLineNumber(parser),
			SDP_OS_ERROR_STRING
		);
		return SDP_FAILURE;
	}

	/* Get the repeat interval: */
	c = copy_of_field;
	subfield = SDP_StrSep(&c, " ");
	rv = _GetOffsetInSeconds(
		subfield, &repeat_interval, error_string, sizeof(error_string)
	);
	if (SDP_FAILED(rv))
	{
		SDP_Destroy(copy_of_field);

		return SDP_RaiseNonFatalError(
			SDP_ERR_MALFORMED_R_FIELD,
			"Line %d contains a malformed \"r\" %s and could not "
			"be parsed. The repeat interval %s.",
			SDP_GetCurrentLineNumber(parser),
			SDP_GetFieldTypeDescription('r'),
			error_string
		);
	}

	/* Get the active duration: */
	subfield = SDP_StrSep(&c, " ");
	rv = _GetOffsetInSeconds(
		subfield, &repeat_interval, error_string, sizeof(error_string)
	);
	if (SDP_FAILED(rv))
	{
		SDP_Destroy(copy_of_field);

		return SDP_RaiseNonFatalError(
			SDP_ERR_MALFORMED_R_FIELD,
			"Line %d contains a malformed \"r\" %s and could not "
			"be parsed. The active duration %s.",
			SDP_GetCurrentLineNumber(parser),
			SDP_GetFieldTypeDescription('r'),
			error_string
		);
	}

	subfield = SDP_StrSep(&c, " ");
	if (subfield)
	{
		int i;

		/* Figure out how many other offsets there are: */
		total_offsets = 1 + SDP_StrCount(c, ' ');

		/* Allocate the array of offsets: */
		repeat_offsets = (unsigned long *) SDP_Allocate(
			sizeof(unsigned long) * total_offsets
		);
		if (repeat_offsets == NULL)
		{
			SDP_Destroy(copy_of_field);

			SDP_RaiseFatalError(
				SDP_ERR_OUT_OF_MEMORY,
				"Couldn't allocate memory to "
				"copy and parse repeat offsets from the \"r\" "
				"%s on line %d: %s.",
				SDP_GetFieldTypeDescription('r'),
				SDP_GetCurrentLineNumber(parser),
				SDP_OS_ERROR_STRING
			);
			return SDP_FAILURE;
		}

		for (i = 0; subfield && *subfield; ++i)
		{
			rv = _GetOffsetInSeconds(
				subfield,
				&repeat_offsets[i],
				error_string,
				sizeof(error_string)
			);
			if (SDP_FAILED(rv))
			{
				SDP_Destroy(copy_of_field);
				SDP_Destroy(repeat_offsets);

				return SDP_RaiseNonFatalError(
					SDP_ERR_MALFORMED_R_FIELD,
					"Line %d contains a malformed \"r\" "
					"%s and could not be parsed. Repeat "
					"offset number %d %s.",
					SDP_GetCurrentLineNumber(parser),
					SDP_GetFieldTypeDescription('r'),
					i,
					error_string
				);
			}

			subfield = SDP_StrSep(&c, " ");
		}
	}



	rv = SDP_AddNewRepeatTime(
		description_builder->current_session_play_time,
		repeat_interval,
		active_duration,
		repeat_offsets,
		total_offsets
	);

	SDP_Destroy(copy_of_field);

	if (repeat_offsets)
		SDP_Destroy(repeat_offsets);

	return rv;
}





/*******************************************************************************
 *
 * 	Name
 * 		_ParseZoneAdjustmentsField(
 * 			parser, description_builder, description, field_value
 * 		);
 *
 * 	Purpose
 * 		Tries to parse a "z" field and extract each time and
 * 		corresponding adjustment to make at that time. It stores them
 * 		as the zone adjustments for the current SDP_Description struct
 * 		being built.
 *
 * 		It returns true if it can parse the field, false otherwise.
 * 
 */

static int _ParseZoneAdjustmentsField(
	SDP_Parser *               parser,
	SDP_DescriptionBuilder *   description_builder,
	SDP_Description *          description,
	const char *               field_value)
{
	/* Used to chop up the "z" field into substrings: */
	char *copy_of_field;
	char *c;

	/* Stores pointers to each time/adjustment pair: */ 
	char *time;
	char *adjustment;

	int rv;



	/* Copy the "z" field so we can manipulate it: */
	copy_of_field = SDP_StrDup(field_value);
	if (copy_of_field == NULL)
	{
		SDP_RaiseFatalError(
			SDP_ERR_OUT_OF_MEMORY,
			"Couldn't allocate memory to copy and parse \"z\" %s "
			"on line %d: %s.",
			SDP_GetFieldTypeDescription('z'),
			SDP_GetCurrentLineNumber(parser),
			SDP_OS_ERROR_STRING
		);
		return SDP_FAILURE;
	}

	/* Get the first zone adjustment: */
	c = copy_of_field;
	time       = SDP_StrSep(&c, " ");
	adjustment = SDP_StrSep(&c, " ");

	/* Now iterate over them: */
	while ((time && *time) && (adjustment && *adjustment))
	{
		long raw_time_value;
		long offset_in_seconds;
		char error_string[64];

		/* Get the raw time value of when the change will happen: */
		raw_time_value = strtoul(time, (char **) NULL, 10);
		if (raw_time_value == 0
			&& SDP_OS_ERROR_CODE == EINVAL
			&& SDP_OS_ERROR_CODE == ERANGE)
		{
			SDP_Destroy(copy_of_field);

			return SDP_RaiseNonFatalError(
				SDP_ERR_MALFORMED_Z_FIELD,
				"Line %d contains a malformed \"z\" %s and "
				"could not be parsed. A time value is "
				"invalid: %d.",
				SDP_GetCurrentLineNumber(parser),
				SDP_GetFieldTypeDescription('z'),
				SDP_OS_ERROR_CODE
			);
		}

		/* Get the time change to occur: */
		rv = _GetOffsetInSeconds(
			adjustment,
			&offset_in_seconds,
			error_string,
			sizeof(error_string)
		);
		if (SDP_FAILED(rv))
		{
			SDP_Destroy(copy_of_field);

			return SDP_RaiseNonFatalError(
				SDP_ERR_MALFORMED_Z_FIELD,
				"Line %d contains a malformed \"z\" %s and "
				"could not be parsed. A time zone adjustment "
				"value %s.",
				SDP_GetCurrentLineNumber(parser),
				SDP_GetFieldTypeDescription('z'),
				error_string
			);
		}

		/* Add it to the list of zone adjustments: */
		rv = SDP_AddNewZoneAdjustment(
			description,
			NETWORK_TIME_TO_OS_TIME(raw_time_value),
			offset_in_seconds
		);
		if (SDP_FAILED(rv))
		{
			SDP_Destroy(copy_of_field);
			return SDP_FAILURE;
		}

		/* Get the next time and corresponding adjustment: */
		time       = SDP_StrSep(&c, " ");
		adjustment = SDP_StrSep(&c, " ");
	}

	SDP_Destroy(copy_of_field);

	return SDP_SUCCESS;
}





/*******************************************************************************
 *
 * 	Name
 * 		_ParseEncryptionField(
 * 			parser, description_builder, description, field_value
 * 		);
 *
 * 	Purpose
 * 		Tries to parse a "k" field and extract the encryption method
 * 		and optional encryption key.
 * 
 * 		It returns true if it can parse the field, false otherwise.
 * 
 */

static int _ParseEncryptionField(
	SDP_Parser *               parser,
	SDP_DescriptionBuilder *   description_builder,
	SDP_Description *          description,
	const char *               field_value)
{
	/* Used to splitup the "k" field: */
	char *copy_of_field;
	char *c;

	/*
	 * Pointers to the encryption method and key substrings in
	 * copy_of_field:
	 */
	char *encryption_method;
	char *encryption_key;

	int rv;



	/* Copy the "k" field: */
	copy_of_field = SDP_StrDup(field_value);
	if (copy_of_field == NULL)
	{
		SDP_RaiseFatalError(
			SDP_ERR_OUT_OF_MEMORY,
			"Couldn't allocate memory to copy and parse \"k\" %s "
			"on line %d: %s.",
			SDP_GetFieldTypeDescription('k'),
			SDP_GetCurrentLineNumber(parser),
			SDP_OS_ERROR_STRING
		);
		return SDP_FAILURE;
	}

	/* Extract the method and key: */
	c = copy_of_field;
	encryption_method = SDP_StrSep(&c, ":");
	encryption_key    = c;

	/*
	 * Now figure out whether this "k" field belongs to the entire session
	 * description or to just one media description, and add it to the
	 * appropriate struct accordingly:
	 */
	if (description_builder->state == BUILDING_MEDIA_DESCRIPTION_STRUCTS)
		rv = SDP_SetMediaEncryption(
			description_builder->current_media_description,
			encryption_method,
			encryption_key
		);
	else
		rv = SDP_SetEncryption(
			description,
			encryption_method,
			encryption_key
		);

	SDP_Destroy(copy_of_field);

	return rv;
}





/*******************************************************************************
 *
 * 	Name
 * 		_ParseAttributeField(
 * 			parser, description_builder, description, field_value
 * 		);
 *
 * 	Purpose
 * 		Tries to parse an "a" field and extract the attribute name and
 * 		optional attribute value. If a media description is currently
 * 		being parsed, then the attribute belongs only to that media and
 * 		gets added to the current SDP_MediaDescription struct being
 * 		built. Otherwise, it belongs to the entire session and gets
 * 		added to the current SDP_Description struct being built.
 * 
 * 		It returns true if it can parse the field, false otherwise.
 * 
 */

static int _ParseAttributeField(
	SDP_Parser *               parser,
	SDP_DescriptionBuilder *   description_builder,
	SDP_Description *          description,
	const char *               field_value)
{
	/* Used to copy and split the attribute field: */
	char *copy_of_field;
	char *c;

	/*
	 * Pointers to the name and value substrings within the
	 * copy_of_field:
	 */
	char *attribute_name;
	char *attribute_value;

	int rv;



	/* Copy the "a" field: */
	copy_of_field = SDP_StrDup(field_value);
	if (copy_of_field == NULL)
	{
		SDP_RaiseFatalError(
			SDP_ERR_OUT_OF_MEMORY,
			"Couldn't allocate memory to copy and parse \"a\" %s "
			"on line %d: %s.",
			SDP_GetFieldTypeDescription('a'),
			SDP_GetCurrentLineNumber(parser),
			SDP_OS_ERROR_STRING
		);
		return SDP_FAILURE;
	}

	/* Extract the name and value: */
	c = copy_of_field;
	attribute_name  = SDP_StrSep(&c, ":");
	attribute_value = c;

	/*
	 * Now figure out whether this "a" field belongs to the entire session
	 * description or to just one media description, and add it to the
	 * appropriate struct accordingly:
	 */
	if (description_builder->state == BUILDING_MEDIA_DESCRIPTION_STRUCTS)
		rv = SDP_AddNewMediaAttribute(
			description_builder->current_media_description,
			attribute_name,
			attribute_value
		);
	else
		rv = SDP_AddNewAttribute(
			description,
			attribute_name,
			attribute_value
		);

	SDP_Destroy(copy_of_field);

	return rv;
}





/*******************************************************************************
 *
 * 	Name
 * 		_ParseMediaDescriptionField(
 * 			parser, description_builder, description, field_value
 * 		);
 *
 * 	Purpose
 * 		Tries to parse an "m" field and extract the media type, media
 * 		port, transport protocol, and media formats. It creates a new
 * 		SDP_MediaDescription struct with these values and adds it to
 * 		the media description linked list in the current
 * 		SDP_Description struct built.
 * 
 * 		It returns true if it can parse the field, false otherwise.
 * 
 */

static int _ParseMediaDescriptionField(
	SDP_Parser *               parser,
	SDP_DescriptionBuilder *   description_builder,
	SDP_Description *          description,
	const char *               field_value)
{
	/* Used to copy the "m" field and split it up into substrings: */
	char *copy_of_field;
	char *c;

	/* Pointers to each substring in copy_of_field: */
	char *media_type;
	char *port_field;
	char *transport_protocol;
	char *media_formats;

	/* The "/" separated values extracted out of the port field: */
	unsigned short media_port  = 0;
	unsigned short total_ports = 0;

	/* Used to store the names of any missing fields separated by commas: */
	char missing_fields[128] = "";

	int rv;



	/* Adjust the builder state: */
	description_builder->state = BUILDING_MEDIA_DESCRIPTION_STRUCTS;

	/* Allocate memory for a new media description struct: */
	description_builder->current_media_description =
		SDP_NewMediaDescription();
	if (description_builder->current_media_description == NULL)
	{
		SDP_RaiseFatalError(
			SDP_ERR_OUT_OF_MEMORY,
			"Couldn't allocate memory to copy and parse \"m\" %s "
			"on line %d: %s.",
			SDP_GetFieldTypeDescription('m'),
			SDP_GetCurrentLineNumber(parser),
			SDP_OS_ERROR_STRING
		);
		return SDP_FAILURE;
	}

	/* Add it to the list of media description structs: */
	SDP_AddMediaDescription(
		description, description_builder->current_media_description
	);



	/* Copy the "m" field so we can parse it: */
	copy_of_field = SDP_StrDup(field_value);
	if (copy_of_field == NULL)
	{
		SDP_RaiseFatalError(
			SDP_ERR_OUT_OF_MEMORY,
			"Couldn't allocate memory to parse media "
			"description: %s.",
			SDP_OS_ERROR_STRING
		);
		return SDP_FAILURE;
	}

	/*
	 * Extract the meida type and then the port number(s), then the
	 * transport protocol and media formats:
	 */
	c = copy_of_field;
	media_type         = SDP_StrSep(&c, " ");
	_CheckForMissingField(missing_fields, media_type, "media type field");
	port_field         = SDP_StrSep(&c, " ");
	_CheckForMissingField(missing_fields, port_field, "port field");
	transport_protocol = SDP_StrSep(&c, " ");
	_CheckForMissingField(
		missing_fields, transport_protocol, "transport protocol field"
	);
	media_formats      = c;

	/* Check for missing fields: */
	if (ARE_REQUIRED_FIELDS_MISSING(missing_fields))
	{
		SDP_Destroy(copy_of_field);

		return SDP_RaiseNonFatalError(
			SDP_ERR_MALFORMED_M_FIELD,
			"Line %d contains a malformed \"m\" %s that could not "
			"be parsed because the following required subfields "
			"are missing: %s.",
			SDP_GetCurrentLineNumber(parser),
			SDP_GetFieldTypeDescription('m'),
			missing_fields
		);
	}

	/* Try to parse the port field: */
	if (strchr(port_field, '/'))
		rv = sscanf(port_field, "%hu/%hu", &media_port, &total_ports);
	else
		rv = sscanf(port_field, "%hu", &media_port);

	if (SDP_FAILED(rv))
	{
		SDP_Destroy(copy_of_field);

		return SDP_RaiseNonFatalError(
			SDP_ERR_MALFORMED_M_FIELD,
			"Line %d contains a malformed \"m\" %s and it could "
			"not be parsed: %s.",
			SDP_GetCurrentLineNumber(parser),
			SDP_GetFieldTypeDescription('m'),
			SDP_OS_ERROR_STRING
		);
	}



	/* Add them all to the new media description struct: */
	rv = SDP_SetMediaType(
		description_builder->current_media_description, media_type
	);
	if (SDP_FAILED(rv))
	{
		SDP_Destroy(copy_of_field);
		return SDP_FAILURE;
	}

	SDP_SetMediaPort(
		description_builder->current_media_description, media_port
	);
	SDP_SetTotalMediaPorts(
		description_builder->current_media_description, total_ports
	);

	rv = SDP_SetMediaTransportProtocol(
		description_builder->current_media_description,
		transport_protocol
	);
	if (SDP_FAILED(rv))
	{
		SDP_Destroy(copy_of_field);
		return SDP_FAILURE;
	}

	rv = SDP_SetMediaFormats(
		description_builder->current_media_description, media_formats
	);
	if (SDP_FAILED(rv))
	{
		SDP_Destroy(copy_of_field);
		return SDP_FAILURE;
	}

	SDP_Destroy(copy_of_field);

	return SDP_SUCCESS;
}





/*
 * This probably ought to be up higher, but we stick it down here tp avoid
 * having to prototype each and every field parsing function.
 */

static void _InitializeFieldParsersTable(void)
{
	if (_field_parsers_table_initialized)
		return;

	/* Set each function pointer to NULL: */
	memset(_field_parsers_table, 0, sizeof(_field_parsers_table));

	/*
	 * ...then replace the NULLs with pointers to the appropriate handlers
	 * for each field:
	 */
	_field_parsers_table['v'] = _ParseProtocolVersionField;
	_field_parsers_table['o'] = _ParseOwnerField;
	_field_parsers_table['s'] = _ParseSessionNameField;
	_field_parsers_table['i'] = _ParseInformationField;
	_field_parsers_table['u'] = _ParseURIField;
	_field_parsers_table['e'] = _ParseEmailContactField;
	_field_parsers_table['p'] = _ParsePhoneContactField;
	_field_parsers_table['c'] = _ParseConnectionField;
	_field_parsers_table['b'] = _ParseBandwidthField;
	_field_parsers_table['t'] = _ParseSessionTimeField;
	_field_parsers_table['r'] = _ParseRepeatTimeField;
	_field_parsers_table['z'] = _ParseZoneAdjustmentsField;
	_field_parsers_table['k'] = _ParseEncryptionField;
	_field_parsers_table['a'] = _ParseAttributeField;
	_field_parsers_table['m'] = _ParseMediaDescriptionField;
	
	_field_parsers_table_initialized = 1;
}





static int _CheckForMissingField(
	char *         missing_fields,
	const char *   field,
	const char *   field_name)
{
	if (field && *field)
		return SDP_SUCCESS;

	if (*missing_fields)
		SDP_StrLCat(missing_fields, ", ", sizeof(missing_fields));

	SDP_StrLCat(missing_fields, field_name, sizeof(missing_fields));

	return SDP_FAILURE;
}





static int _ParseContactField(
	char *    field_to_parse,
	char **   required_value,
	char **   optional_value,
	int *     error)
{
	/* The separator (either "<" or "("): */
	char *separator;

	/* The terminator of the optional second field (either ">" or ")"): */
	char *terminator;

	/*
	 * Each email address field contains either the address optionally
	 * followed by the person's real name in "(" ")" characters, or the
	 * person's real name and then the address in "<" ">" characters.
	 * Similarly, each phone number field contains either the phone number
	 * optionally followed by the person's real name in "(" ")" characters,
	 * or the person's real name and then the phone number in "<" ">"
	 * characters.
	 * 
	 * So we check to see which style the field is in then parse it
	 * appropriately:
	 */
	separator = strchr(field_to_parse, '(');
	if (separator)
	{
		*required_value = field_to_parse;
		*optional_value = separator + 1;

		/*
		 * Strip whitespace from the front and back of the first field:
		 */
		while (isspace(**required_value))
			++*required_value;
		if (separator > field_to_parse)
		{
			while (*(separator - 1) != '\0'
				&& isspace(*(separator - 1)))
			{
				--separator;
			}
		}

		/* NULL terminate both fields: */
		*separator = '\0';
		terminator = strchr(*optional_value, ')');
		if (terminator == NULL)
		{
			*error = ERR_MISSING_PARENTHESIS_TERMINATOR;
			return SDP_FAILURE;
		}
		*terminator = '\0';
	}
	else
	{
		separator = strchr(field_to_parse, '<');
		if (separator)
		{
			*optional_value = field_to_parse;
			*required_value = separator + 1;

			/*
			 * Strip whitespace from the front and back of the
			 * first field:
			 */
			while (isspace(**optional_value))
				++*optional_value;
			if (separator > field_to_parse)
			{
				while (*(separator - 1) != '\0'
					&& isspace(*(separator - 1)))
				{
					--separator;
				}
			}

			/* NULL terminate both fields: */
			*separator = '\0';
			terminator = strchr(*required_value, '>');
			if (terminator == NULL)
			{
				*error = ERR_MISSING_ANGLE_BRACKET_TERMINATOR;
				return SDP_FAILURE;
			}
			*terminator = '\0';
		}
		else
		{
			/* Default to just the required field: */
			*required_value = field_to_parse;
			*optional_value = NULL;
		}
	}

	if (!*required_value || **required_value == '\0')
	{
		*error = ERR_MISSING_REQUIRED_FIELD;
		return SDP_FAILURE;
	}

	return SDP_SUCCESS;
}





/*
 * RFC 2327 allows descriptions to specify offsets from some NTP time value as
 * either a number of seconds or as a number followed by "m" for minutes, "h"
 * for hours, or "d" for days. This extracts a time offset from a string in any
 * of those formats and converts the number to seconds if needed:
 */

#define SECONDS_IN_DAY    (60 * 60 * 24)
#define SECONDS_IN_HOUR   (60 * 60)
#define SECONDS_IN_MINUTE (60)

static int _GetOffsetInSeconds(
	const char *   string,
	long *         offset_in_seconds,
	char *         error_string,
	size_t         error_string_size)
{
	long offset;
	char *unit_of_measurement;

	/* Make sure there's some offset to parse first: */
	if (string == NULL || *string == '\0')
	{
		*offset_in_seconds = 0;
		snprintf(error_string, error_string_size, "is empty");
		return SDP_FAILURE;
	}

	/* Try to extract the offset number: */
	offset = strtol(string, &unit_of_measurement, 10);
	if (offset == 0
		&& (SDP_OS_ERROR_CODE == EINVAL
			|| SDP_OS_ERROR_CODE == ERANGE))
	{
		*offset_in_seconds = 0;
		snprintf(
			error_string,
			error_string_size,
			"is invalid: %s",
			SDP_OS_ERROR_STRING
		);
		return SDP_FAILURE;
	}

	/* Now apply the appropriate quantifier to it: */ 
	if (*unit_of_measurement)
	{
		switch (*unit_of_measurement)
		{
			case 'd':
				*offset_in_seconds = offset * SECONDS_IN_DAY;
			case 'h':
				*offset_in_seconds = offset * SECONDS_IN_HOUR;
			case 'm':
				*offset_in_seconds = offset * SECONDS_IN_MINUTE;
			case 's':
				*offset_in_seconds = offset;
			default:
				*offset_in_seconds = 0;
				snprintf(
					error_string,
					error_string_size,
					"has a bad unit of measurement: %c. "
					"It should be \"d\", \"h\", \"m\", or "
					"\"s\" instead",
					*unit_of_measurement
				);
				return SDP_FAILURE;
		}
	}
	else
	{
		*offset_in_seconds = offset;
	}

	return SDP_SUCCESS;
}
