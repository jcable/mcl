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
 *-----------------------------------------------------------------------------
 *
 * This file contains definitions for several non-standard string functions
 * used by SinisterSdp.
 * 
 ******************************************************************************/

#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include "SDP_Str.h"
#include "SDP_Utility.h"

#define INITIAL_BUFFER_SIZE 1024







/* Used to initialze the string buffer and grow it after that if needed: */
static int _InitializeStrBuffer(
	SDP_Str *   string,
	size_t      initial_size
);
static int _GrowStrBuffer(
	SDP_Str *   string,
	size_t      size_to_grow_by
);

int SDP_CopyToStr(
	SDP_Str *      destination,
	const char *   string_to_copy)
{
	size_t bytes_needed;
	unsigned long length;
	int rv;

	/*
	 * If destination->buffer has already been allocated and "string" is
	 * a NULL pointer, then we make the string pointed to by
	 * destination->buffer an empty string:
	 */
	if (string_to_copy == NULL)
	{
		if (SDP_IsStrInitialized(*destination))
			strncpy(destination->buffer, "", 1);

		return SDP_SUCCESS;
	}

	length       = strlen(string_to_copy);
	bytes_needed = length + 1; /* for the null terminator. */

	if (!SDP_IsStrInitialized(*destination))
	{
		/* Allocate the destination string's buffer: */
		rv = _InitializeStrBuffer(destination, bytes_needed);
		if (SDP_FAILED(rv))
			return SDP_FAILURE;
	}
	else if (destination->size < bytes_needed)
	{
		/* The current buffer isn't big enough; reallocate it: */
		rv = _GrowStrBuffer(
			destination, bytes_needed - destination->size
		);
		if (SDP_FAILED(rv))
			return SDP_FAILURE;
	}

	/* Copy it, and then NULL terminate it by hand: */
	strncpy(destination->buffer, string_to_copy, length);
	*(destination->buffer + length) = '\0';

	destination->length = length;
	destination->size   = bytes_needed;

	return SDP_SUCCESS;
}





int SDP_CatToStr(
	SDP_Str *      destination,
	const char *   string_to_cat)
{
	unsigned long length = strlen(string_to_cat);
	int rv;

	/* Just break out if it's NULL; nothing to concatenate: */
	if (string_to_cat == NULL)
		return SDP_SUCCESS;

	if (!SDP_IsStrInitialized(*destination))
	{
		return SDP_CopyToStr(destination, string_to_cat);
	}
	else if (SDP_StrBytesAvailable(*destination) < length)
	{
		rv = _GrowStrBuffer(destination, length);
		if (SDP_FAILED(rv))
			return SDP_FAILURE;
	}

	/* Copy it: */
	strncat(
		destination->buffer + destination->length,
		string_to_cat,
		length
	);
	destination->length += length;

	/* NULL terminate it by hand: */
	*(destination->buffer + destination->length) = '\0';

	return SDP_SUCCESS;
}





int SDP_CopyToStrUsingFormat(
	SDP_Str *      destination,
	size_t         bytes_needed,
	const char *   format,
	va_list        args)
{
	char *c;
	int rv;



	/*
	 * Ok, now make sure the buffer is initialized; that it's already
	 * been allocated:
	 */
	if (!SDP_IsStrInitialized(*destination))
	{
		rv = _InitializeStrBuffer(destination, bytes_needed);
		if (SDP_FAILED(rv))
			return SDP_FAILURE;
	}

	/* Reallocate the buffer if its too small for the string to be added: */
	if (destination->size < bytes_needed)
	{
		rv = _GrowStrBuffer(
			destination, bytes_needed - destination->size
		);
		if (SDP_FAILED(rv))
			return SDP_FAILURE;
	}

	/* Now add the formatted string to the generator buffer: */
	vsnprintf(
		destination->buffer,
		destination->size,
		format,
		args
	);

	/*
	 * We can't trust the return value of snprintf() and can't just do
	 * pointer += bytes_written. Instead, we need to go searching
	 * for NULL to find out how long this string is:
	 */
	c = destination->buffer;
	destination->length = 0;
	while (*c)
	{
		++destination->length;
		++c;
	}

	return SDP_SUCCESS;
}





