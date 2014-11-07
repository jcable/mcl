/* $Id: robcast_server.c,v 1.2 2004/11/29 17:07:29 chneuman Exp $ */
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
 * robcast_server.c
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
int	cur_TSI = -1;			/* current Transport Session Id to */
					/* use for data session */
int	cur_TOI = -1;			/* current Transport Object Id to */
					/* use to carry the TSI announcement */


/*
 * Create and setup a session (ctrl or data)
 */
void
setup_session (int	index)
{
	int	err;		/* error return value */
	int	delivery_mode = DEL_MODE_ON_DEMAND;
	int	unset = 0;

	if ((id[index] = mcl_open("w")) < 0)
		EXIT(("server/setup_session: ERROR: mcl_open failed for session %d\n", index))
	if (verbose[index] > 0) {
		err = mcl_ctl(id[index], MCL_OPT_VERBOSITY, (void*)&verbose[index],
				sizeof(verbose[index]));
		EXIT_ON_ERROR(err, (
		"server/setup_session: ERROR: mcl_ctl VERBOSITY failed for session %d\n", index))
	}
	/* don't use the virtual memory service here (small messages) */
	mcl_ctl(id[index], MCL_OPT_VIRTUAL_TX_MEMORY, (void*)&unset,
		sizeof(unset));
	/* Select the appropriate multicast address and port */
	err = mcl_ctl(id[index], MCL_OPT_ADDR, (void*)&addr[index], sizeof(addr[index]));
	EXIT_ON_ERROR(err, (
		"server/setup_session: ERROR, MCL_OPT_ADDR failed for session %d\n", index))
	err = mcl_ctl(id[index], MCL_OPT_PORT, (void*)&port[index], sizeof(port[index]));
	EXIT_ON_ERROR(err, (
		"server/setup_session: ERROR, MCL_OPT_PORT failed for session %d\n", index))
	/* Switch to "on demand" delivery mode */
	err = mcl_ctl(id[index], MCL_OPT_DELIVERY_MODE, (void*)&delivery_mode,
			sizeof(delivery_mode));
	EXIT_ON_ERROR(err, (
		"server/setup_session: ERROR, MCL_OPT_DELIVERY_MODE failed\n"))
	if (index == CTRL_IDX) {
		float	fec = 1.0;	/* n/k = 1.0 means no FEC */
		/* no need for FEC here */
		err = mcl_ctl(id[index], MCL_OPT_FEC_RATIO, (void*)&fec, sizeof(fec));
		EXIT_ON_ERROR(err, (
		"server/setup_session: ERROR, MCL_OPT_FEC_RATIO failed\n"))

	}
}


/*
 * MCL control session management.
 * Retrieves the new TSI to use for the data session (i.e. TSI of the
 * previous data session + 1)
 * and the new TOI to use for the control session (i.e. TSI of the
 * previous control session + 1)
 * which are both stored on a permanent location (i.e. a file), then
 * send the TSI periodically in the control session (in case a client
 * is restarted) in the object of identifier TOI.
 */
