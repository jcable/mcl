/* $Id: test_several_fec_codecs.c,v 1.7 2004/06/15 15:37:19 roca Exp $ */
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
 * test_several_fec_codes.c, both sides...
 * Principle:
 *	The source uses several FEC codecs, one per object.
 *	The receivers decodes each of them using the appropriate codec.
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

//#define BUFLEN	200000		/* buffer size; must be large enough */
#define BUFLEN		200003		/* buffer size; must be large enough */
					/* 200003 is a prime number, usefull */
					/* to better test the code */
#define VERBOSITY_TX	0		/* verbosity level for sender */
#define VERBOSITY_RX	0		/* verbosity level for receiver */
#define NB_OBJECTS	8		/* total number of objects to tx */

int		port = 2000;				/* in host format */
int		verbose;
int		ttl = 0;			/* do not leave the host */


void sender	 (void);
void receiver	 (void);
void init_buffer (char *buf);
void set_fec_ratios (int id, bool sender);


int
main (int       argc,
      char      *argv[])
{
	if (argc != 2) {
		printf("test_several_fec_codes: ERROR: bad arg count %d; \"-send\" or \"-recv\" expected\n", argc);
		exit(-1);
	}
	if (strncmp(argv[1], "-send", strlen("-send")) == 0) {
		sender();
	} else if (strncmp(argv[1], "-recv", strlen("-recv")) == 0) {
		receiver();
	} else {
		printf("test_several_fec_codes: ERROR: unknown arg %s; \"-send\" or \"-recv\" expected\n", argv[1]);
		exit(-1);
	}
	return 0;
}


void
sender ()
{
	int     id;
	UINT32	addr = ntohl(inet_addr("225.1.0.0")); /* host format */
	int	err;
	char	buf[BUFLEN];
	int	codec_idx;	/* fec codec to use */
	int	sched;		/* sched to use */
	int	obj_count;

	printf("test_several_fec_codecs: SENDER\n");
	verbose = VERBOSITY_TX;
	if ((id = mcl_open("w")) < 0) {
		printf("test_several_fec_codes: ERROR: mcl_open failed for sender\n");
		exit(-1);
	}
	err = mcl_ctl(id, MCL_OPT_PORT, (void*)&port, sizeof(port));
	if (err != 0) {
		printf("test_several_fec_codes: ERROR: mcl_ctl PORT failed for sender\n");
		exit(-1);
	}
	err = mcl_ctl(id, MCL_OPT_ADDR, (void*)&addr, sizeof(addr));
	if (err != 0) {
		printf("test_several_fec_codes: ERROR: mcl_ctl ADDR failed for sender\n");
		exit(-1);
	}
	if (verbose > 0) {
		err = mcl_ctl(id, MCL_OPT_VERBOSITY, (void*)&verbose, sizeof(verbose));
		if (err != 0) {
			printf("test_several_fec_codes: ERROR: mcl_ctl VERBOSITY failed for sender\n");
			exit(-1);
		}
	}
	err = mcl_ctl(id, MCL_OPT_TTL, (void*)&ttl, sizeof(ttl));
	if (err != 0) {
		printf("test_several_fec_codes: ERROR: mcl_ctl TTL failed\n");
		exit(-1);
	}
	set_fec_ratios(id, true);
	/*
	 * use the basic scheduler, since with FEC_NULL there is no
	 * FEC DU at all, while schedulers 2 and 3 assume there are some!
	 */
	sched = MCL_SCHED_LCT1;
	err = mcl_ctl(id, MCL_OPT_SCHED, (void*)&sched, sizeof(sched));
	if (err != 0) {
		printf("test_several_fec_codes: ERROR: mcl_ctl MCL_OPT_SCHED failed for sender\n");
		exit(-1);
	}

	/* fill in the buffer */
	init_buffer(buf);

	/* don't send immediately but wait... */
	err = mcl_ctl(id, MCL_OPT_KEEP_DATA, NULL, 0);
	if (err != 0) {
		printf("ERROR: mcl_ctl KEEP_DATA failed\n");
		exit(-1);
	}

	/* submit all objects, changing the codec for each of them */
	codec_idx = MCL_FEC_SCHEME_NULL;	/* start with NULL codec */
	for (obj_count = NB_OBJECTS; obj_count > 0; obj_count--) {
		err = mcl_ctl(id, MCL_OPT_SET_FEC_CODE, (void*)&codec_idx,
				sizeof(codec_idx));
		if (err != 0) {
			printf("WARNING: codec %d non-available... skipped\n",
				codec_idx);
		} else {
			if ((err = mcl_send(id, buf, BUFLEN)) < 0) {
				printf("test_several_fec_codes: ERROR: 1st mcl_send failed for sender\n");
				exit(-1);
			}
			printf("SENDER: object, %d bytes, with FEC %d encoding sent\n", err, codec_idx);
		}
		codec_idx = (codec_idx + 1) % MCL_FEC_SCHEME_NB; /* switch to following codec */
	}

	/* and now send... */
	err = mcl_ctl(id, MCL_OPT_PUSH_DATA, NULL, 0);
	if (err != 0) {
		printf("ERROR: mcl_ctl PUSH_DATA failed\n");
		exit(-1);
	}

	if ((err = mcl_close(id)) < 0) {
		printf("test_several_fec_codes: ERROR: mcl_close failed for sender\n");
		exit(-1);
	}
	printf("test_several_fec_codecs: SENDER ok\n");
	exit(0);
}


