/* $Id: test_vtm_rx.c,v 1.2 2004/02/24 16:57:53 chneuman Exp $ */
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
 * test_vtm_rx.c
 * open two sessions, use the VTM service, and check what has been received
 * receiver side version...
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
#include "../../src/common/mcl_lib_api.h"

#define SESSION_NB	2		/* 2 sessions max */
#define BUFLEN		150000		/* buffer size */
#define VERBOSITY	0		/* verbosity level */

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
	int	i;
	int	verbose = VERBOSITY;
	int	unset = 0;			/* unset value for mcl_ctl */

	printf("\ttest_vtm_rx: create %d receiving sessions...\n\t", SESSION_NB);
	for (ses = 0; ses < SESSION_NB; ses++) {
		printf(" %d ", ses);
		if ((id[ses] = mcl_open("r")) < 0) {
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
		/* diseable VTM as we are a receiver... */
		err = mcl_ctl(id[ses], MCL_OPT_VIRTUAL_TX_MEMORY, (void*)&unset, sizeof(unset));
		if (err != 0) {
			printf("ERROR: mcl_ctl VIRTUAL_TX_MEMORY: not supported, ignore test...\n");
			exit(0);
		}

	}

	printf("\n\ttest_vtm_rx: finish the init of the %d sessions...\n\t",
			SESSION_NB);
	for (ses = 0; ses < SESSION_NB; ses++) {
		if ((err = mcl_recv(id[ses], NULL, 0)) < 0) {
			printf("ERROR: empty mcl_recv failed for session %d\n", ses);
			exit(-1);
		}
	}

	printf("\n\ttest_vtm_rx: read on the %d sessions...\n",
			SESSION_NB);
	for (ses = 0; ses < SESSION_NB; ses++) {
		printf("\trecv on session %d... ", ses);
		if ((err = mcl_recv(id[ses], buf, BUFLEN)) < 0) {
			printf("ERROR: mcl_recv failed for session %d\n", ses);
			exit(-1);
		}
		for (i = 0; i < BUFLEN; i++) {
			/* all bytes should be equal to ses */
			if (buf[i] != ses) {
				printf("ERROR: bad value in received buffer (got %d, expected %d)\n",
					buf[i], ses);
				exit(-1);
			}
		}
	}

	printf("\n\ttest_vtm_rx: close the %d sessions...\n",
			SESSION_NB);
	for (ses = 0; ses < SESSION_NB; ses++) {
		printf("\tclose session %d... ", ses);
		if ((err = mcl_close(id[ses])) < 0) {
			printf("ERROR: mcl_close failed for session %d\n", ses);
			exit(-1);
		}
	}
	printf("\n\t... test_vtm_rx ok\n");
	return 0;
}

