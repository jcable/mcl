/* $Id: full_coder.h,v 1.3 2005/05/18 07:37:31 roca Exp $ */

/*  LDPC Full FEC encoder/decoder include file.
 *  (c) Copyright 2002-2005 INRIA - All rights reserved
 *  Main authors: Christoph Neumann (christoph.neumann@inrialpes.fr)
 *                Vincent Roca      (vincent.roca@inrialpes.fr)
 *		  Julien Laboure   (julien.laboure@inrialpes.fr)
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

#ifdef WIN32		/* Windows specific includes */
#include <Winsock2.h>
#include <windows.h>
#else	/* UNIX */	/* Unix specific includes */
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/time.h>	/* for gettimeofday */
#endif	/* OS */

#include "../src/ldpc_fec.h"

/*
 * OS dependant definitions
 */
#ifdef WIN32
  #define SLEEP(t)	 Sleep(t)
#else
  #define SOCKET	 int
  #define SOCKADDR	 sockaddr
  #define SOCKADDR_IN	 sockaddr_in
  #define INVALID_SOCKET (-1)
  #define SOCKET_ERROR	 (-1)
  #define closesocket	 close
  #define SLEEP(t)	 usleep(t*1000)
#endif	/* OS */


/*
 * DEFAULT parameters...
 */
#define PKTSZ	1024	// Packets size, in bytes (multiple of 4).
#define LEFT_DEGREE	7	// Left degree of data nodes in the checks graph
#define SEED		1	// Seed used to initialize LDPC matrix
#define VERBOSITY	1	// Define the verbosity level :
				//	0 : no infos
				//	1 : infos
				//	2 : packets dump

#define SERVER_IP	"127.0.0.1"	// Server IP
#define SERVER_PORT	10978		// Server port (TCP)


