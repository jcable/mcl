/* $Id: test_vtm_tx.c,v 1.2 2004/02/24 16:57:55 chneuman Exp $ */
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
 * test_vtm_tx.c
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
#endif /* OS_DEP */

#include <stdlib.h>			/* for exit */
#include <string.h>			/* for memset */
#include "../../src/common/mcl_lib_api.h"

#define SESSION_NB	2		/* 2 sessions max */
#define BUFLEN		150000		/* buffer size */
#define VERBOSITY	0		/* verbosity level */

int	ttl = 0;			/* do not leave the host */


int
main (int       argc,
      char      *argv[])
{
	int     id[SESSION_NB];
	int	port[SESSION_NB] = {2000, 2100};	/* in host format! */
	UINT32	addr[SESSION_NB] = {
			ntohl(inet_addr("225.1.0.0")),
			ntohl(inet_addr("225.2.0.0"))}; /* in host format! */
	int	err;
	int	ses;
	char	buf[BUFLEN];
	int	verbose = VERBOSITY;
	int	set = 1;			/* set value for mcl_ctl */

	printf("test_vtm_tx: create %d sending sessions...\n", SESSION_NB);
	for (ses = 0; ses < SESSION_NB; ses++) {
		printf(" %d ", ses);
		if ((id[ses] = mcl_open("w")) < 0) {
			printf("ERROR: mcl_open failed for session %d\n", ses);
			exit(-1);
		}
		err = mcl_ctl(id[ses], MCL_OPT_PORT, (void*)&port[ses], sizeof(*port));
		if (err != 0) {
			printf("ERROR: mcl_ctl PORT failed for session %d\n", ses);
			exit(-1);
		}
		err = mcl_ctl(id[ses], MCL_OPT_ADDR, (void*)&addr[ses], sizeof(*addr));
		if (err != 0) {
			printf("ERROR: mcl_ctl ADDR failed for session %d\n", ses);
			exit(-1);
		}
		if (verbose > 0) {
			err = mcl_ctl(id[ses], MCL_OPT_VERBOSITY, (void*)&verbose, sizeof(verbose));
			if (err != 0) {
				printf("ERROR: mcl_ctl VERBOSITY failed for session %d\n", ses);
				exit(-1);
			}
		}
		err = mcl_ctl(id[ses], MCL_OPT_TTL, (void*)&ttl, sizeof(ttl));
		if (err != 0) {
			printf("ERROR: mcl_ctl TTL failed\n");
			exit(-1);
		}
		err = mcl_ctl(id[ses], MCL_OPT_VIRTUAL_TX_MEMORY, (void*)&set, sizeof(set));
		if (err != 0) {
			printf("ERROR: mcl_ctl VIRTUAL_TX_MEMORY: not supported, ignore test...\n");
			exit(0);
		}
		/* don't send immediately but wait... */
		err = mcl_ctl(id[ses], MCL_OPT_KEEP_DATA, NULL, 0);
		if (err != 0) {
			printf("ERROR: mcl_ctl KEEP_DATA failed for session %d\n", ses);
			exit(-1);
		}
	}
	printf("\ntest_vtm_tx: finish the init of the %d sessions...\n",
			SESSION_NB);
	for (ses = 0; ses < SESSION_NB; ses++) {
		printf(" %d ", ses);
		memset(buf, ses, BUFLEN);	/* init buffer with session # */
		if ((err = mcl_send(id[ses], buf, BUFLEN)) < 0) {
			printf("ERROR: mcl_send failed for session %d\n", ses);
			exit(-1);
		}
	}
	printf("\ntest_vtm_tx: write on the %d sessions...\n", SESSION_NB);
	/* ...and now send everything */
	for (ses = 0; ses < SESSION_NB; ses++) {
		err = mcl_ctl(id[ses], MCL_OPT_PUSH_DATA, NULL, 0);
		if (err != 0) {
			printf("ERROR: mcl_ctl PUSH_DATA failed for session %d\n", ses);
			exit(-1);
		}
	}
	printf("\ntest_vtm_tx: close the %d sessions...\n", SESSION_NB);
	for (ses = 0; ses < SESSION_NB; ses++) {
		printf(" %d ", ses);
		if ((err = mcl_close(id[ses])) < 0) {
			printf("ERROR: mcl_close failed for session %d\n", ses);
			exit(-1);
		}
	}
	printf("\n... test_vtm_tx ok\n");
	return 0;
}

