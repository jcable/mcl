/* $Id: mclsend.c,v 1.1.1.1 2003/09/03 12:45:43 chneuman Exp $ */
/*
 *  Copyright (c) 2002 INRIA - All rights reserved
 *  (main author: Vincent Roca - vincent.roca@inrialpes.fr)
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
 * mclsend.c
 *
 *	Sender (deamon) side of the mini FTP over MCL tool
 *	This example shows how to use the MCL library.
 *	It remains exactly the same while changing the
 *	internal MCL library algorithms (scheduling,
 *	reliability, etc.)
 */

#include "mclftp.h"

void	send			(void);
void	parse_commandline	(int argc, char *argv[]);
void	usage			(int argc, char *argv[]);


int 	fd;		/* input file fd */
int	id = 0;		/* mcl endpoint fd */
int	verbose = -1;
int	port = 2324;
unsigned long addr = 0;
int	ttl = 5;
#ifdef ALC
int	layer = 8;
#endif


int
main (int	argc,
      char	*argv[])
{
	parse_commandline(argc, argv);
	send();
#ifdef WIN32
	WSACleanup();
#endif
	exit(0);
}


/*
 * serious stuff is here; not too complex, isn't it?
 * NB: assumes an ordered delivery service !!!
 */
void
send (void)
{
	int	len;
	char	buf[BUFLEN];
	int	sent = 0;	/* total number of bytes sent */

	/* open as a sender... */
	if ((id = mcl_open("w")) < 0)
		EXIT(("send: mcl_open failed\n"))
	/* specify a few important parameters... */
	if (verbose >= 0) {
		int	stats = 1;	/* 1 => all intermediate stats */
		mcl_ctl(id, MCL_OPT_VERBOSITY, (void*)&verbose,sizeof(verbose));
		mcl_ctl(id, MCL_OPT_STATS, (void*)&stats, sizeof(int));
	}
	if (port > 0)		/* in host format! */
		mcl_ctl(id, MCL_OPT_PORT, (void*)&port, sizeof(port));
	if (addr > 0)		/* in host format! */
		mcl_ctl(id, MCL_OPT_ADDR, (void*)&addr, sizeof(addr));
	if (ttl > 0)
		mcl_ctl(id, MCL_OPT_TTL, (void*)&ttl, sizeof(ttl));

#ifdef ALC
	if (layer > 0)
		mcl_ctl(id, MCL_OPT_LAYER_NB, (void*)&layer, sizeof(layer));
#endif

	/* send the file content... */
	while ((len = read(fd, buf, BUFLEN)) > 0) {
		if (mcl_send(id, buf, len) < 0)
			EXIT(("send: mcl_send %d failed\n", len))
		sent += len;
	}
	if (len < 0)
		EXIT(("send: read failed, returned %d\n", len))
	PRINT_OUT((stdout, "send: %d bytes sent\n", sent))
	/* close... */
	mcl_close(id);
	/* ... and that's all folks */
}


void
parse_commandline (int	argc,
		   char	*argv[])
{
	int	c;
#ifdef SOLARIS
        extern char *optarg;
#elif defined(WIN32)
	char *optarg = NULL;
#endif

	if(argc<2)
		usage(argc, argv);

	/* last arg is file name */
	ASSERT((argv[argc-1]))
	if (strcmp(argv[argc-1], "-h") && strcmp(argv[argc-1], "-help")
			&& strcmp(argv[argc-1], "/h") && strcmp(argv[argc-1], "/?") )
	{
		if ((fd = open(argv[argc-1],
#ifdef WIN32
					_O_BINARY |
#endif
					O_RDONLY)) < 0) {
			perror("Error while parsing command line: ");
			EXIT(("Unable to open file \"%s\"\n",
				argv[argc-1]))
		}
		argc--;
	}
#ifdef WIN32
	while ((c = GetOption(argc, argv, "a:v:h", &optarg)) != 0)
#else
	while ((c = getopt(argc, argv, "a:v:h")) != EOF)
#endif
	{
		switch (c) {
		case 'a':
			if (*optarg) {
				/* process addr/port argument */
				char	rname[128];	/* remote name string */
				char	*p;

  				strncpy(rname, optarg, sizeof(rname));
				if ((p = strchr(rname, '/')) != NULL) {
					*p = '\0';
					/* in host order */
					port = (unsigned short)atoi(p+1);
				}
				/*
				 * addr must be in host byte order, but
				 * gethostbyname/inet_addr return network order
				 */
				if (isalpha((int)rname[0])) {
					if (gethostbyname(rname) == NULL)
						EXIT(("unknown host ""%s""\n", rname))
					addr = ntohl(*(unsigned long *)
						(gethostbyname(rname)->h_addr));
				} else
					addr = ntohl(inet_addr(rname));
			} else
				EXIT(( "bad argument -a%s\n", optarg))
			break;
		case 'v':	/* verbosity level */
			if (isdigit((int)*optarg))
                                verbose = atoi(optarg);
                        else
				EXIT(("bad argument -v%s\n", optarg))
			break;
		case 'h':
			usage(argc, argv);
		default:
			/*
			 * NB: getopt returns '?' when finding an
			 * unknown argument; avoid the following
			 * error msg in that case
			 */
			if (c != '?')
				PRINT_OUT((stdout, "bad argument %c\n", c))
			usage(argc, argv);
		}
	}
}


void
usage (int	argc,
       char	*argv[])
{
	if (id == 0) {		/* need an MCL endpoint first */
		if ((id = mcl_open("w")) < 0)
			EXIT(("usage: mcl_open failed\n"))
	}
	mcl_ctl(id, MCL_OPT_MOREABOUT, NULL, 0);

	PRINT_OUT((stdout, "\n\nUsage: %s [options] file_to_tx\n", argv[0]))
	PRINT_OUT((stdout, "	-h[elp]		this help\n"))

	PRINT_OUT((stdout, "	-an[/p]		set uni/multicast address or name to n and\n"))
	PRINT_OUT((stdout, "			port number to p (default 127.0.0.1/%d)\n", port))
	PRINT_OUT((stdout, "	-vn		set verbosity level to n\n"))
	exit(0);
}
