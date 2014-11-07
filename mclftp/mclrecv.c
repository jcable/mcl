/* $Id: mclrecv.c,v 1.1.1.1 2003/09/03 12:45:43 chneuman Exp $ */
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
 * mclrecv.c
 *
 *	Receiver side of the mini FTP over MCL tool.
 *	This example shows how to use the MCL library.
 *	It remains exactly the same while changing the
 *	internal MCL library algorithms (scheduling,
 *	reliability, etc.)
 */

#include "mclftp.h"

void	mclget			(void);
void	parse_commandline	(int argc, char *argv[]);
void	mclget_usage		(int argc, char *argv[]);


int 	fd;		/* input file fd */
int	id = 0;		/* mcl endpoint fd */
int	verbose = -1;
#ifdef ALC
int	layers = 0;
#endif
int	port = 2324;
unsigned long	addr = 0;


int
main (int	argc,
      char	*argv[])
{
	parse_commandline(argc, argv);
	mclget();
	exit(0);
}


/*
 * serious stuff is here; not too complex, isn't it?
 * NB: assumes an ordered delivery service !!!
 */
void
mclget (void)
{
	int	len;
	char	buf[BUFLEN];
	int	received = 0;	/* total number of bytes received */

	/* open as a sender... */
	if ((id = mcl_open("r")) < 0)
		EXIT(("mclget: mcl_open failed\n"))
	/* specify a few important parameters... */
	if (verbose >= 0) {
		int	stats = 2;	/* 2 => all intermediate stats */
		mcl_ctl(id, MCL_OPT_VERBOSITY, (void*)&verbose,sizeof(verbose));
		mcl_ctl(id, MCL_OPT_STATS, (void*)&stats, sizeof(stats));
	}
#ifdef ALC
	if (layers >= 0)
		mcl_ctl(id, MCL_OPT_LAYER_NB, (void*)&layers, sizeof(layers));
#endif
	if (port > 0)		/* in host format! */
		mcl_ctl(id, MCL_OPT_PORT, (void*)&port, sizeof(port));
	if (addr > 0)		/* in host format! */
		mcl_ctl(id, MCL_OPT_ADDR, (void*)&addr, sizeof(addr));
	/* get the file content... */
	while ((len = mcl_recv(id, buf, BUFLEN)) > 0) {
		write(fd, buf, len);
		received += len;
		PRINT_OUT((stdout, "mclget: %d bytes read, total=%d\n", len, received))
	}
	PRINT_OUT((stdout, "mclget: %d bytes received\n", received))
	/* close... */
	mcl_close(id);
	/* ... and that's all folks */
}


void
parse_commandline (int	argc,
		   char	*argv[])
{
	int	c;
	char	filename[256];
#ifdef SOLARIS
        extern char *optarg;
#elif defined(WIN32)
	char *optarg = NULL;
#endif
#ifdef ALC
	const char	*getopt_string = "a:v:l:h";
#else
	const char	*getopt_string = "a:v:h";
#endif

#ifdef WIN32
	strncpy(filename, tempnam(".\\", "mclftp."), 256);
#else
	sprintf(filename, "/tmp/mclftp_%d", (int)getpid());
#endif /* OS_DEP */

	PRINT_OUT((stdout, "mclget: output file is %s\n", filename))

#ifdef WIN32
	if ((fd = open(	filename, O_WRONLY | O_CREAT | O_BINARY, _S_IWRITE) ) < 0)
#else
	if ((fd = open(	filename, O_WRONLY | O_CREAT, S_IRWXU)) < 0)
#endif
	{
		perror("Error : ");
		EXIT(("Error while opening file \"%s\"\n", filename))
	}

#ifdef WIN32
	while ((c = GetOption(argc, argv, getopt_string, &optarg)) != 0)
#else

	while ((c = getopt(argc, argv, getopt_string)) != EOF)
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
#ifdef ALC
		case 'l':	/* number of layers */
			if (isdigit((int)*optarg))
                                layers = atoi(optarg);
                        else
				EXIT(("bad argument -l%s\n", optarg))
			break;
#endif
		case 'h':
			mclget_usage(argc, argv);
		default:
			/*
			 * NB: getopt returns '?' when finding an
			 * unknown argument; avoid the following
			 * error msg in that case
			 */
			if (c != '?')
				PRINT_OUT((stdout, "bad argument %c\n", c))
			mclget_usage(argc, argv);
		}
	}
}


void
mclget_usage (int	argc,
	      char	*argv[])
{
	if (id == 0) {		/* need an MCL endpoint first */
		if ((id = mcl_open("r")) < 0)
			EXIT(("mclget_usage: mcl_open failed\n"))
	}
	mcl_ctl(id, MCL_OPT_MOREABOUT, NULL, 0);

	PRINT_OUT((stdout, "\n\nUsage: %s [options]\n", argv[0]))

#ifdef WIN32
	PRINT_OUT((stdout, "	Receive and save a file in .\\mclftp* where '*' is an unique suffix\n"))
#else
	PRINT_OUT((stdout, "	Receive and save a file in /tmp/mclfile_`pid`\n"))
#endif
	
	PRINT_OUT((stdout, "	-h[elp]		this help\n"))
	PRINT_OUT((stdout, "	-an[/p]		set uni/multicast address or name to n and\n"))
	PRINT_OUT((stdout, "			port number to p (default 127.0.0.1/%d)\n", port))
#ifdef ALC
	PRINT_OUT((stdout, "	-ln		set number of layers to n\n"))
#endif
	PRINT_OUT((stdout, "	-vn		set verbosity level to n\n"))
	exit(0);
}