void
ctrl_mgmt (void)
{
	int	buf[1];			/* buffer sent on ctrl session which */
					/* contains TSI in network format */
	char	tsi_file_name[] = "./robcast_server_repository.txt";
					/* name of permanent TSI depository */
	int	fd_tsi = -1;		/* fd to file containing the TSI value*/
	FILE	*FILE_tsi = NULL;	/* ptr to file containing TSI value */
	int	ret;
	int	err;			/* error return value */

	/*
	 * First, retrieve the TSI to use. If the variable cur_TSI is not set,
	 * (e.g. after a reboot), then retrieve it from disk file, if any,
	 * or create this file.
	 */
	if (cur_TSI == -1) {
		/* server has been restarted, so search the TSI from disk */
		if ((fd_tsi = open(tsi_file_name, O_RDWR)) < 0) {
			if (errno != ENOENT) {
				perror("server/ctrl_mgmt: open failed 1");
				EXIT(("server/ctrl_mgmt: ERROR while opening file %s\n", tsi_file_name))
			}
			/* does not exist */
			cur_TSI = 1;	/* reset TSI (must be >= 0) */
			cur_TOI = 1;	/* reset TSI (must be >= 1) */
			printf("\t\trobcast_server: no TSI/TOI repository, create it\n");

			if ((fd_tsi = open( tsi_file_name, O_WRONLY | O_CREAT, S_IRWXU)) < 0) {
				perror("server/ctrl_mgmt: open failed 2");
				EXIT(("server/ctrl_mgmt: ERROR while creating file %s\n", tsi_file_name))
			}
			if ((FILE_tsi = fdopen(fd_tsi, "w")) == NULL) {
				perror("server/ctrl_mgmt: fdopen failed");
				EXIT(("server/ctrl_mgmt: ERROR, fdopen failed\n"))
			}
		} else {
			/* read the value, increment it, and save it */
			if ((FILE_tsi = fdopen(fd_tsi, "r+")) == NULL) {
				perror("server/ctrl_mgmt: fdopen failed");
				EXIT(("server/ctrl_mgmt: ERROR, fdopen failed\n"))
			}
			if ((ret = fscanf(FILE_tsi, "next_TSI=%d  next_TOI=%d", &cur_TSI, &cur_TOI)) != 2) {
				perror("server/ctrl_mgmt: fscanf failed");
				EXIT(("server/ctrl_mgmt: ERROR, fscanf failed\n"))
			}
			rewind(FILE_tsi);
			printf("\trobcast_server: found the TSI/TOI repository\n");
		}
		/* store the next TSI and TOI to use, flush it and close */
		fprintf(FILE_tsi, "next_TSI=%d  next_TOI=%d", cur_TSI+1, cur_TOI+1);
		fflush(FILE_tsi);
		if (fclose(FILE_tsi) != 0) {
			perror("server/ctrl_mgmt: fclose failed");
			EXIT(("server/ctrl_mgmt: ERROR, fclose failed\n"))
		}
		/* fclose() implicitely issues a close() too */
		/* set the new TSI on the data session */
		printf("\trobcast_server: switch to new TSI=%d\n", cur_TSI);
		err = mcl_ctl(id[DATA_IDX], MCL_OPT_DEMUX_LABEL, (void*)&cur_TSI, sizeof(cur_TSI));
		EXIT_ON_ERROR(err, (
		"server/ctrl_mgmt: ERROR, MCL_OPT_DEMUX_LABEL failed for data session\n"))
		/* set the new TOI for the control session */
		printf("\trobcast_server: switch to new TOI=%d\n", cur_TOI);
		err = mcl_ctl(id[CTRL_IDX], MCL_OPT_SET_NEXT_TOI, (void*)&cur_TOI, sizeof(cur_TOI));
		EXIT_ON_ERROR(err, (
		"server/ctrl_mgmt: ERROR, MCL_OPT_SET_NEXT_TOI failed for control session\n"))
	}
	/*
	 * Secondly, send the new TSI to clients...
	 */
	buf[0] = htonl(cur_TSI);
	if ((ret = mcl_send(id[CTRL_IDX], &buf, sizeof(buf))) < 0)
		EXIT(("server/ctrl_mgmt: ERROR, control session closed"))
	/*printf("\t\trobcast_server: TSI msg sent on ctrl session\n");*/
}


int
main (int       argc,
      char      *argv[])
{
	char	buf[BUFLEN];
	int	ret;

	port[CTRL_IDX] = CTRL_PORT;	/* in host format (required by MCL) */
	port[DATA_IDX] = DATA_PORT;
	addr[CTRL_IDX] = ntohl(inet_addr(CTRL_ADDR));	/* in host format */
							/* (required by MCL) */
	addr[DATA_IDX] = ntohl(inet_addr(DATA_ADDR));
	verbose[CTRL_IDX] = VERBOSITY_CTRL;
	verbose[DATA_IDX] = VERBOSITY_DATA;

	printf("\trobcast_server: create and init two sessions...\n");
	setup_session(CTRL_IDX);
	setup_session(DATA_IDX);

	/*
	 * start data transmission now.
	 */
	ctrl_mgmt();
	printf("\trobcast_server: send data and wait til we're stopped...\n");
	sprintf(buf, "Hi all, this is an HELLO message sent on data session/TSI=%d", cur_TSI);
	if ((ret = mcl_send(id[DATA_IDX], buf, strlen(buf)+1)) < 0)
		EXIT(("server/main: data session closed\n"))
	/*
	 * and now loop forever while MCL-ALC sends data in on-demand mode...
	 */
	while (1)
	{
#ifdef WIN32
		Sleep(1);
#else
		sleep(1);
#endif
	}
	return 0;
}