int SDP_CatToStrUsingFormat(
	SDP_Str *      destination,
	size_t         bytes_needed,
	const char *   format,
	va_list        args)
{
	char *c;
	int rv;



	/*
	 * Ok, now make sure the buffer is initialized; that it's already
	 * been allocated:
	 */
	if (!SDP_IsStrInitialized(*destination))
	{
		rv = _InitializeStrBuffer(destination, bytes_needed);
		if (SDP_FAILED(rv))
			return SDP_FAILURE;
	}

	/* Reallocate the buffer if its too small for the string to be added: */
	if (SDP_StrBytesAvailable(*destination) < bytes_needed)
	{
		rv = _GrowStrBuffer(destination, bytes_needed);
		if (SDP_FAILED(rv))
			return SDP_FAILURE;
	}

	/* Now add the formatted string to the generator buffer: */
	vsnprintf(
		SDP_GetPtrToEndOfStr(*destination),
		SDP_StrBytesAvailable(*destination),
		format,
		args
	);

	/*
	 * We can't trust the return value of snprintf() and can't just do
	 * pointer += bytes_written. Instead, we need to go searching
	 * for NULL to find out how much longer this new string is:
	 */
	c = SDP_GetPtrToEndOfStr(*destination);
	while (*c)
	{
		++destination->length;
		++c;
	}

	return SDP_SUCCESS;
}





void SDP_DestroyStrBuffer(SDP_Str *string)
{
	if (string == NULL)
		return;

	if (string->buffer)
		SDP_Destroy(string->buffer);

	string->buffer = NULL;
	string->size   = 0;
	string->length = 0;
}





/*******************************************************************************
 * 
 * 	Name
 * 		_InitializeStrBuffer(string, initial_size)
 * 
 * 	Purpose
 * 		This function attempts to allocate a string buffer for "string"
 * 		to the number of bytes specified by "initial_size". It returns
 * 		true if it can, false if it can't
 * 
 * 	Parameters
 * 		string       - Pointer to the SDP_Str struct to initialize.
 * 		initial_size - The initial size of the string buffer to
 * 		               allocate.
 * 
 */

static int _InitializeStrBuffer(
	SDP_Str *   string,
	size_t      initial_size)
{
	string->buffer = (char *) SDP_Allocate(initial_size);
	if (string->buffer == NULL)
		return SDP_FAILURE;

	/*
	 * Initialize the size and length, and start the empty buffer off with
	 * a NULL character:
	 */
	string->size      = initial_size;
	string->length    = 0;
	*(string->buffer) = '\0';

	return SDP_SUCCESS;
}





/*******************************************************************************
 * 
 * 	Name
 * 		_GrowStrBuffer(string, bytes_to_grow)
 * 
 * 	Purpose
 * 		This function attempts to grow the string buffer in the SDP_Str
 * 		struct pointed to by "string" by the number of bytes specified
 * 		by "bytes_to_grow". It returns true if it can grow the string
 * 		buffer, false if it can't.
 * 
 * 	Parameters
 * 		string        - Pointer to the SDP_Str struct to grow the
 * 		                buffer of.
 * 		bytes_to_grow - The number of bytes to grow the string buffer
 * 		                by (not *to*).
 * 
 */

static int _GrowStrBuffer(
	SDP_Str *   string,
	size_t      bytes_to_grow)
{
	/* The size of the new, bigger buffer: */
	size_t new_buffer_size;
	char *bigger_buffer;

	new_buffer_size = string->size + bytes_to_grow;
	bigger_buffer   = (char *) SDP_Reallocate(
		string->buffer, new_buffer_size
	);
	if (bigger_buffer == NULL)
		return SDP_FAILURE;

	string->buffer = bigger_buffer;
	string->size   = new_buffer_size;

	return SDP_SUCCESS;
}





/*******************************************************************************
 *
 * Generic string manipulation functions:
 *
 ******************************************************************************/