void
receiver ()
{
	int     id;
	UINT32	addr = ntohl(inet_addr("225.1.0.0")); /* host format */
	int	err;
	char	buf[BUFLEN];		/* buffer containing received data */
	char	ref_buf[BUFLEN];	/* reference buffer */
	int	i;

	printf("test_several_fec_codecs: RECEIVER\n");
	verbose = VERBOSITY_RX;
	if ((id = mcl_open("r")) < 0) {
		printf("test_several_fec_codes: ERROR: mcl_open failed for receiver\n");
		exit(-1);
	}
	err = mcl_ctl(id, MCL_OPT_PORT, (void*)&port, sizeof(port));
	if (err != 0) {
		printf("test_several_fec_codes: ERROR: mcl_ctl PORT failed for receiver\n");
		exit(-1);
	}
	err = mcl_ctl(id, MCL_OPT_ADDR, (void*)&addr, sizeof(addr));
	if (err != 0) {
		printf("test_several_fec_codes: ERROR: mcl_ctl ADDR failed for receiver\n");
		exit(-1);
	}
	if (verbose > 0) {
		err = mcl_ctl(id, MCL_OPT_VERBOSITY, (void*)&verbose, sizeof(verbose));
		if (err != 0) {
			printf("test_several_fec_codes: ERROR: mcl_ctl VERBOSITY failed for receiver\n");
			exit(-1);
		}
	}
	/* no need to set FEC ratio at RX, carried in EXT_FTI header with
	 * most FEC codes */
	//set_fec_ratios(id, true);

	/* fill in the reference buffer */
	init_buffer(ref_buf);

	for (i = 0; ; i++) {
		if ((err = mcl_recv(id, buf, BUFLEN)) < 0) {
			break;	/* finished */
		}
		if (strncmp(buf, ref_buf, BUFLEN) != 0) {
			printf("test_several_fec_codes: ERROR: bad object received\n");
			exit(-1);
		}
		printf("RECEIVER: object %d, %d bytes, received ok\n", i, err);
	}
	if ((err = mcl_close(id)) < 0) {
		printf("test_several_fec_codes: ERROR: mcl_close failed for receiver\n");
		exit(-1);
	}
	printf("test_several_fec_codecs: RECEIVER ok\n");
	exit(0);
}


/*
 * Fill in the buffer.
 */
void
init_buffer (char	*buf)
{
	int	*ptr;
	int	i;

	for (i = BUFLEN/4, ptr = (int*)buf; i > 0; i--, ptr++) {
		*ptr = i;
	}
}


/*
 * Init the FEC ratios of the various FEC codes.
 * Used by the sender and also by the receiver with the NULL and RSE codes.
 */
void
set_fec_ratios (int	id,
		bool	sender)
{
	int	codec_idx;	/* fec codec to use */
	float   fec_ratio;	/* fec ratio to use */
	int	err;

	for (codec_idx = MCL_FEC_SCHEME_NULL;
	     codec_idx < MCL_FEC_SCHEME_NB;
	     codec_idx++) {
		if (sender == false &&
		    codec_idx != MCL_FEC_SCHEME_NULL &&
		    codec_idx != MCL_FEC_SCHEME_RSE_129_0) {
			/* not required */
			continue;
		}
		err = mcl_ctl(id, MCL_OPT_SET_FEC_CODE, (void*)&codec_idx,
				sizeof(codec_idx));
		if (err != 0) {
			//printf("WARNING: codec %d non-available... skipped\n",
			//	codec_idx);
			continue;
		}
		fec_ratio = 1.5;
		err = mcl_ctl(id, MCL_OPT_FEC_RATIO, (void*)&fec_ratio, sizeof(fec_ratio));
		if (err != 0) {
			printf("test_several_fec_codes: ERROR: mcl_ctl MCL_OPT_FEC_RATIO failed for %s\n", (sender ? "sender" : "receiver"));
			exit(-1);
		}
	}
}
