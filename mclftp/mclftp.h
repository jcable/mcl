/* $Id: mclftp.h,v 1.1.1.1 2003/09/03 12:45:43 chneuman Exp $ */
/*
 *  Copyright (c) 2002 INRIA - All rights reserved
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


#include <stdio.h>
#include <stdlib.h>
#include <errno.h>

#include <ctype.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#ifdef WIN32
#include <winsock2.h>
#include <io.h>
#include "getopt.h"
#else
#include <unistd.h>
#include <netdb.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#endif /* OS_DEP */

#include "../src/common/mcl_lib_api.h"


/* send/recv buffer size */
#define BUFLEN	31744	/* 32768-1024 to keep room for 4 offset bytes */
/*#define BUFLEN	32768*/
/*#define BUFLEN	16384*/

/****** general macros ******/

/*
 * print to stdout
 */
#define PRINT_OUT(a) { \
		fprintf a; \
		fflush(stdout); \
	}

/*
 * print and exit
 */
#define EXIT(a) { \
		printf a; \
		fflush(stdout); \
		exit(-1); \
	}

#define ASSERT(c) { \
		if (!(c)) { \
			fprintf(stderr, "ASSERT [%s:%d] failed\n", \
				__FILE__, __LINE__); \
			fflush(stderr); \
			exit (-1); \
		} \
	}

