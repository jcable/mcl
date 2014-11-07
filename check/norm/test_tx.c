/* $Id: test_tx.c,v 1.1.1.1 2003/09/03 12:45:41 chneuman Exp $ */
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

/*
 * test_tx.c
 */

#include <stdio.h>
#include <unistd.h>

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

#define SESSION_NB	2		/* number of sessions */
#define BUFLEN		150000		/* buffer size in bytes */
#define VERBOSITY	0		/* verbosity level */
#define STATS		1		/* statistics level */
#define SEND_NB		10		/* number of calls to mcl_send() */

int
main (int       argc,
      char      *argv[])
{
	int     id[SESSION_NB];
	int	port[SESSION_NB] = {2000, 2100};	/* in host format! */
	unsigned long	addr[SESSION_NB] = {
			ntohl(inet_addr("225.1.0.0")),
			ntohl(inet_addr("225.2.0.0"))}; /* in host format! */
	int	err;
	int	ses;
	char	buf[BUFLEN];
	int	verbose = VERBOSITY;
	int	stats = STATS;
	int	ttl = 2;
	//int	set = 1;			/* set value for mcl_ctl */
	int	send_nb;			/* number of call to mcl_send */

	printf("test_tx: create %d sending sessions...\n", SESSION_NB);
	for (ses = 0; ses < SESSION_NB; ses++) {
		printf(" %d ", ses);
		if ((id[ses] = mcl_open("w")) < 0) {
			printf("ERROR: mcl_open failed for session %d\n", ses);
			exit(-1);
		}
		if (verbose > 0) {
			err = mcl_ctl(id[ses], MCL_OPT_VERBOSITY, (void*)&verbose, sizeof(verbose));
			if (err != 0) {
				printf("ERROR: mcl_ctl VERBOSITY failed for session %d\n", ses);
				exit(-1);
			}
		}
		if (stats > 0) {
			err = mcl_ctl(id[ses], MCL_OPT_STATS, (void*)&stats, sizeof(stats));
			if (err != 0) {
				printf("ERROR: mcl_ctl STATS failed for session %d\n", ses);
				exit(-1);
			}
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
		err = mcl_ctl(id[ses], MCL_OPT_TTL, (void*)&ttl, sizeof(ttl));
		if (err != 0) {
			printf("ERROR: mcl_ctl TTL failed for session %d\n", ses);
			exit(-1);
		}
	}
	printf("\ntest_tx: finish the init of the %d sessions...\n",
			SESSION_NB);
	for (ses = 0; ses < SESSION_NB; ses++) {
		printf(" %d ", ses);
		if ((err = mcl_send(id[ses], NULL, 0)) < 0) {
			printf("ERROR: mcl_send(NULL,0) failed for session %d\n", ses);
			exit(-1);
		}
	}
	printf("\ntest_tx: write on the %d sessions...\n", SESSION_NB);
	for (ses = 0; ses < SESSION_NB; ses++) {
		printf(" %d ", ses);
		memset(buf, ses, BUFLEN);	/* init buffer with session # */
		for (send_nb = SEND_NB; send_nb > 0; send_nb--) {
			if ((err = mcl_send(id[ses], buf, BUFLEN)) < 0) {
				printf("ERROR: mcl_send failed for session %d\n", ses);
				exit(-1);
			}
		}
	}
	printf("\ntest_tx: close the %d sessions...\n", SESSION_NB);
	for (ses = 0; ses < SESSION_NB; ses++) {
		printf(" %d ", ses);
		if ((err = mcl_close(id[ses])) < 0) {
			printf("ERROR: mcl_close failed for session %d\n", ses);
			exit(-1);
		}
	}
	printf("\n... test_tx ok\n");
	return 0;
}

