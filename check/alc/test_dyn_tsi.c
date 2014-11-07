/* $Id: test_dyn_tsi.c,v 1.6 2004/06/15 15:37:19 roca Exp $ */
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
 * test_dyn_tsi.c, both sides...
 * Principle:
 *	The source uses TSI==1 to announce the new TSI in the first object,
 *	then changes to this new TSI, and sends a second object.
 *	The receiver starts with TSI==1, retrieves the new TSI, changes to
 *	this TSI dynamically, and receives the second object.
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
#include <string.h>
#include "../../src/common/mcl_lib_api.h"

#define DEMUX_LABEL_0	1		/* initial TSI */
#define DEMUX_LABEL_1	99		/* new TSI */
#define BUFLEN		2400		/* buffer size */
#define TX_VERBOSITY	0		/* verbosity level */
#define RX_VERBOSITY	0		/* verbosity level */
#define NB_OBJECTS_WITH_NEW_TSI  50
#define	FEC_RATIO_STEP1	4.0		/* high ratio for step 1 */
#define	FEC_RATIO_STEP2	2.0		/* normal fec ratio for step 2 */


int		port = 2000;				/* in host format */
int		verbose;
char		object_string[] = "And this is the second object";
int		ttl = 0;			/* do not leave the host */


void sender (void);
void receiver (void);


int
main (int       argc,
      char      *argv[])
{
	if (argc != 2) {
		printf("ERROR: bad arg count %d; \"-send\" or \"-recv\" expected\n", argc);
		exit(-1);
	}
	if (strncmp(argv[1], "-send", strlen("-send")) == 0) {
		sender();
	} else if (strncmp(argv[1], "-recv", strlen("-recv")) == 0) {
		receiver();
	} else {
		printf("ERROR: unknown arg %s; \"-send\" or \"-recv\" expected\n", argv[1]);
		exit(-1);
	}
	return 0;
}


