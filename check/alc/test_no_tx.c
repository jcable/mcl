/* $Id: test_no_tx.c,v 1.2 2004/02/24 16:57:42 chneuman Exp $ */
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
 * test_no_tx.c
 */

#include <stdio.h>

#ifdef FREEBSD
#include <sys/types.h>
#endif

#ifdef WIN32
#include <winsock2.h>
#else
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>			/* for sleep */
#endif /* OS_DEP */

#include <stdlib.h>			/* for exit */
#include "../../src/common/mcl_lib_api.h"

int	ttl = 0;			/* do not leave the host */


int
main (int       argc,
      char      *argv[])
{
	int     id = 0;
	int	port = 2324;
	unsigned long	addr = ntohl(inet_addr("230.1.2.3"));
	int	err;

	printf("test_no_tx: open(r)/close\n");
	if ((id = mcl_open("r")) < 0) {
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
	err = mcl_ctl(id, MCL_OPT_TTL, (void*)&ttl, sizeof(ttl));
	if (err != 0) {
		printf("ERROR: mcl_ctl TTL failed\n");
		exit(-1);
	}
	if ((err = mcl_close(id)) < 0) {
		printf("ERROR: mcl_close failed\n");
		exit(-1);
	}
	printf("... ok\n");

	printf("test_no_tx: open(w)/close\n");
	if ((id = mcl_open("w")) < 0) {
		printf("ERROR: mcl_open failed\n");
		exit(-1);
	}
	if (port > 0) {		/* in host format! */
		mcl_ctl(id, MCL_OPT_PORT, (void*)&port, sizeof(port));
		if (err != 0) {
			printf("ERROR: mcl_ctl PORT failed\n");
			exit(-1);
		}
	}
	if (addr > 0) {		/* in host format! */
		mcl_ctl(id, MCL_OPT_ADDR, (void*)&addr, sizeof(addr));
		if (err != 0) {
			printf("ERROR: mcl_ctl ADDR failed\n");
			exit(-1);
		}
	}
	if ((err = mcl_close(id)) < 0) {
		printf("ERROR: mcl_close failed\n");
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