char *SDP_StrDup(const char *string)
{
	size_t size_of_string;
	char *copy_of_string;

	size_of_string = strlen(string) + 1;

	copy_of_string = (char *) SDP_Allocate(size_of_string);
	if (copy_of_string == NULL)
		return NULL;

	memcpy(copy_of_string, string, size_of_string);

	return copy_of_string;
}





unsigned long SDP_StrCount(const char *string, char c)
{
	unsigned long total_occurrences = 0;
	char *last_occurrence;

	last_occurrence = strchr(string, c);
	while (last_occurrence)
	{
		string = last_occurrence;

		++total_occurrences;

		last_occurrence = strchr(string, c);
	}

	return total_occurrences;
}





/*
 * This renamed implementation of strsep() was taken from the OpenBSD project.
 * Here's the copyright notice for it:
 * -----------------------------------------------------------------------------
 * Copyright (c) 1990, 1993
 *	The Regents of the University of California.  All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the University nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 * -----------------------------------------------------------------------------
 *
 * Get next token from string *stringp, where tokens are possibly-empty
 * strings separated by characters from delim.  
 *
 * Writes NULs into the string at *stringp to end tokens.
 * delim need not remain constant from call to call.
 * On return, *stringp points past the last NUL written (if there might
 * be further tokens), or is NULL (if there are definitely no more tokens).
 *
 * If *stringp is NULL, strsep returns NULL.
 */
char *SDP_StrSep(char **stringp, const char *delim)
{
	char *s;
	const char *spanp;
	int c, sc;
	char *tok;

	if ((s = *stringp) == NULL)
		return (NULL);
	for (tok = s;;) {
		c = *s++;
		spanp = delim;
		do {
			if ((sc = *spanp++) == c) {
				if (c == 0)
					s = NULL;
				else
					s[-1] = 0;
				*stringp = s;
				return (tok);
			}
		} while (sc != 0);
	}
	/* NOTREACHED */
}





/*
 * Both of these renamed implementations of strlcat() and strlcpy() were taken
 * from the OpenBSD project. Here is the copyright notice for them:
 * -----------------------------------------------------------------------------
 * Copyright (c) 1998 Todd C. Miller <Todd.Miller@courtesan.com>
 *
 * Permission to use, copy, modify, and distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 * -----------------------------------------------------------------------------
 *
 * Copy src to string dst of size siz.  At most siz-1 characters
 * will be copied.  Always NUL terminates (unless siz == 0).
 * Returns strlen(src); if retval >= siz, truncation occurred.
 */
size_t SDP_StrLCopy(char *dst, const char *src, size_t siz)
{
	register char *d = dst;
	register const char *s = src;
	register size_t n = siz;

	/* Copy as many bytes as will fit */
	if (n != 0 && --n != 0) {
		do {
			if ((*d++ = *s++) == 0)
				break;
		} while (--n != 0);
	}

	/* Not enough room in dst, add NUL and traverse rest of src */
	if (n == 0) {
		if (siz != 0)
			*d = '\0';		/* NUL-terminate dst */
		while (*s++)
			;
	}

	return(s - src - 1);	/* count does not include NUL */
}

/*
 * Appends src to string dst of size siz (unlike strncat, siz is the
 * full size of dst, not space left).  At most siz-1 characters
 * will be copied.  Always NUL terminates (unless siz <= strlen(dst)).
 * Returns strlen(src) + MIN(siz, strlen(initial dst)).
 * If retval >= siz, truncation occurred.
 */
size_t SDP_StrLCat(char *dst, const char *src, size_t siz)
{
	register char *d = dst;
	register const char *s = src;
	register size_t n = siz;
	size_t dlen;

	/* Find the end of dst and adjust bytes left but don't go past end */
	while (n-- != 0 && *d != '\0')
		d++;
	dlen = d - dst;
	n = siz - dlen;

	if (n == 0)
		return(dlen + strlen(s));
	while (*s != '\0') {
		if (n != 1) {
			*d++ = *s;
			n--;
		}
		s++;
	}
	*d = '\0';

	return(dlen + (s - src));	/* count does not include NUL */
}