void
sender ()
{
	int     id;
	int	tsi[2] = {DEMUX_LABEL_0, DEMUX_LABEL_1}; /* labels to use */
	UINT32  addr = ntohl(inet_addr("225.1.0.0")); /* host format */
	int	err;
	char	buf[BUFLEN];
	int	fec_code;  /* FEC codec to use (RSE for 1st part) */
	float   fec_ratio; /* fec ratio to use in step 1 (high) and 2 (normal)*/
	int	i;

	printf("test_dyn_tsi: SENDER, 1st step...\n");
	verbose = TX_VERBOSITY;
	if ((id = mcl_open("w")) < 0) {
		printf("ERROR: mcl_open failed for sender\n");
		exit(-1);
	}
	err = mcl_ctl(id, MCL_OPT_PORT, (void*)&port, sizeof(port));
	if (err != 0) {
		printf("ERROR: mcl_ctl PORT failed for sender\n");
		exit(-1);
	}
	err = mcl_ctl(id, MCL_OPT_ADDR, (void*)&addr, sizeof(addr));
	if (err != 0) {
		printf("ERROR: mcl_ctl ADDR failed for sender\n");
		exit(-1);
	}
	if (verbose > 0) {
		err = mcl_ctl(id, MCL_OPT_VERBOSITY, (void*)&verbose, sizeof(verbose));
		if (err != 0) {
			printf("ERROR: mcl_ctl VERBOSITY failed for sender\n");
			exit(-1);
		}
	}
	err = mcl_ctl(id, MCL_OPT_TTL, (void*)&ttl, sizeof(ttl));
	if (err != 0) {
		printf("ERROR: mcl_ctl TTL failed\n");
		exit(-1);
	}
	/*
	 * use RSE in this test (high FEC ratio/small block required for the
	 * first part of test, but other choices could be possible for the 
	 * second part)
	 */
	fec_code = MCL_FEC_SCHEME_RSE_129_0;
	if (mcl_ctl(id, MCL_OPT_SET_FEC_CODE, (void*)&fec_code,
			sizeof(fec_code))) {
		printf("ERROR: RSE FEC codec available but required for this test\n");
		exit(-1);
	}
	/*
	 * 1st step: send the new TSI as an object
	 */
	err = mcl_ctl(id, MCL_OPT_DEMUX_LABEL, (void*)&tsi[0], sizeof(tsi[0]));
	if (err != 0) {
		printf("ERROR: 1st mcl_ctl DEMUX_LABEL failed for sender\n");
		exit(-1);
	}

	fec_ratio = FEC_RATIO_STEP1;	/* high ratio for step 1 */
	err = mcl_ctl(id, MCL_OPT_FEC_RATIO, (void*)&fec_ratio, sizeof(fec_ratio));
	if (err != 0) {
		printf("ERROR: mcl_ctl MCL_OPT_FEC_RATIO failed for sender\n");
		exit(-1);
	}
	sprintf(buf, "**%d**", tsi[1]);
	if ((err = mcl_send(id, buf, strlen(buf))) < 0) {
		printf("ERROR: 1st mcl_send failed for sender\n");
		exit(-1);
	}
	/*
	 * wait a bit untill all packets have been sent before actually
	 * changing of TSI. Without it, there could be a risk that some
	 * packets of object 1 be sent with the second TSI!
	 */
	if ((err = mcl_wait_event(id, MCL_WAIT_EVENT_END_TX)) < 0) {
		printf("ERROR: mcl_wait_event END_TX failed\n");
		exit(-1);
	}
	printf("test_dyn_tsi: SENDER, 2nd step...\n");
	/*
	 * 2nd step: change of TSI and send several objects
	 */
	err = mcl_ctl(id, MCL_OPT_DEMUX_LABEL, (void*)&tsi[1], sizeof(tsi[1]));
	if (err != 0) {
		printf("ERROR: 2nd mcl_ctl DEMUX_LABEL failed for sender\n");
		exit(-1);
	}
	fec_ratio = FEC_RATIO_STEP2;	/* normal fec ratio for step 2 */
	err = mcl_ctl(id, MCL_OPT_FEC_RATIO, (void*)&fec_ratio, sizeof(fec_ratio));
	if (err != 0) {
		printf("ERROR: mcl_ctl MCL_OPT_FEC_RATIO failed for sender\n");
		exit(-1);
	}
	memset(buf, 1, BUFLEN);	/* required to avoid purify UMR error reports */
	sprintf(buf, object_string);
	for (i = 0; i < NB_OBJECTS_WITH_NEW_TSI; i++) {
		if ((err = mcl_send(id, buf, BUFLEN)) < 0) {
			printf("ERROR: 2nd mcl_send failed for sender\n");
			exit(-1);
		}
	}
	if ((err = mcl_close(id)) < 0) {
		printf("ERROR: mcl_close failed for sender\n");
		exit(-1);
	}
	printf("test_dyn_tsi: sender ok\n");
	exit(0);
}


