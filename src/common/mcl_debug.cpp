/* $Id: mcl_debug.cpp,v 1.9 2005/01/11 13:12:37 roca Exp $ */
/*
 *  Copyright (c) 2004 INRIA - All rights reserved
 *  (main author: Vincent Roca - vincent.roca@inrialpes.fr)
 *
 *  This program is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU General Public License
 *  as published by the Free Software Foundation; either version 2
 *  of the License, or (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307,
 *  USA.
 */

#ifdef ALC
#include "../alc/mcl_includes.h"

#elif defined(NORM)

#include "../norm/mcl_includes.h"
extern void	mcl_exit (int n);
#endif // ALC|NORM

#ifdef DEBUG

/*
 * Called only when an ASSERT failes. Usefull to set up a breakpoint.
 */
void
mcl_assert (void)
{
	mcl_exit(-1);
}


/*
 * Call it when you want to easily setup a breakpoint somewhere in code.
 */
void
mcl_break (void)
{
	return;
}

#endif /* DEBUG */


/**
 * Dump a data buffer.
 * @param len		total length of the buffer
 * @param to_dump	number of UINT32 words to dump
 */
void
mcl_dump_buffer (char	*buf,
		 int	len,
		 int	to_dump)	/* number of u32 words to dump */
{
	int	i, j;
	int	*ptr;

#ifndef min
#define min(a,b)	((a) <= (b) ? (a) : (b))
#endif
	i = min(to_dump, (len >> 2));
	j = 0;
	PRINT_OUT((mcl_stdout, "\tDUMP %d first bytes...\n\t", i * sizeof(int)))
	for (ptr = (int*)buf; i > 0; i--, ptr++) {
		/* convert to big endian format to be sure of byte order */
		PRINT_OUT((mcl_stdout, "%08lx ", htonl(*ptr)))
		if (++j == 8) {
			j = 0;
			PRINT_OUT((mcl_stdout, "\n\t"))
		}
	}
	PRINT_OUT((mcl_stdout, "\n"))
}

/**
 * Dump a data buffer containing ascii content.
 * @param to_dump	number of bytes to dump
 */
void
mcl_dump_ascii_buffer  (char	*buf,
			int	to_dump)	/* number of bytes to dump */
{
	int	i, j;
	char	*ptr;

#ifndef min
#define min(a,b)	((a) <= (b) ? (a) : (b))
#endif
	i = to_dump;
	j = 0;
	PRINT_OUT((mcl_stdout, "\tDUMP %d first bytes...\n---START-----\n\t", i * sizeof(int)))
	for (ptr = buf; i > 0; i--, ptr++) {
		/* convert to big endian format to be sure of byte order */
		PRINT_OUT((mcl_stdout, "%c", *ptr))
		if (++j == 40) {
			j = 0;
			PRINT_OUT((mcl_stdout, "\n\t"))
		}
	}
	PRINT_OUT((mcl_stdout, "\n---END-------\n"))
}

