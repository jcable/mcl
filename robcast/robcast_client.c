/* $Id: robcast_client.c,v 1.3 2004/11/29 17:07:29 chneuman Exp $ */
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
 * robcast_client.c
 * Test MCL robustness in front of either client or server reboot.
 * A server reboot triggers a TSI update, in order to distinguish
 * the sessions before and after the reboot.
 * A client reboot must not affect reception.
 * The session follows an on-demand delivery mode in order to
 * enable asynchronous client arrivals.
 * Two MCL sessions are created: a control session (TSI=0) used by
 * the server to announce communicate the new TSI to use, and a
 * data session (using the new TSI) to convey data.
 */

#include "robcast.h"

#define VERBOSITY_CTRL	0		/* verbosity level for control session*/
#define VERBOSITY_DATA	0		/* verbosity level for data session */


int	id[SESSION_NB];			/* MCL session identifiers */
int	port[SESSION_NB];
unsigned long	addr[SESSION_NB];
int	verbose[SESSION_NB];
int	cur_TSI = 0;			/* current Transport Session Id to */
					/* use for data session */


/*
 * Create and setup a session (ctrl or data)
 */
void
setup_session (int	index)
{
	int	err;		/* error return value */
	int	unset = 0;

	if ((id[index] = mcl_open("r")) < 0)
		EXIT(("client/setup_session: ERROR: mcl_open failed for session %d\n", index))
	if (verbose[index] > 0) {
		err = mcl_ctl(id[index], MCL_OPT_VERBOSITY, (void*)&verbose[index],
				sizeof(verbose[index]));
		EXIT_ON_ERROR(err, (
		"client/setup_session: ERROR: mcl_ctl VERBOSITY failed for session %d\n", index))
	}
	/* don't use the virtual memory service here (small messages) */
	mcl_ctl(id[index], MCL_OPT_VIRTUAL_RX_MEMORY, (void*)&unset,
		sizeof(unset));
	/* Select the appropriate multicast address and port */
	err = mcl_ctl(id[index], MCL_OPT_ADDR, (void*)&addr[index], sizeof(addr[index]));
	EXIT_ON_ERROR(err, (
	"client/setup_session: ERROR, MCL_OPT_ADDR failed for session %d\n", index))
	err = mcl_ctl(id[index], MCL_OPT_PORT, (void*)&port[index], sizeof(port[index]));
	EXIT_ON_ERROR(err, (
	"client/setup_session: ERROR, MCL_OPT_PORT failed for session %d\n", index))
	if (index == CTRL_IDX) {
		int	set = 1;
		/* switch to immediate delivery mode */
		err = mcl_ctl(id[index], MCL_OPT_IMMEDIATE_DELIVERY_TO_APPLI, (void*)&set, sizeof(set));
		EXIT_ON_ERROR(err, (
		"client/setup_session: ERROR, MCL_OPT_IMMEDIATE_DELIVERY failed\n"))
	}
}


/*
 * MCL control session management.
 * Receives new TSI announcements, and controls the data session
 * accordingly.
 */
void
ctrl_mgmt (void)
{
	int	new_TSI = 0;	/* Transport Session Id just received. It can */
				/* be a reminder, or a new TSI... */
	int	ret;
	int	err;		/* error return value */
	fd_set	readfs;

	FD_ZERO(&readfs);
	FD_SET(id[CTRL_IDX], &readfs);
	/* don't wait here */
	ret = mcl_select(id[CTRL_IDX]+1, &readfs, NULL, NULL, NULL);
	if (ret < 0)
		EXIT(("client/ctrl_mgmt: ERROR, mcl_select failed"))
	if (ret == 0) {
		/* nothing received, so return */
		/*printf("client/ctrl_mgmt: no new TSI received yet\n");*/
		return;
	}
	/* something available... */
	if (ret != 1)
		EXIT(("client/ctrl_mgmt: ERROR, expected ret == 1, got %d", ret))
	if ((ret = mcl_recv(id[CTRL_IDX], &new_TSI, sizeof(new_TSI))) < 0)
		EXIT(("client/ctrl_mgmt: ERROR, control session closed"))
	if (ret == 0)
		EXIT(("client/ctrl_mgmt: ERROR, read on control session returns 0 whereas mcl_select() says there's data!"))
	new_TSI = ntohl(new_TSI);
	if (new_TSI == cur_TSI) {
		/* nothing to do... this is just a reminder */
		printf("robcast_client: recvd a reminder for TSI=%d\n", new_TSI);
		return;
	}
	/*
	 * switch to new TSI.
	 * It requires to close the close the current data
	 * session (to flush buffers), reopen a new session
	 * and specifying the new TSI to use.
	 */
	printf("robcast_client: switch to new TSI=%d...\n", new_TSI);
	mcl_close(id[DATA_IDX]);
	setup_session(DATA_IDX);
	err = mcl_ctl(id[DATA_IDX], MCL_OPT_DEMUX_LABEL, (void*)&new_TSI, sizeof(new_TSI));
	EXIT_ON_ERROR(err, (
		"client/setup_session: ERROR, MCL_OPT_DEMUX_LABEL failed for data session\n"))
}


int
main (int       argc,
      char      *argv[])
{
	char	buf[BUFLEN];
	struct timeval timeout;
	int	ret;
	fd_set	readfs;

	port[CTRL_IDX] = CTRL_PORT;	/* in host format (required by MCL) */
	port[DATA_IDX] = DATA_PORT;
	addr[CTRL_IDX] = ntohl(inet_addr(CTRL_ADDR));	/* in host format */
							/* (required by MCL) */
	addr[DATA_IDX] = ntohl(inet_addr(DATA_ADDR));
	verbose[CTRL_IDX] = VERBOSITY_CTRL;
	verbose[DATA_IDX] = VERBOSITY_DATA;

	printf("robcast_client: create and init two sessions...\n");
	setup_session(CTRL_IDX);
	setup_session(DATA_IDX);
	printf("robcast_client: wait for data now til we're stopped...\n");

	/*
	 * start data reception now.
	 */
	while (1) {
		FD_ZERO(&readfs);
		FD_SET(id[DATA_IDX], &readfs);
		timeout.tv_sec = 0;	/* 0.1 second timeout */
		timeout.tv_usec = 100000;
		ret = mcl_select(id[DATA_IDX]+1, &readfs, NULL, NULL, &timeout);
		if (ret < 0) {
			EXIT(("client/main: ERROR, mcl_select failed"))
		} else if (ret == 0) {
			/* nothing received, check the control session now... */
			/*printf("client/ctrl_mgmt: no new data received yet\n");*/
			ctrl_mgmt();
		} else if (ret != 1) {
			EXIT(("client/main: ERROR, expected ret == 1, got %d", ret))
		} else {
			if ((ret = mcl_recv(id[DATA_IDX], buf, BUFLEN)) < 0)
				EXIT(("client/main: data session closed\n"))
			printf("new message received from robcast_server:\n\t%s\n", buf);
#if 0
			/*
			 * depending on the appli, you can also stop
			 * the client once he received a data message...
			 */
			printf("\n\t... robcast_client ok\n");
			return 0;
#endif
		}
	}
	return -1;
}

