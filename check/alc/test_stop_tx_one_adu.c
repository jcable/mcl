/* $Id: test_stop_tx_one_adu.c,v 1.2 2005/03/01 11:03:22 moi Exp $ */
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
 * 
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

#define OBJECT_NB	500		/* total number of objects */
#define NB_STOPPED	50		/* number of objects to stop tx */
#define BUFLEN		64		/* buffer size; not too large for */
					/* increased speed */
#define VERBOSITY	0		/* verbosity level */
#define SEED		2334

int	ttl = 0;			/* do not leave the host */


int
main (int       argc,
      char      *argv[])
{
	int     id;
	int	port = 2000;				/* in host format! */
	UINT32	addr = ntohl(inet_addr("225.1.0.0"));	/* in host format! */
	int	err;
	int	obj;
	char	buf[BUFLEN];
	int	i;
	int	verbose = VERBOSITY;
	float	fec_ratio = 2.0;	/* no need for a high fec ratio */
	int	txprof_mode = MCL_TX_PROFILE_HIGH_SPEED_LAN;
	//int	txprof_mode = MCL_TX_PROFILE_MID_RATE_INTERNET;
	//int	txprof_mode = MCL_TX_PROFILE_HIGH_SPEED_INTERNET;

	printf("test_stop_tx_one_adu: test with %d successive stops...\n", NB_STOPPED);
	fflush(NULL);
	if ((id = mcl_open("w")) < 0) {
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
	err += mcl_ctl(id, MCL_OPT_FEC_RATIO, (void*)&fec_ratio, sizeof(fec_ratio));
	if (err != 0) {
		printf("ERROR: mcl_ctl FEC_RATIO failed\n");
		exit(-1);
	}
	err += mcl_ctl(id, MCL_OPT_TX_PROFILE, (void*)&txprof_mode, sizeof(txprof_mode));
	if (err) {
		printf("ERROR: mcl_ctl TX_PROFILE failed\n");
		exit(-1);
	}
	err += mcl_ctl(id, MCL_OPT_VERBOSITY, (void*)&verbose, sizeof(verbose));
	if (err != 0) {
		printf("ERROR: mcl_ctl VERBOSITY failed\n");
		exit(-1);
	}
	err = mcl_ctl(id, MCL_OPT_TTL, (void*)&ttl, sizeof(ttl));
	if (err != 0) {
		printf("ERROR: mcl_ctl TTL failed\n");
		exit(-1);
	}

	for (i = 0; i < BUFLEN; i++)
		buf[i] = (char)i;
	for (obj = 0; obj < OBJECT_NB; obj++) {
		
		if ((err = mcl_send(id, buf, BUFLEN)) < 0) {
			printf("ERROR: mcl_send failed\n");
			exit(-1);
		}

	}
	
	/* Now randomly cancel NB_STOPPED of them */
	srand(SEED);
	UINT64 toi;
	for (obj = 0; obj < NB_STOPPED; obj++) {
		toi =(rand()% (NB_STOPPED - 1)) + 1;
		printf(" %d ", toi);
		if((err = mcl_ctl(id, MCL_OPT_STOP_TRANSMITTING_ADU, (void *)&toi, sizeof(toi))) <0)
		{
			printf("ERROR: MCL_OPT_STOP_TRANSMITTING_ADU failed for object %d\n", toi);
			exit(-1);	
		}
	}

	if ((err = mcl_close(id)) < 0) {
		printf("ERROR: mcl_close failed\n");
		exit(-1);
	}

	printf("\n... test_reset_transmission ok\n");
	return 0;
}

