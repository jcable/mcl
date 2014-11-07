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
 * This file contains the stream tokenizer that extracts newline-delimited
 * tokens from some sort of input stream.
 * 
 ******************************************************************************/

#include <stdio.h>
#include <string.h>
#include <sys/types.h>

#include "SDP_StreamTokenizer.h"
#include "SDP_Utility.h"

/* The initial size of the token buffer to allocate: */
#define INITIAL_TOKEN_BUFFER_SIZE 1024

/*
 * This macro adds a single character retrieved from some stream to the token
 * buffer:
 */
#define ADD_TO_TOKEN_BUFFER(_stream_tokenizer_, _c_) \
	(*((_stream_tokenizer_)->token_buffer + bytes_in_use) = _c_)

/*
 * Character and string constants to handle Unix, Windows, and MacOS newlines
 * elegantly:
 */
#define C_CR   '\015'
#define C_LF   '\012'
#define S_CRLF "\015\012"
#define S_CR   "\015"
#define S_LF   "\012"







SDP_StreamTokenizer *SDP_NewStreamTokenizer(void)
{
	SDP_StreamTokenizer *stream_tokenizer =
		(SDP_StreamTokenizer *) SDP_Allocate(
			sizeof(SDP_StreamTokenizer)
		);
	if (stream_tokenizer == NULL)
		return NULL;

	memset(stream_tokenizer, 0, sizeof(SDP_StreamTokenizer));

	/* Allocate the token buffer: */
	stream_tokenizer->token_buffer = (char *) SDP_Allocate(
		INITIAL_TOKEN_BUFFER_SIZE
	);
	if (stream_tokenizer->token_buffer == NULL)
	{
		SDP_Destroy(stream_tokenizer);
		return NULL;
	}

	stream_tokenizer->token_buffer_size = INITIAL_TOKEN_BUFFER_SIZE;

	/* The stream union need to be set to something right from the onset: */
	SDP_UseStringAsStream(stream_tokenizer, (char *) NULL);

	return stream_tokenizer;
}





void SDP_UseStringAsStream(
	SDP_StreamTokenizer *   stream_tokenizer,
	const char *            stream)
{
	SDP_AssertNotNull(stream_tokenizer);

	stream_tokenizer->stream_type      = SDP_CHARACTER_STREAM;
	stream_tokenizer->stream.character = (char *) stream;
}





void SDP_UseFileAsStream(
	SDP_StreamTokenizer *   stream_tokenizer,
	FILE *                  stream)
{
	SDP_AssertNotNull(stream_tokenizer);

	stream_tokenizer->stream_type = SDP_FILE_STREAM;
	stream_tokenizer->stream.file = stream;
}





/* Allocates more memory for the token buffer: */
static int _GrowTokenBuffer(SDP_StreamTokenizer *stream_tokenizer);

/* Gets the next character from the stream: */
static char _GetNextChar(SDP_StreamTokenizer *stream_tokenizer);

/* Puts the last character back into the stream: */
static void _UngetNextChar(
	SDP_StreamTokenizer *   stream_tokenizer,
	char                    c
);

int SDP_GetNextToken(
	SDP_StreamTokenizer *   stream_tokenizer,
	char **                 destination)
{
	size_t bytes_available = stream_tokenizer->token_buffer_size;
	size_t bytes_in_use    = 0;
	char c;
	int rv;

	SDP_AssertNotNull(stream_tokenizer);

	/*
	 * Handle the possible end-of-stream we have already reached because
	 * of previous calls to this routine:
	 */
	c = _GetNextChar(stream_tokenizer);
	if (c == '\0')
	{
		*destination = NULL;
		return SDP_SUCCESS;
	}

	while (c)
	{
		/*
		 * If there's no more space in the token buffer even for one
		 * more character, then we need to reallocate it:
		 */
		if (bytes_available <= 0)
		{
			rv = _GrowTokenBuffer(stream_tokenizer);
			if (SDP_FAILED(rv))
				return SDP_FAILURE;

			/*
			 * Adjust the current number of bytes available to
			 * account for the reallocation of the token buffer:
			 */
			bytes_available =
			stream_tokenizer->token_buffer_size - bytes_in_use;
		}

		/* Check for newline to terminate the current token: */
		if (c == C_CR || c == C_LF)
		{
			/*
			 * If this is a CR and the next character is a LF, then
			 * both characters are a single line ending and need to
			 * be removed from the stream:
			 */
			if (c == C_CR)
			{
				char next_char = _GetNextChar(stream_tokenizer);

				/* Check for end of stream: */
				if (next_char == '\0');
					break;

				/*
				 * If it's not a line feed after that carriage
				 * return, put it back:
				 */
				if (next_char != C_LF)
					_UngetNextChar(
						stream_tokenizer, next_char
					);
			}

			/*
			 * The line ending we've just removed from the stream
			 * terminates this token:
			 */
			break;
		}

		ADD_TO_TOKEN_BUFFER(stream_tokenizer, c);

		++bytes_in_use;
		--bytes_available;

		c = _GetNextChar(stream_tokenizer);
	}

	ADD_TO_TOKEN_BUFFER(stream_tokenizer, '\0');

	*destination = stream_tokenizer->token_buffer;

	return SDP_SUCCESS;
}





