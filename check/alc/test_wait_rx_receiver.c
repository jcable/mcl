/* $Id: test_wait_rx_receiver.c,v 1.3 2004/11/29 17:07:27 chneuman Exp $ */
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
 * test_wait_tx.c
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

#define BUFLEN		4096		/* buffer size */
#define VERBOSITY	0		/* verbosity level */

int
main (int       argc,
      char      *argv[])
{
	int     id;
	int	port = 2000;				/* in host format! */
	unsigned long	addr = ntohl(inet_addr("225.1.0.0"));	/* in host format! */
	int	err;
	char	buf[BUFLEN];
	int	verbose = VERBOSITY;
	int	never_leave_base_layer = 1;

	if ((id = mcl_open("r")) < 0) {
		printf("ERROR: mcl_open failed\n");
		exit(-1);
	}
	err = mcl_ctl(id, MCL_OPT_PORT, (void*)&port, sizeof(port));
	if (err != 0) {
		printf("ERROR: mcl_ctl PORT failed\n");
		exit(-1);
	}
	err = mcl_ctl(id, MCL_OPT_ADDR, (void*)&addr, sizeof(addr));
	if (err != 0) {
		printf("ERROR: mcl_ctl ADDR failed\n");
		exit(-1);
	}
	err = mcl_ctl(id, MCL_OPT_NEVER_LEAVE_BASE_LAYER,
		      (void*)&never_leave_base_layer,
		      sizeof(never_leave_base_layer));
	if (err != 0) {
		printf("ERROR: mcl_ctl NEVER_LEAVE_BASE_LAYER failed\n");
		exit(-1);
	}
	if (verbose > 0) {
		err = mcl_ctl(id, MCL_OPT_VERBOSITY, (void*)&verbose, sizeof(verbose));
		if (err != 0) {
			printf("ERROR: mcl_ctl VERBOSITY failed\n");
			exit(-1);
		}
	}
	printf("\nreceiver: recv\n");
	if ((err = mcl_recv(id, buf, BUFLEN)) < 0) {
		printf("ERROR: mcl_recv failed\n");
		exit(-1);
	}
	printf("\nreceiver: wait end of rx now...\n");
	if ((err = mcl_wait_event(id, MCL_WAIT_EVENT_END_RX)) < 0) {
		printf("ERROR: mcl_wait_event END_RX failed\n");
		exit(-1);
	}
	printf("\nreceiver: ... wait returns, all rx must be finished...\n");
	printf("\nreceiver: wait CLOSE msg now...\n");
	if ((err = mcl_wait_event(id, MCL_WAIT_EVENT_CLOSED)) < 0) {
		printf("ERROR: mcl_wait_event CLOSED failed\n");
		exit(-1);
	}
	printf("\nreceiver: ... wait returns, CLOSE received...\n");
	printf("\nreceiver: close\n");
	if ((err = mcl_close(id)) < 0) {
		printf("ERROR: mcl_close failed\n");
		exit(-1);
	}
	return 0;
}
