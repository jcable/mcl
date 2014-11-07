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

#ifndef SDP_STREAM_TOKENIZER_INCLUDED
#define SDP_STREAM_TOKENIZER_INCLUDED

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>
#include <sys/types.h>



typedef struct {
	/* The type of stream we're reading from: */
	enum {
		SDP_CHARACTER_STREAM,
		SDP_FILE_STREAM
	} stream_type;

	/*
	 * This stores the source of the stream we'll read from to get each
	 * token:
	 */
	union {
		char *character;
		FILE *file;
	} stream;

	/*
	 * This stores a dynamically allocated buffer that contains each
	 * token one at a time:
	 */
	char *token_buffer;

	/* The size in bytes of the currently allocated token buffer: */
	size_t token_buffer_size;

	/* A pointer to the current position within token_buffer: */ 
	char *token_buffer_ptr;
} SDP_StreamTokenizer;



extern SDP_StreamTokenizer *SDP_NewStreamTokenizer(void);

extern void SDP_UseStringAsStream(
	SDP_StreamTokenizer *   stream_tokenizer,
	const char *            stream
);
extern void SDP_UseFileAsStream(
	SDP_StreamTokenizer *   stream_tokenizer,
	FILE *                  stream
);
extern int SDP_GetNextToken(
	SDP_StreamTokenizer *   stream_tokenizer,
	char **                 destination
);

extern void SDP_DestroyStreamTokenizer(SDP_StreamTokenizer *stream_tokenizer);



#ifdef __cplusplus
}
#endif

#endif
