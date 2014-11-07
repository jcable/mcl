/* $Id: mcl_debug.h,v 1.6 2005/01/11 13:12:38 roca Exp $ */
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

/****** prototypes ******/

extern FILE	*mcl_stdout;
extern FILE	*mcl_stderr;


/**
 * Dump a data buffer in hexa.
 * @param len		total length of the buffer
 * @param to_dump	number of UINT32 words to dump
 */
extern void	mcl_dump_buffer		(char *buf, int len, int to_dump);

/**
 * Dump a data buffer in ascii.
 * @param len		total length of the buffer
 * @param to_dump	number of UINT32 words to dump
 */
extern void	mcl_dump_ascii_buffer	(char *buf, int to_dump);


/****** general macros ******/

/*
 * print to stdout
 */
#define PRINT_OUT(a) { \
		fprintf a; \
		fflush(mcl_stdout); \
	}

/*
 * print to stderr
 */
#define PRINT_ERR(a) { \
		fprintf a; \
		fflush(mcl_stderr); \
	}

/*
 * Print according to level
 */
#ifdef ALC
#define PRINT_LVL(l, a)	if (mclcb->verbose >= (l)) { \
				fprintf a; \
				fflush(mcl_stdout); \
			}
#else
#define PRINT_LVL(l, a)	if (mclcb->get_verbosity() >= (l)) { \
				fprintf a; \
				fflush(mcl_stdout); \
			}
#endif

/*
 * Trace only in debug mode according to level
 */
#ifdef DEBUG
#define TRACE(a)	{ \
				fprintf a; \
				fflush(mcl_stdout); \
			}

#define TRACELVL(l, a)	if (mclcb->get_verbosity() >= (l)) { \
				fprintf a; \
				fflush(mcl_stdout); \
			}

#else  /* DEBUG */
#define TRACE(a)		;	/* empty instruction is less risky */
#define TRACELVL(lvl, a)	;	/* empty instruction is less risky */
#endif /* DEBUG */

#if 0
/*
 * Close everything and exit.
 */
#define EXIT(c, n) { \
		if (c) \
			kill(getppid(), SIGUSR2); \
		mcl_exit(n); \
	}
#endif

/*
 * Check that an assertion is true.
 */
#ifdef ASSERT	/* in case it was already defined, erase it... */
#undef ASSERT
#endif

#ifdef DEBUG
/* More convenient with a function since we can breakpoint it! */
extern void mcl_assert (void);
#define ASSERT(c) { \
		if (!(c)) { \
			fprintf(stderr, "ASSERT [%s:%d] failed\n", \
				__FILE__, __LINE__); \
			fflush(stderr); \
			mcl_assert(); \
		} \
	}
#else  /* DEBUG */
#define ASSERT(c)
#endif /* DEBUG */


/****** Other debug functions ******/

#ifdef ALC
#ifdef DEBUG
#define	PRINTDULIST(tl)		PrintDUList(tl)
#else  /* DEBUG */
#define	PRINTDULIST(tl)
#endif /* DEBUG */
#endif /* ALC */

#define MAGIC_VER	0XAB	/* magic char number for pkt header debug */

