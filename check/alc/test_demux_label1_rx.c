/* $Id: test_demux_label1_rx.c,v 1.5 2004/08/03 06:35:56 roca Exp $ */
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
 * test_demux_label1_rx.c
 * for the receiver side...
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

#define SESSION_NB	3		/* 2 sessions max */
#define DEMUX_LABEL_0	0
#define DEMUX_LABEL_1	1
#define DEMUX_LABEL_2	2
#define BUFLEN		2400		/* buffer size */
#define VERBOSITY	0		/* verbosity level */


int
main (int       argc,
      char      *argv[])
{
	int     id[SESSION_NB];
	int	dl[SESSION_NB] =			/* labels to use */
			{DEMUX_LABEL_0, DEMUX_LABEL_1, DEMUX_LABEL_2};
	int	port = 2000;				/* in host format! */
	UINT32  addr = ntohl(inet_addr("225.1.1.1"));	/* in host format! */
	int	err;
	int	ses;
	char	buf[BUFLEN];
	int	i;
	int	verbose = VERBOSITY;
	int	dl_rx;					/* value recv'ed */

	printf("test_demux_label1_rx: create %d receiving sessions...\n", SESSION_NB);
	for (i = 0; i < BUFLEN; i++)
		buf[i] = (char)i;
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
		if (verbose > 0) {
			err = mcl_ctl(id[ses], MCL_OPT_VERBOSITY, (void*)&verbose, sizeof(verbose));
			if (err != 0) {
				printf("ERROR: mcl_ctl VERBOSITY failed for session %d\n", ses);
				exit(-1);
			}
		}
		err = mcl_ctl(id[ses], MCL_OPT_DEMUX_LABEL, (void*)&dl[ses], sizeof(dl[ses]));
		if (err != 0) {
			printf("ERROR: mcl_ctl DEMUX_LABEL failed for session %d\n", ses);
			exit(-1);
		}
#if 0
		port += 20;	/* incr port and addr */
		addr += 20;
#endif
	}

	printf("\ntest_demux_label1_rx: finish the init of the %d sessions...\n",
			SESSION_NB);
	for (ses = 0; ses < SESSION_NB; ses++) {
		if ((err = mcl_recv(id[ses], NULL, 0)) < 0) {
			printf("ERROR: empty mcl_recv failed for session %d\n", ses);
			exit(-1);
		}
	}

	printf("\ntest_demux_label1_rx: read on the %d sessions...\n",
			SESSION_NB);
	for (ses = 0; ses < SESSION_NB; ses++) {
		printf("recv on session %d... ", ses);
		if ((err = mcl_recv(id[ses], buf, BUFLEN)) < 0) {
			printf("ERROR: mcl_recv failed for session %d\n", ses);
			exit(-1);
		}
		err = sscanf(buf, "**%d**", &dl_rx);
		if (err != 1) {
			printf("ERROR: sscanf failed for session %d, err=%d\n",
				ses, err);
			exit(-1);
		}
		printf("found value %d in received object\n", dl_rx);
		if (dl_rx != dl[ses]) {
			printf("ERROR: rx bad value for session %d, expected %d\n",
				ses, dl[ses]);
			exit(-1);
		}
	}

	printf("\ntest_demux_label1_rx: close the %d sessions...\n",
			SESSION_NB);
	for (ses = 0; ses < SESSION_NB; ses++) {
		printf("close session %d... ", ses);
		if ((err = mcl_close(id[ses])) < 0) {
			printf("ERROR: mcl_close failed for session %d\n", ses);
			exit(-1);
		}
	}
	printf("\n... test_demux_label1_rx ok\n");
	return 0;
}

