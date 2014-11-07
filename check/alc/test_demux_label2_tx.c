/* $Id: test_demux_label2_tx.c,v 1.1 2004/12/21 08:45:40 roca Exp $ */
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
 * test_demux_label2.c
 * Same as "1" version, but with large TSI identifiers, which
 * requires the use of non compact (H mode) LCT headers.
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
#define DEMUX_LABEL_0	0xFFFFFFF0
#define DEMUX_LABEL_1	0xFFFFFFF1
#define DEMUX_LABEL_2	0xFFFFFFF2
//#define BUFLEN		2400		/* buffer size */
#define BUFLEN		240		/* buffer size */
#define VERBOSITY	0		/* verbosity level */

int	ttl = 0;			/* do not leave the host */


int
main (int       argc,
      char      *argv[])
{
	int     id[SESSION_NB];
	int	dl[SESSION_NB] =			/* labels to use */
			{DEMUX_LABEL_0, DEMUX_LABEL_1, DEMUX_LABEL_2};
	int	port = 2000;				/* in host format! */
	unsigned long	addr = ntohl(inet_addr("225.1.1.1"));	/* in host format! */
	int	err;
	int	ses;
	char	buf[BUFLEN];
	int	i;
	int	verbose = VERBOSITY;

	printf("test_demux_label1_tx: create %d sending sessions...\n", SESSION_NB);
	for (i = 0; i < BUFLEN; i++)
		buf[i] = (char)i;
	for (ses = 0; ses < SESSION_NB; ses++) {
		printf(" %d ", ses);
		if ((id[ses] = mcl_open("w")) < 0) {
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
		err = mcl_ctl(id[ses], MCL_OPT_TTL, (void*)&ttl, sizeof(ttl));
		if (err != 0) {
			printf("ERROR: mcl_ctl TTL failed\n");
			exit(-1);
		}
	}
	printf("\ntest_demux_label1_tx: write on the %d sessions...\n", SESSION_NB);
	for (ses = 0; ses < SESSION_NB; ses++) {
		printf(" %d ", ses);
		sprintf(buf, "**%d**", dl[ses]);
		if ((err = mcl_send(id[ses], buf, BUFLEN)) < 0) {
			printf("ERROR: mcl_send failed for session %d\n", ses);
			exit(-1);
		}
	}
	printf("\ntest_demux_label1_tx: close the %d sessions...\n", SESSION_NB);
	for (ses = 0; ses < SESSION_NB; ses++) {
		printf(" %d ", ses);
		if ((err = mcl_close(id[ses])) < 0) {
			printf("ERROR: mcl_close failed for session %d\n", ses);
			exit(-1);
		}
	}
	printf("\n... test_demux_label1_tx ok\n");
	return 0;
}

