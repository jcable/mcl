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

#ifndef SDP_ERROR_INCLUDED
#define SDP_ERROR_INCLUDED

#ifdef __cplusplus
extern "C" {
#endif

#include <errno.h>
#include <string.h>

/*
 * We put SDP_OS_ERROR_STRING in parenthesis so we can treat it like a "char *"
 * variable and dereference it (e.g., "*SDP_OS_ERROR_STRING == '!'"):
 */
#define SDP_OS_ERROR_CODE   errno
#define SDP_OS_ERROR_STRING (strerror(SDP_OS_ERROR_CODE))





typedef enum {
	/* No error; nothing bad has happened! */
	SDP_NO_ERROR = 0,

	/* Some sort of error occurred; this is used mostly for testing: */
	SDP_ERR_GENERIC = 1,

	/*
	 * These errors are fatal, non-recoverable, meaning that the function
	 * that riases them must return immediately and cannot continue
	 * execution:
	 */
	SDP_ERR_OUT_OF_MEMORY = 1000,
	SDP_ERR_FILE_OPEN_FAILED,

	/*
	 * These errors are non-fatal, recoverable parser errors; the parser
	 * can either stop or keep going depending on the return value of the
	 * error handler:
	 */
	SDP_ERR_MALFORMED_LINE = 2000,
	SDP_ERR_MALFORMED_V_FIELD,
	SDP_ERR_MALFORMED_O_FIELD,
	SDP_ERR_MALFORMED_E_FIELD,
	SDP_ERR_MALFORMED_P_FIELD,
	SDP_ERR_MALFORMED_C_FIELD,
	SDP_ERR_MALFORMED_B_FIELD,
	SDP_ERR_MALFORMED_T_FIELD,
	SDP_ERR_MALFORMED_R_FIELD,
	SDP_ERR_MALFORMED_Z_FIELD,
	SDP_ERR_MALFORMED_K_FIELD,
	SDP_ERR_MALFORMED_A_FIELD,
	SDP_ERR_MALFORMED_M_FIELD,
	SDP_ERR_INVALID_TYPE_CHARACTER,
	SDP_ERR_MULTIPLE_UNIQUE_FIELDS,
	SDP_ERR_FIELDS_OUT_OF_SEQUENCE
} SDP_Error;

/* Function pointer typedefs for each type of error handler: */
typedef void (*SDP_FatalErrorHandler)(
	SDP_Error      error_code,
	const char *   error_string
);
typedef int (*SDP_NonFatalErrorHandler)(
	SDP_Error      error_code,
	const char *   error_string
);



extern int SDP_ErrorRaised(void);
extern SDP_Error SDP_GetLastError(void);
extern const char *SDP_GetLastErrorString(void);
extern void SDP_SetFatalErrorHandler(SDP_FatalErrorHandler handler);
extern void SDP_SetNonFatalErrorHandler(SDP_NonFatalErrorHandler handler);
extern void SDP_UseHandlersForErrors(int use_handlers);
extern void SDP_RaiseFatalError(
	SDP_Error      code,
	const char *   format,
	...
);
extern int SDP_RaiseNonFatalError(
	SDP_Error      code,
	const char *   format,
	...
);





#ifdef __cplusplus
}
#endif

#endif
