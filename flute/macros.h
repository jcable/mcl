/* $Id: macros.h,v 1.5 2005/05/12 16:03:31 moi Exp $ */
/*
 *  Copyright (c) 2003-2004 INRIA - All rights reserved
 *  main authors: Christoph Neumann - christoph.neumann@inrialpes.fr
 *		  Vincent Roca - vincent.roca@inrialpes.fr
 *		  Julien Laboure - julien.laboure@inrialpes.fr
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


#ifndef MACROS_H
#define MACROS_H

#include <stdio.h>
#include <stdlib.h>

/****** general macros ******/

#ifndef min
#define min(a,b)	((a) <= (b) ? (a) : (b))
#endif
#ifndef max
#define max(a,b)	((a) >= (b) ? (a) : (b))
#endif

/*
 * print to stdout
 */
#define PRINT(a) {  printf a; fflush(stdout);  }

/*
 * print and exit
 */
#define EXIT(a) { printf a; fflush(stdout); exit(-1); }

/*
 * test, print and exit if error (i.e. if != 0)
 */
#define EXIT_ON_ERROR(err, a)	{ if (err) { EXIT(a) } }


#define ASSERT(c) { \
		if (!(c)) { \
			fprintf(stderr, "ASSERT [%s:%d] failed\n", \
				__FILE__, __LINE__); \
			fflush(stderr); \
			exit (-1); \
		} \
	}

#define BUFFER_TO_INT32(x) ((*(x)<<24) + (*(x+1)<<16) + (*(x+2)<<8) + (*(x+3)))


#endif