void SDP_DestroyStreamTokenizer(SDP_StreamTokenizer *stream_tokenizer)
{
	SDP_AssertNotNull(stream_tokenizer);

	if (stream_tokenizer->token_buffer)
		SDP_Destroy(stream_tokenizer->token_buffer);

	SDP_Destroy(stream_tokenizer);
}





/*******************************************************************************
 * 
 * 	Name
 * 		_GrowTokenBuffer(stream_tokenizer)
 * 
 * 	Purpose
 * 		This function tries to grow the token buffer, doubling it in
 * 		size. It returns true if it successfully grows the token
 * 		buffer, false if memory reallocation fails.
 * 
 * 	Parameters
 * 		tokenizer - A pointer to the SDP_StreamTokenizer struct to grow
 * 		            the token buffer of.
 * 
 */
static int _GrowTokenBuffer(SDP_StreamTokenizer *stream_tokenizer)
{
	size_t bytes_to_reallocate;
	char *larger_token_buffer;

	bytes_to_reallocate = stream_tokenizer->token_buffer_size * 2;
	larger_token_buffer = (char *) SDP_Reallocate(
		stream_tokenizer->token_buffer, bytes_to_reallocate
	);
	if (larger_token_buffer == NULL)
		return SDP_FAILURE;

	stream_tokenizer->token_buffer      = larger_token_buffer;
	stream_tokenizer->token_buffer_size = bytes_to_reallocate;

	return SDP_SUCCESS;
}





/*******************************************************************************
 * 
 * 	Name
 * 		_GetNextChar(stream_tokenizer)
 * 
 * 	Purpose
 * 		This function gets the next character from the stream and
 * 		returns it. It returns '\0' when the stream is exausted of all
 * 		of its characters.
 * 
 * 	Parameters
 * 		tokenizer - A pointer to the SDP_StreamTokenizer struct.
 * 
 */
static char _GetNextChar(SDP_StreamTokenizer *stream_tokenizer)
{
	/* "int", because fgetc() returns -1 for EOF: */
	int c;

	switch (stream_tokenizer->stream_type)
	{
		case SDP_CHARACTER_STREAM:
			c = *(stream_tokenizer->stream.character);

			/*
			 * Only move on to the next char if it isn't NULL,
			 * otherwise it just saty at this point:
			 */
			if (c)
				++stream_tokenizer->stream.character;

			break;
		case SDP_FILE_STREAM:
			c = fgetc(stream_tokenizer->stream.file);

			/* Same thing as above: */
			if (c == EOF)
				c = '\0';

			break;
		default:
			SDP_AssertionFailed("Bad stream type.");
			break;
	}

	return((char) c);
}




/*******************************************************************************
 * 
 * 	Name
 * 		_UngetNextChar(stream_tokenizer, c)
 * 
 * 	Purpose
 * 		This function puts the last character back onto the stream if
 * 		you're not ready for it yet.
 * 
 * 	Parameters
 * 		tokenizer - A pointer to the SDP_StreamTokenizer struct.
 * 		c         - The character to put back in the stream (ignored
 * 		            for some types of streams).
 * 
 */
static void _UngetNextChar(
	SDP_StreamTokenizer *   stream_tokenizer,
	char                    c)
{
	switch (stream_tokenizer->stream_type)
	{
		case SDP_CHARACTER_STREAM:
			--stream_tokenizer->stream.character;
			break;
		case SDP_FILE_STREAM:
			ungetc(c, stream_tokenizer->stream.file);
			break;
		default:
			SDP_AssertionFailed("Bad stream type.");
			break;
	}
}
