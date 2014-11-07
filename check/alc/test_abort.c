/* $Id: test_abort.c,v 1.2 2004/02/24 16:57:26 chneuman Exp $ */
/*
 *  Copyright (c) 1999-2002 INRIA - Universite Paris 6 - All rights reserved
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

/*
 * test_abort.c
 */

#include <stdio.h>
#include <stdlib.h>			/* for exit */

#ifdef FREEBSD
#include <sys/types.h>
#endif

#ifdef WIN32
#include <winsock2.h>
#include <io.h>
#else
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>			/* for sleep */
#endif /* OS_DEP */

#include "../../src/common/mcl_lib_api.h"


#define BUFLEN		2400		/* buffer size */
#define VERBOSITY	0		/* verbosity level */

int     ttl = 0;			/* do not leave the host */


int
main (int       argc,
      char      *argv[])
{
	int     id = 0;
	int	port = 2324;
	unsigned long	addr = ntohl(inet_addr("230.1.2.3"));
	int	verbose = VERBOSITY;
	char	buf[BUFLEN];
	int	err;

	printf("test_abort: open(w)/send/abort\n");
	if ((id = mcl_open("w")) < 0) {
		printf("ERROR: mcl_open failed\n");
		exit(-1);
	}
	if (port > 0) {		/* in host format! */
		err = mcl_ctl(id, MCL_OPT_PORT, (void*)&port, sizeof(port));
		if (err != 0) {
			printf("ERROR: mcl_ctl PORT failed\n");
			exit(-1);
		}
	}
	if (addr > 0) {		/* in host format! */
		err = mcl_ctl(id, MCL_OPT_ADDR, (void*)&addr, sizeof(addr));
		if (err != 0) {
			printf("ERROR: mcl_ctl ADDR failed\n");
			exit(-1);
		}
	}
	if (verbose > 0) {
		err = mcl_ctl(id, MCL_OPT_VERBOSITY, (void*)&verbose,
				sizeof(verbose));
		if (err != 0) {
			printf("ERROR: mcl_ctl VERBOSITY failed\n");
			exit(-1);
		}
	}
	err = mcl_ctl(id, MCL_OPT_TTL, (void*)&ttl, sizeof(ttl));
	if (err != 0) {
		printf("ERROR: mcl_ctl TTL failed\n");
		exit(-1);
	}
	if ((err = mcl_send(id, buf, BUFLEN)) < 0) {
		printf("ERROR: mcl_send failed\n");
		exit(-1);
	}
	if ((err = mcl_abort(id)) < 0) {
		printf("ERROR: mcl_abort failed\n");
		exit(-1);
	}
	printf("... ok\n");

	printf("******\n");
	printf("test_abort: open(w)/abort\n");
	if ((id = mcl_open("w")) < 0) {
		printf("ERROR: mcl_open failed\n");
		exit(-1);
	}
	if (port > 0) {		/* in host format! */
		err = mcl_ctl(id, MCL_OPT_PORT, (void*)&port, sizeof(port));
		if (err != 0) {
			printf("ERROR: mcl_ctl PORT failed\n");
			exit(-1);
		}
	}
	if (addr > 0) {		/* in host format! */
		err = mcl_ctl(id, MCL_OPT_ADDR, (void*)&addr, sizeof(addr));
		if (err != 0) {
			printf("ERROR: mcl_ctl ADDR failed\n");
			exit(-1);
		}
	}
	if (verbose > 0) {
		err = mcl_ctl(id, MCL_OPT_VERBOSITY, (void*)&verbose,
				sizeof(verbose));
		if (err != 0) {
			printf("ERROR: mcl_ctl VERBOSITY failed\n");
			exit(-1);
		}
	}
	if ((err = mcl_abort(id)) < 0) {
		printf("ERROR: mcl_abort failed\n");
		exit(-1);
	}
	printf("... ok\n");
#ifdef WIN32
	_sleep(1000);	/* for the wait pid in test_no_tx.sh to work properly */
#else
	sleep(1);	/* for the wait pid in test_no_tx.sh to work properly */
#endif

        return(0);
}

