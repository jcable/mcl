/* $Id: test_multi_sessions2.c,v 1.2 2004/09/22 14:10:21 chneuman Exp $ */
/*
 *  Copyright (c) 1999-2003 INRIA - Universite Paris 6 - All rights reserved
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
 * test_multi_sessions2.c
 * for the receiver side, no read...
 */

#include <stdio.h>
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
#endif /* OS_DEP */

#include <stdlib.h>			/* for exit */
#include "../../src/common/mcl_lib_api.h"

#ifdef SOLARIS		/* with solaris the max # of opened fd is much lower! */
#define SESSION_NB	5		/* number of sessions */
#else
#define SESSION_NB	10		/* number of sessions */
#endif
#define VERBOSITY	0 		/* verbosity level */

int
main (int       argc,
      char      *argv[])
{
	int     id[SESSION_NB];
	int	port = 2000;				/* in host format! */
	unsigned long	addr = ntohl(inet_addr("230.0.0.0"));	/* in host format! */
	int	err;
	int	ses;
	int	verbose = VERBOSITY;

	printf("test_multi_sessions2: create %d receiving sessions...\n", SESSION_NB);
	for (ses = 0; ses < SESSION_NB; ses++) {
		printf(" %d ", ses);
		if ((id[ses] = mcl_open("r")) < 0) {
			printf("ERROR: mcl_open failed for session %d\n", ses);
			exit(-1);
		}
		err = mcl_ctl(id[ses], MCL_OPT_PORT, (void*)&port, sizeof(port));
		if (err != 0) {
			printf("ERROR: mcl_ctl PORT failed for session %d\n", ses);
			exit(-1);
		}
		err = mcl_ctl(id[ses], MCL_OPT_ADDR, (void*)&addr, sizeof(addr));
		if (err != 0) {
			printf("ERROR: mcl_ctl ADDR failed for session %d\n", ses);
			exit(-1);
		}
		if (verbose >= 0) {
			err = mcl_ctl(id[ses], MCL_OPT_VERBOSITY, (void*)&verbose, sizeof(verbose));
			if (err != 0) {
				printf("ERROR: mcl_ctl VERBOSITY failed for session %d\n", ses);
				exit(-1);
			}
		}
		port += 20;	/* incr port and addr */
		addr += 20;
	}
	printf("\ntest_multi_sessions2: finish init of the %d sessions with null recv...\n", SESSION_NB);
	for (ses = 0; ses < SESSION_NB; ses++) {
		printf(" %d ", ses);
		if ((err = mcl_recv(id[ses], NULL, 0)) < 0) {
			printf("ERROR: mcl_recv failed for session %d\n", ses);
			exit(-1);
		}
	}
	printf("\ntest_multi_sessions2: close the %d sessions...\n", SESSION_NB);
	for (ses = 0; ses < SESSION_NB; ses++) {
		printf(" %d ", ses);
		if ((err = mcl_close(id[ses])) < 0) {
			printf("ERROR: mcl_close failed for session %d\n", ses);
			exit(-1);
		}
	}
	printf("\n... test_multi_sesions2 ok\n");
	return 0;
}

