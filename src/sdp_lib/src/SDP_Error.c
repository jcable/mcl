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
#include "SDP_Error.h"
#include "SDP_Utility.h"

/*
 * The size of the global string used to store a description of the last error
 * to occur:
 */
#define ERROR_STRING_SIZE 512







/* The error code of the last error to occur: */
static SDP_Error _last_error = SDP_NO_ERROR;

/* The string containing the error message of the last error to occur: */
static char _last_error_string[ERROR_STRING_SIZE];



/*
 * If this symbol is defined, then these two default error handlers will be
 * registered:
 */
#ifdef SINISTERSDP_REGISTER_DEFAULT_ERROR_HANDLERS

/* Default error handlers: */
static void _DefaultFatalErrorHandler(
	SDP_Error      error_code,
	const char *   error_string
);
static int _DefaultNonFatalErrorHandler(
	SDP_Error      error_code,
	const char *   error_string
);

/* The callback for fatal, non-recoverable errors: */
static SDP_FatalErrorHandler _fatal_error_handler =
	_DefaultFatalErrorHandler;

/* The callback for non-fatal, recoverable errors: */
static SDP_NonFatalErrorHandler _non_fatal_error_handler =
	_DefaultNonFatalErrorHandler;

#else

static SDP_FatalErrorHandler _fatal_error_handler        = NULL;
static SDP_NonFatalErrorHandler _non_fatal_error_handler = NULL;

#endif



/*
 * If some function raises an error, should either of the appropriate error
 * handlers above be invoked?
 */
static int _use_error_handlers = 1;





int SDP_ErrorRaised(void)
{
	return _last_error ? 1 : 0;
}





SDP_Error SDP_GetLastError(void)
{
	return _last_error;
}





const char *SDP_GetLastErrorString(void)
{
	return _last_error_string;
}





void SDP_SetFatalErrorHandler(SDP_FatalErrorHandler handler)
{
	_fatal_error_handler = handler;
}





void SDP_SetNonFatalErrorHandler(SDP_NonFatalErrorHandler handler)
{
	_non_fatal_error_handler = handler;
}





void SDP_UseHandlersForErrors(int use_handlers)
{
	_use_error_handlers = use_handlers ? 1 : 0;
}





void SDP_RaiseFatalError(
	SDP_Error      code,
	const char *   format,
	...)
{
	va_list args;

	SDP_AssertTrue(code);
	SDP_AssertNotNull(format);

	_last_error = code;

	va_start(args, format);
	vsnprintf(
		_last_error_string, sizeof(_last_error_string), format, args
	);
	va_end(args);

	if (_use_error_handlers && _fatal_error_handler)
		(*_fatal_error_handler)(_last_error, _last_error_string);
}





int SDP_RaiseNonFatalError(
	SDP_Error      code,
	const char *   format,
	...)
{	
	va_list args;

	SDP_AssertTrue(code);
	SDP_AssertNotNull(format);

	_last_error = code;

	va_start(args, format);
	vsnprintf(
		_last_error_string, sizeof(_last_error_string), format, args
	);
	va_end(args);

	/*
	 * We use the handler's return value as a flag to tell us whether or
	 * not we should keep going even after encountering an error.
	 *
	 * If there's no registered handler, then just stop:
	 */
	if (_use_error_handlers && _non_fatal_error_handler)
		return (*_non_fatal_error_handler)(
			_last_error, _last_error_string
		);
	else
		return SDP_STOP_AFTER_ERROR;
}





#ifdef SINISTERSDP_REGISTER_DEFAULT_ERROR_HANDLERS

static void _DefaultFatalErrorHandler(
	SDP_Error      error_code,
	const char *   error_string)
{
	fprintf(stderr, "Fatal error: %s\n", error_string);

	/* Exit the entire process: */
	exit(EXIT_FAILURE);
}





static int _DefaultNonFatalErrorHandler(
	SDP_Error      error_code,
	const char *   error_string)
{
	fprintf(stderr, "Error: %s\n", error_string);

	/* Force the caller to exit prematurely: */
	return SDP_STOP_AFTER_ERROR; 
}

#endif