void
receiver ()
{
	int     id;
	int	tsi = DEMUX_LABEL_0;	/* label to use first */
	UINT32  addr = ntohl(inet_addr("225.1.0.0")); /* host format */
	int	err;
	char	buf[BUFLEN];
	int	fec_code;  /* FEC codec to use (RSE for 1st part) */
	float   fec_ratio; /* fec ratio to use in step 1 (high) and 2 (normal)*/
	int	i;

#define	TEST_2_STEPS	/* TEST_2_STEPS should be defined */
#ifdef TEST_2_STEPS
	printf("test_dyn_tsi: RECEIVER, 1st step...\n");
#else
	printf("test_dyn_tsi: RECEIVER, test directly 2nd step...\n");
#endif
	verbose = RX_VERBOSITY;
	if ((id = mcl_open("r")) < 0) {
		printf("ERROR: mcl_open failed for receiver\n");
		exit(-1);
	}
	err = mcl_ctl(id, MCL_OPT_PORT, (void*)&port, sizeof(port));
	if (err != 0) {
		printf("ERROR: mcl_ctl PORT failed for receiver\n");
		exit(-1);
	}
	err = mcl_ctl(id, MCL_OPT_ADDR, (void*)&addr, sizeof(addr));
	if (err != 0) {
		printf("ERROR: mcl_ctl ADDR failed for receiver\n");
		exit(-1);
	}
	if (verbose > 0) {
		err = mcl_ctl(id, MCL_OPT_VERBOSITY, (void*)&verbose, sizeof(verbose));
		if (err != 0) {
			printf("ERROR: mcl_ctl VERBOSITY failed for receiver\n");
			exit(-1);
		}
	}
	/*
	 * use RSE in this test.
	 * specifying it here is required to specify the FEC ratio of the
	 * right FEC code.
	 */
	fec_code = MCL_FEC_SCHEME_RSE_129_0;
	if (mcl_ctl(id, MCL_OPT_SET_FEC_CODE, (void*)&fec_code,
			sizeof(fec_code))) {
		printf("ERROR: RSE FEC codec available but required for this test\n");
		exit(-1);
	}
#ifdef TEST_2_STEPS
	/*
	 * 1st step: receive the new TSI (sent as an object)
	 */
	/* specifying the FEC ratio at a receiver is required with RSE */
	fec_ratio = FEC_RATIO_STEP1;	/* high ratio for step 1 */
	err = mcl_ctl(id, MCL_OPT_FEC_RATIO, (void*)&fec_ratio, sizeof(fec_ratio));
	if (err != 0) {
		printf("ERROR: mcl_ctl MCL_OPT_FEC_RATIO failed for receiver\n");
		exit(-1);
	}
	err = mcl_ctl(id, MCL_OPT_DEMUX_LABEL, (void*)&tsi, sizeof(tsi));
	if (err != 0) {
		printf("ERROR: 1st mcl_ctl DEMUX_LABEL failed for receiver\n");
		exit(-1);
	}
	if ((err = mcl_recv(id, buf, BUFLEN)) < 0) {
		printf("ERROR: 1st mcl_recv failed for receiver\n");
		exit(-1);
	}
	sscanf(buf, "**%d**", &tsi);
	if (tsi != DEMUX_LABEL_1) {
		printf("test_dyn_tsi: receiver: ERROR: bad TSI received (%d received, %d expected)\n", tsi, DEMUX_LABEL_1);
		exit(-1);
	}
	printf("test_dyn_tsi: RECEIVER, 2nd step, change for TSI==%d...\n", tsi);
#else
	tsi = 99;	/* switch immediately to 2nd step */
#endif
	/*
	 * 2nd step: change of TSI and receive several objects
	 */
	/* specifying the FEC ratio at a receiver is required with RSE */
	fec_ratio = FEC_RATIO_STEP2;	/* normal fec ratio for step 2 */
	err = mcl_ctl(id, MCL_OPT_FEC_RATIO, (void*)&fec_ratio, sizeof(fec_ratio));
	if (err != 0) {
		printf("ERROR: mcl_ctl MCL_OPT_FEC_RATIO failed for receiver\n");
		exit(-1);
	}
	err = mcl_ctl(id, MCL_OPT_DEMUX_LABEL, (void*)&tsi, sizeof(tsi));
	if (err != 0) {
		printf("ERROR: 2nd mcl_ctl DEMUX_LABEL failed for receiver\n");
		exit(-1);
	}
#ifdef TEST_2_STEPS
#else
	if ((err = mcl_recv(id, buf, BUFLEN)) < 0) {
		printf("ERROR: 1st mcl_recv failed for receiver\n");
		exit(-1);
	}
#endif
	for (i = 0; i < NB_OBJECTS_WITH_NEW_TSI; i++) {
		if ((err = mcl_recv(id, buf, BUFLEN)) < 0) {
			printf("ERROR: 2nd mcl_recv failed for receiver\n");
			exit(-1);
		}
		if (strncmp(buf, object_string, strlen(object_string)) != 0) {
			printf("ERROR: bad 2nd object:\n\t\"%s\" received, \"%s\" expected\n", buf, object_string);
			exit(-1);
		} else {
			printf(" object %d rx/ok ", i);
		}
	}
	if ((err = mcl_close(id)) < 0) {
		printf("ERROR: mcl_close failed for receiver\n");
		exit(-1);
	}
	printf("test_dyn_tsi: receiver ok\n");
	exit(0);
}


