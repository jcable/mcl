/* $Id: fcast.h,v 1.10 2005/01/11 13:12:22 roca Exp $ */
/*
 *  Copyright (c) 1999-2003 INRIA - Universite Paris 6 - All rights reserved
 *  (main authors: Julien Laboure - julien.laboure@inrialpes.fr
 *                 Vincent Roca - vincent.roca@inrialpes.fr)
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


#ifdef WIN32
#include <time.h>
#include <io.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include "getopt.h"
#else
#if 0
#if SOLARIS
#include <sys/dir.h>	/* see comment about readdir in fsend.c */
#else
#include <dirent.h>
#endif
#else
#include <dirent.h>
#endif
#include <unistd.h>
#include <netdb.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>
#endif /* OS_DEP */



#include "../src/common/mcl_lib_api.h"
#include "checksum.h"
#include "metadata.h"
#include "macros.h"
#include "filemanage.h"
#include "fsend.h"
#include "frecv.h"


#if defined(LINUX) /* On Linux systems, signal handlers must be of __sighandler_t type */
#define	sighandler_t	__sighandler_t
#elif defined(SOLARIS) || defined(IRIX) || defined(WIN32)
#define	sighandler_t	void (*)(int)
#elif defined(FREEBSD)
#define sighandler_t	sig_t
#endif


/****** Constants ******/

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
#ifdef ALC
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

#elif defined(NORM)

#define RSE_MAX_FRAGMENT_SIZE	(20*1024*1024)	// use larger ADUs: eg. 20 MB
#define NULL_FEC_MAX_FRAGMENT_SIZE (20*1024*1024)	// 20 MB

#endif /* RM_PROTOCOL */
#define MAX_TRAILER_SIZE	2050	// trailer appended to each fragment


/* other constants, do not edit */
#define SEND			1
#define RECV			2

#define OPTIMIZE_SPEED		1
#define OPTIMIZE_SPACE		2
#define OPTIMIZE_CPU		3


/****** Globals ******/

extern int	id;
extern int	verbose;
extern int	levels;
//extern unsigned long addr;
extern struct sockaddr	*addr;
//extern unsigned long	src_addr;
extern struct sockaddr	*src_addr;
extern int	port;
extern int	ttl;
extern float	fec_ratio;
extern int	delivery_mode;

extern int	mode;
extern int	optimode;
extern int	speedmode;
extern int	overwrite;
extern int	single_layer;
extern int	reuse_tx_buff;
extern char	fileparam[MAX_PATH+MAX_FILENAME];
extern bool	recursive;
extern bool	meta_object_mode;
extern int	silent;
extern int	tx_huge_file;

/********************/

