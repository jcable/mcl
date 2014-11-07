/* $Id: FluteAPI.h,v 1.2 2005/05/12 16:03:34 moi Exp $ */
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

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <signal.h>
#include <string.h>

#ifdef WIN32
#include <time.h>
#include <io.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#else
#include <unistd.h>
#include <netdb.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <sys/ioctl.h>
#include <termios.h>
#include <sys/time.h>
#endif /* OS_DEP */

#ifndef ALC
#define ALC
#endif
#include "../common/mcl_lib_api.h"

#if defined(LINUX) /* On Linux systems, signal handlers must be of __sighandler_t type */
#define	sighandler_t	__sighandler_t

#elif defined(SOLARIS) || defined(IRIX) || defined(WIN32)

#define	sighandler_t	void (*)(int)

#elif defined(FREEBSD)
#define sighandler_t   sig_t

#endif


/* lock definitions */

#if defined(WIN32)
#define	flute_mutex_t	CRITICAL_SECTION
#define flute_thread_t	HANDLE

#elif defined(NO_PTHREAD_LOCK)

#define	flute_mutex_t	unsigned int
#define flute_thread_t	pthread_t

#else

#define	flute_mutex_t	pthread_mutex_t
#define flute_thread_t	pthread_t
#endif /* OS */


/**  Constants **/

#define CONTINUOUS	-1

#define PROMPT	1
#define NEVER	2
#define ALWAYS	3


/****** Profile Constants ******/

/*
 * Big files are all fragmented into fragments of size at most
 * XXX_MAX_FRAGMENT_SIZE bytes.
 * Try to make sure that the ALC object total size
 * (i.e. XXX_MAX_FRAGMENT_SIZE + MAX_TRAILER_SIZE) is small enough to be
 * encoded in a single ALC source block.
 * XXX: this is specified statically, should be done by querying MCL
 *
 * Edit it as appropriate...
 */
/*
 * choose the appropriate size according to the FEC codec used by MCL
 */
#define RSE_MAX_FRAGMENT_SIZE	(64*1024)		// 65535 bytes
#define LDGM_MAX_FRAGMENT_SIZE	(20*1024*1024)		// 20 MB
#define NULL_FEC_MAX_FRAGMENT_SIZE (1*1024*1024)	// 1 MB

/*
 * Files larger than this size should be handled by LDGM-*, others by RSE.
 * NB:	RSE is more efficient than LDGM-* even if a few blocks are needed.
 *	Above a certain threshold, this is no longer true.
 */
#define RSE_LDGM_FILE_SIZE_THRESHOLD	(20 * RSE_MAX_FRAGMENT_SIZE)

/*
 * choose the appropriate maximum size
 */
#define FLUTE_MAX_FDT_SIZE	(64*1024)	// 64 KB
#define FLUTE_MAX_FILE_SIZE (100*1024*1024)	// 100 MB


/* Maximum number of files supported by flute */
#define MAXNUMBER_OF_FILES	500

/* path and file name length */
#ifndef MAX_PATH
#define MAX_PATH 1024
#endif

#define MAX_FILENAME  512

/* includes */

#include "Flute.h"
#include "FluteReceiver.h"
#include "FluteSender.h"
#include "FluteFileInfo.h"
#include "FluteFileDeliveryCarousel.h"
