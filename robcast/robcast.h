/* $Id: robcast.h,v 1.1.1.1 2003/09/03 12:45:43 chneuman Exp $ */
/*
 *  Copyright (c) 2003 INRIA - All rights reserved
 *  (main authors: Vincent Roca - vincent.roca@inrialpes.fr)
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
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>

#ifdef WIN32
#include <winsock2.h>
#include <io.h>
#define S_IRWXU _S_IWRITE
#else
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>			/* for sleep */
#endif /* OS_DEP */

#include <stdlib.h>			/* for exit */
#include "../src/common/mcl_lib_api.h"


/*
 * print and exit
 */
#define EXIT(a) { printf a; fflush(stdout); exit(-1); }

/*
 * test, print and exit if error (i.e. if != 0)
 */
#define EXIT_ON_ERROR(err, a)   { if (err) { EXIT(a) } }


#define BUFLEN		150		/* buffer size */
#define SESSION_NB	2		/* two sessions: ctrl (0), data (1) */
#define CTRL_IDX	0		/* index in tabs for ctrl session */
#define DATA_IDX	1		/* index in tabs for data session */

#define CTRL_PORT	13200
#define DATA_PORT	13250
#define CTRL_ADDR	"225.132.0.0"
#define DATA_ADDR	"225.133.0.0"
