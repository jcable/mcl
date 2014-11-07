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

#ifndef SDP_UTILITY_INCLUDED
#define SDP_UTILITY_INCLUDED

#ifdef __cplusplus
extern "C" {
#endif

#include <stdlib.h>


#if defined WIN32
#define snprintf _snprintf
#define vsnprintf _vsnprintf
#endif

/*
 * The first two macros are used as return values, and the second two are used
 * to test a return value:
 */
#define SDP_FAILURE         0
#define SDP_SUCCESS         1
#define SDP_FAILED(_rv_)    ((_rv_) ? 0 : 1)
#define SDP_SUCCEEDED(_rv_) (SDP_FAILED(_rv_) ? 0 : 1)

/* Are we on Windows? */
#if (defined(__WIN32__) || defined(__WIN32) || defined(__WIN32))
#	define SDP_ON_WINDOWS 
#endif

/*
 * Simple named wrappers around malloc(), realloc() and free(). Feel free to
 * change the replacements with what ever routines you like (for example, the
 * Win32 API GetProcHeap()/HeapAlloc(), HeapReAlloc(), and HeapFree()
 * combination):
 */
#define SDP_Allocate(_bytes_)             malloc(_bytes_)
#define SDP_Reallocate(_memory_, _bytes_) realloc(_memory_, _bytes_)
#define SDP_Destroy(_memory_)             free(_memory_)

/*
 * SinisterSdp uses these assertion macros to ensure things that should never
 * happen, don't happen. If you don't want the added weight in compiled code
 * these statements will add, then define SINISTERSDP_NO_ASSERTIONS when
 * building:
 */
#ifdef SINISTERSDP_NO_ASSERTIONS
#	define SDP_Assert(_expr_)
#	define SDP_AssertNull(_expr_)
#	define SDP_AssertNotNull(_expr_)
#	define SDP_AssertTrue(_expr_)
#	define SDP_AssertFalse(_expr_)
#else
#	define SDP_Assert(_expr_)                                             \
		((!(_expr_))                                                  \
			? SDP_AssertionFailed(                                \
				"Assertion \"%s\" failed. (Assertion failed " \
				"at line %d from %s.",                        \
				#_expr_,                                      \
				__LINE__,                                     \
				__FILE__                                      \
			  )                                                   \
			: (void) 0)
#	define SDP_AssertNull(_expr_)                                     \
		((_expr_)                                                 \
			? SDP_AssertionFailed(                            \
				"\"%s\" should be NULL, but it isn't. "   \
				"(Assertion failed at line %d from %s.)", \
				#_expr_,                                  \
				__LINE__,                                 \
				__FILE__                                  \
			  )                                               \
			: (void) 0)
#	define SDP_AssertNotNull(_expr_)                                  \
		((_expr_ == NULL)                                         \
			? SDP_AssertionFailed(                            \
				"\"%s\" is NULL, and it shouldn't be. "   \
				"(Assertion failed at line %d from %s.)", \
				#_expr_,                                  \
				__LINE__,                                 \
				__FILE__                                  \
			  )                                               \
			: (void) 0)
#	define SDP_AssertTrue(_expr_)                                     \
		((!(_expr_))                                              \
			? SDP_AssertionFailed(                            \
				"\"%s\" is false; it should be true. "    \
				"(Assertion failed at line %d from %s.)", \
				#_expr_,                                  \
				__LINE__,                                 \
				__FILE__                                  \
			  )                                               \
			: (void) 0)
#	define SDP_AssertFalse(_expr_)                                   \
		((_expr_)                                                \
			? SDP_AssertionFailed(                           \
				"\"%s\" is ture; it should be false. "   \
				"(Assertion failed at line %d from %s.", \
				#_expr_,                                 \
				__LINE__,                                \
				__FILE__                                 \
			  )                                              \
			: (void) 0)
#endif

/* Return value macros for handlers: */
#define SDP_CONTINUE_AFTER_ERROR 1;
#define SDP_STOP_AFTER_ERROR     0;



extern void SDP_AssertionFailed(const char *format, ...);
extern int SDP_IsKnownFieldType(char type);
extern char *SDP_GetFieldTypeDescription(char type);



#ifdef __cplusplus
}
#endif

#endif
