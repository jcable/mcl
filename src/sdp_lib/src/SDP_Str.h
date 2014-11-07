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

#ifndef SDP_STR_INCLUDED
#define SDP_STR_INCLUDED

#ifdef __cplusplus
extern "C" {
#endif

#include <stdarg.h>
#include <sys/types.h>





/* A struct we use to keep track of dynamically allocated strings: */
typedef struct {
	/* Pointer to the dynamically buffer that contains the string: */
	char *buffer;

	/* The size of the dynamically allocated buffer: */
	size_t size;

	/*
	 * The length of the string in the buffer (not counting the terminating
	 * NULL character, of course):
	 */
	unsigned long length;
} SDP_Str;



/* Accessor macros for the struct members: */
#define SDP_GetStrBuffer(_str_) ((_str_).buffer)
#define SDP_GetStrSize(_str_)   ((_str_).size)
#define SDP_GetStrLength(_str_) ((_str_).length)

/*
 * Has the string been initialized yet (has the "buffer" member been
 * allocated?):
 */
#define SDP_IsStrInitialized(_str_) ((_str_).buffer ? 1 : 0)

/*
 * This returns the the number of bytes in use within an SDP_Str struct's
 * "buffer" member (basically, the characters that make up the string and the
 * terminating NULL character:
 */
#define SDP_StrBytesInUse(_str_) \
	(SDP_IsStrInitialized(_str_) ? (_str_).length + 1 : 0)

/*
 * This returns the number of available bytes within an SDP_Str struct's
 * "buffer" member (that is, bytes which haven't been written to yet):
 */
#define SDP_StrBytesAvailable(_str_)                  \
	(SDP_IsStrInitialized(_str_)                  \
		? (_str_).size - ((_str_).length + 1) \
		: 0)

/* Gets a pointer to the last character: */
#define SDP_GetPtrToEndOfStr(_str_) ((_str_).buffer + (_str_).length)



/*******************************************************************************
 * 
 * 	Name
 * 		SDP_CopyToStr(destination, string_to_copy)
 * 
 * 	Purpose
 * 		Copies "string_to_copy" to the buffer of the SDP_Str struct
 * 		pointed to by "destination", growing it if necessary to
 * 		accommodate "string_to_copy". It returns true if it can
 * 		successfully copy the string, and false if any needed memory
 * 		(re)allocation fails.
 * 
 * 	Parameters
 * 		destination    - Pointer to the SDP_Str struct to copy to.
 * 		string_to_copy - The string to copy.
 * 
 */
extern int SDP_CopyToStr(
	SDP_Str *      destination,
	const char *   string_to_copy
);

/*******************************************************************************
 * 
 * 	Name
 * 		SDP_CatToString(destination, string_to_cat)
 * 
 * 	Purpose
 * 		Concatenates "string_to_cat" to the buffer of the SDP_Str
 * 		struct pointed to by "destination", growing it if necessary to
 * 		accommodate "string_to_cat". It returns true if it can
 * 		successfully concatenate the string, and false if any needed
 * 		memory (re)allocation fails.
 * 
 * 	Parameters
 * 		destination   - Pointer to the SDP_Str struct to concatenate to.
 * 		string_to_cat - The string to concatenate.
 * 
 */
extern int SDP_CatToString(
	SDP_Str *      destination,
	const char *   string_to_cat
);

/*******************************************************************************
 * 
 * 	Name
 * 		SDP_CopyToStrUsingFormat(
 * 			destination, bytes_needed, format, args
 * 		)
 * 
 * 	Purpose
 * 		Copies a formatted string to the buffer of the SDP_Str struct
 * 		pointed to by "destination". Works like vsnprintf() except it
 * 		grows the buffer to accommodate how ever many bytes your
 * 		formatted string needs, as specified by "bytes_needed". Returns
 * 		true if it can copy the formatted string, false if memory
 * 		allocation fails.
 * 
 * 	Parameters
 * 		destination  - Pointer to the SDP_Str struct to copy to.
 * 		bytes_needed - The number of bytes your formatted string will
 * 		               take up. If it ends up taking up more than this,
 * 		               then the string will be cutoff.
 * 		format       - The format string.
 * 		args         - Pointer to a va_arg list.
 *
 */
extern int SDP_CopyToStrUsingFormat(
	SDP_Str *      destination,
	size_t         bytes_needed,
	const char *   format,
	va_list        args
);

/*******************************************************************************
 * 
 * 	Name
 * 		SDP_CatToStrUsingFormat(
 * 			destination, bytes_needed, format, args
 * 		)
 * 
 * 	Purpose
 * 		Concatenates a formatted string to the buffer of the SDP_Str
 * 		struct pointed to by "destination". Works sort of like
 * 		vsnprintf(); it grows the buffer in the SDP_Str struct to
 * 		accommodate how ever many bytes your formatted string needs, as
 * 		specified by "bytes_needed". Returns true if it can concatenate
 * 		the formatted string, false if memory reallocation fails.
 * 
 * 	Parameters
 * 		destination  - Pointer to the SDP_Str struct to concatenate to.
 * 		bytes_needed - The number of bytes your formatted string will
 * 		               take up. If it ends up taking up more than this,
 * 		               then the string will be cutoff.
 * 		format       - The format string.
 * 		args         - Pointer to a va_arg list.
 * 
 */
extern int SDP_CatToStrUsingFormat(
	SDP_Str *      destination,
	size_t         bytes_needed,
	const char *   format,
	va_list        args
);

/*******************************************************************************
 * 
 * 	Name
 * 		SDP_DestroyStrBuffer(string)
 * 
 * 	Purpose
 * 		Destroys a dynamically allocated string buffer in an SDP_Str
 * 		struct; returning its memory back to the operating system, and
 * 		also sets the "length" and "size" members if the struct to zero.
 * 
 * 	Parameters
 * 		string - A pointer to an SDP_Str struct whose buffer will be
 * 		         destroyed.
 * 
 */
extern void SDP_DestroyStrBuffer(SDP_Str *string);



/*
 * The rest are generic string functions that don't operate specifically on
 * SDP_Str structs:
 */

/*******************************************************************************
 * 
 * 	Name
 * 		SDP_StrDup(string)
 * 
 * 	Purpose
 * 		This function works just like the C standard library function
 * 		strdup(), only instead of calling malloc(), it calls
 * 		SDP_Allocate().
 * 
 * 	Parameters
 * 		string - The string to duplicate.
 * 
 */
extern char *SDP_StrDup(const char *string);

/*******************************************************************************
 * 
 * 	Name
 * 		SDP_StrCount(string, c)
 * 
 * 	Purpose
 * 		This function searches "string" for occurrances of
 * 		"c" in it and returns the total number of occurrances.
 * 
 * 	Parameters
 * 		string - The string to search in.
 * 		c      - The character to search for.
 * 
 */
extern unsigned long SDP_StrCount(const char *string, char c);

/*******************************************************************************
 * 
 * 	Name
 * 		SDP_StrSep(string, delimiters)
 * 
 * 	Purpose
 * 		This function works just like Linux/BSD strsep(). It's provided
 * 		here under a different name for portability purposes, since
 * 		strsep() is *still* non-standard even after C99.
 * 
 * 	Parameters
 * 		Same as strsep().
 * 
 */
extern char *SDP_StrSep(char **stringp, const char *delim);

/*******************************************************************************
 * 
 * 	Names
 * 		SDP_StrLCopy(dst, src, siz), SDP_StrLCat(dst, src, siz)
 * 
 * 	Purposes
 * 		These two functions are just the OpenBSD strlcpy() and
 * 		strlcat() functions under different names, since they too--like
 * 		strsep()--are non-standard, and will most likely stay that
 * 		way since apparently I'm the only one that needs safe string
 * 		functions.
 * 
 * 	Parameters
 * 		Same as strlcpy() and strlcat().
 * 
 */
extern size_t SDP_StrLCopy(char *dst, const char *src, size_t siz);
extern size_t SDP_StrLCat(char *dst, const char *src, size_t siz);





#ifdef __cplusplus
}
#endif

#endif
