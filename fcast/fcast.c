/* $Id: fcast.c,v 1.32 2005/05/23 15:05:33 roca Exp $ */
/*
 *  Copyright (c) 1999-2002 INRIA - Universite Paris 6 - All rights reserved
 *  (main authors: Julien Laboure - julien.laboure@inrialpes.fr
 *                 Vincent Roca - vincent.roca@inrialpes.fr)
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
 * fcast.c
 *
 *	Fcast file transfert tool over MCL Multicast Library.
 */
#include "fcast.h"

static void	Usage		(int argc, char	*argv[]);
static void 	ParseCommandLine(int argc, char *argv[]);
static void	interrupted	(void);
static int	extract_addr	(char *addr_string, struct sockaddr **addr,
				 int port);



/******** Globals */

/*
 * global variables common to ALC / NORM
 */
int		id = 0;
int		verbose = 0;
int		stats = 0;
struct sockaddr	*addr = NULL;		/* session address (IPv4 or IPv6) */
struct sockaddr	*src_addr = NULL;	/* sender address  (IPv4 or IPv6) */
struct sockaddr *mcast_if_addr = NULL;	/* interface address (IPv4 or IPv6) */
char		*mcast_if_name = NULL;	/* interface name (IPv4 or IPv6) */
int		port = 9998;
int		demux_label = 0;
int		ttl = 1;
float		fec_ratio = 1.5;

int		mode = 0;
int		reuse_tx_buff = 1;

int		overwrite = PROMPT;
char		fileparam[MAX_PATH+MAX_FILENAME];
bool		recursive = false;
bool		meta_object_mode = false;
#ifdef WIN32
int		pause = 0;
int		ip_version = 4;
#endif
int		silent = 0;
int		tx_huge_file = 0;
#define	MAX_FILE_NAME_LEN	256
char		tmp_dir[MAX_FILE_NAME_LEN];
int		tmp_dir_set = 0;
int		txprof_set = 0;		/* set tx profile only once */
int		txprof_mode = MCL_TX_PROFILE_MID_RATE_INTERNET;
int		txprof_dt_size = 0;	/* default datagram size in bytes */
double		txprof_rate = 0.0;	/* default rate in pkts/s (converted
					   from bps)*/
#if defined(ALC)					   
int		congestion_control = FLID_SL_CC;
#else
int		congestion_control = 0;
#endif

#if defined(ALC)
/*
 * global variables specific to ALC
 */
int	nb_layers = 0;
int	nb_tx = 1;
int	delivery_mode = DEL_MODE_PUSH;
int	optimode = OPTIMIZE_SPEED;


#elif defined(NORM)
/*
 * global variables specific to NORM
 */
#endif /* RM_PROTOCOL */


int
main (int argc, char *argv[])
{
	int	err = 0;	/* return value of mcl_ctl functions */

	signal(SIGINT, (sighandler_t)interrupted);

#ifdef WIN32
	/* Quick and dirty hack to access WSA functions */
	if (id == 0) {		
		if ((id = mcl_open("r")) < 0)
			EXIT(("Fcast: ERROR, mcl_open failed\n"))
	}
#endif

	ParseCommandLine(argc, argv); /* Parameters parsing... */

	if (mode == SEND) {		/* sender */
		if ((id = mcl_open("w")) < 0)
			EXIT(("FcastSend: ERROR, mcl_open(w) failed\n"))
	}
	else if (mode == RECV) {	/* or receiver */
		if ((id = mcl_open("r")) < 0)
			EXIT(("FcastRecv: ERROR, mcl_open(r) failed\n"))
	} else {			/* or what? */
		PRINT(("Error: -send or -recv mode required\n"))
		Usage(argc, argv);
		return -1;
	}

	/* specify few important parameters... */
	err += mcl_ctl(id, MCL_OPT_VERBOSITY, (void*)&verbose, sizeof(verbose));
	EXIT_ON_ERROR(err, ("Fcast: ERROR, mcl_ctl failed for VERBOSITY\n"))
	err += mcl_ctl(id, MCL_OPT_STATS, (void*)&stats, sizeof(int));
	EXIT_ON_ERROR(err, ("Fcast: ERROR, mcl_ctl failed for STATS\n"))
#if defined(ALC)
	if (congestion_control > 0) {		/* must be before TX_PROFILE */
		err += mcl_ctl(id, MCL_OPT_SET_CC_SCHEME, (void*)&congestion_control,
				sizeof(congestion_control));
		EXIT_ON_ERROR(err, (
			"Flute: ERROR, mcl_ctl failed for MCL_OPT_SET_CC_SCHEME\n"))
	}
#endif /* RM_PROTOCOL */
	if (txprof_dt_size == 0) {
		err += mcl_ctl(id, MCL_OPT_TX_PROFILE, (void*)&txprof_mode,
				sizeof(txprof_mode));
		EXIT_ON_ERROR(err, (
			"Fcast: ERROR, mcl_ctl failed for TX_PROFILE\n"))
	}
	if (txprof_dt_size > 0) {	/* must be after TX_PROFILE */
		err += mcl_ctl(id, MCL_OPT_DATAGRAM_SIZE,
				(void*)&txprof_dt_size, sizeof(txprof_dt_size));
		EXIT_ON_ERROR(err, (
			"Fcast: ERROR, mcl_ctl failed for DATAGRAM_SIZE %d\n",
			txprof_dt_size))
	}
	if (txprof_rate > 0.0) {	/* must be after TX_PROFILE */
		err += mcl_ctl(id, MCL_OPT_TX_RATE,
				(void*)&txprof_rate, sizeof(txprof_rate));
		EXIT_ON_ERROR(err, (
			"Fcast: ERROR, mcl_ctl failed for TX_RATE %d\n",
			txprof_rate))
	}

#if defined(ALC)
	if (nb_layers > 0) {
		err += mcl_ctl(id, MCL_OPT_LAYER_NB, (void*)&nb_layers, sizeof(int));
		EXIT_ON_ERROR(err, ("Fcast: ERROR, mcl_ctl failed for LAYER_NB\n"))
	}
#endif /* RM_PROTOCOL */
	if (addr != NULL) {
		int	addr_len;
		addr_len = (addr->sa_family == AF_INET6) ?
				sizeof(struct sockaddr_in6) :
				sizeof(struct sockaddr_in);
		err += mcl_ctl(id, MCL_OPT_BIND, (void*)addr, addr_len);
		EXIT_ON_ERROR(err, ("Fcast: ERROR, mcl_ctl failed for BIND\n"))
	}
	if (mcast_if_addr != NULL) {		/* in host format! */
		err += mcl_ctl(id, MCL_OPT_SET_NETIF_ADDR,
				(void*)mcast_if_addr, sizeof(*mcast_if_addr));
		EXIT_ON_ERROR(err, ("Fcast: ERROR, mcl_ctl failed for SET_NETIF_ADDR\n"))
	}
#ifndef WIN32
	else if (mcast_if_name != NULL) {
		err += mcl_ctl(id, MCL_OPT_SET_NETIF_NAME,
				(void*)mcast_if_name, strlen(mcast_if_name));
		EXIT_ON_ERROR(err, ("Fcast: ERROR, mcl_ctl failed for SET_NETIF_NAME\n"))
	}
#endif /* OS */
#if defined(ALC)
	if (demux_label > 0) {
		err += mcl_ctl(id, MCL_OPT_DEMUX_LABEL, (void*)&demux_label,
				sizeof(demux_label));
		EXIT_ON_ERROR(err, (
			"Fcast: ERROR, mcl_ctl failed for DEMUX_LABEL\n"))
	}
#endif /* RM_PROTOCOL */
	if (ttl >= 0) {
		err += mcl_ctl(id, MCL_OPT_TTL, (void*)&ttl, sizeof(ttl));
		EXIT_ON_ERROR(err, (
			"Fcast: ERROR, mcl_ctl failed for TTL\n"))
	}
#if defined(ALC)
	if (nb_tx > 1) {
		err += mcl_ctl(id, MCL_OPT_NB_OF_TX, (void*)&nb_tx,
				sizeof(nb_tx));
		EXIT_ON_ERROR(err, (
			"Fcast: ERROR, mcl_ctl failed for NB_OF_TX\n"))
	}
#endif /* RM_PROTOCOL */
	if (tmp_dir_set) {
		err += mcl_ctl(id, MCL_OPT_TMP_DIR, (void*)&tmp_dir,
				strlen(tmp_dir));
		EXIT_ON_ERROR(err, (
			"Fcast: ERROR, mcl_ctl failed for TMP_DIR \"%s\"\n",
			tmp_dir))
	}
#if defined(ALC)
	err += mcl_ctl(id, MCL_OPT_DELIVERY_MODE, (void*)&delivery_mode,
						sizeof(delivery_mode));
	EXIT_ON_ERROR(err, ("Fcast: ERROR, mcl_ctl failed for DELIVERY_MODE\n"))
	if (meta_object_mode) {
		err += mcl_ctl(id, MCL_OPT_SET_META_OBJECT_ONLY_MODE, NULL, 0);
		EXIT_ON_ERROR(err, ("Fcast: ERROR, mcl_ctl failed for META_OBJECT_ONLY_MODE\n"))
	}

#endif /* RM_PROTOCOL */

	if (mode == SEND) /*sender */
		FcastSend();
	else if (mode == RECV) /* or receiver */
		FcastRecv();

#ifdef WIN32
	if (pause) {
		system("pause");
	}
	WSACleanup();
#endif
	return 0;
}



void Usage (int argc, char *argv[])
{
	int	err = 0;	/* return value of mcl_ctl functions */

	if (id == 0) {		/* need an MCL endpoint first */
		if ((id = mcl_open("r")) < 0)
			EXIT(("Fcast: Usage: ERROR, mcl_open failed\n"))
	}

	PRINT(("\nFCAST Multicast File Transfert Tool\n"))

	mcl_ctl(id, MCL_OPT_MOREABOUT, NULL, 0);
	EXIT_ON_ERROR(err, ("Fcast: Usage: mcl_ctl failed for MOREABOUT\n"))

	PRINT(("\n\n"))
	PRINT(("USAGE   (sender)   %s -send [options] file\n", argv[0]))
	PRINT(("             or    %s -send [options] -R directory\n", argv[0]))
	PRINT(("      (receiver)   %s -recv [options]\n\n", argv[0]))
	PRINT(("COMMON OPTIONS\n"))
	PRINT(("    -help           this help\n"))
	PRINT(("    -send or -recv  choose Fcast mode:  sender or receiver.\n"))
	PRINT(("    -an[/p]         set the uni/multicast IPv4/IPv6 address or hostname to n\n"))
	PRINT(("                    and port number to p (default 127.0.0.1/%d)\n", port))
#ifdef WIN32
	PRINT(("    -ipvn           IP version: 4 or 6 (default: 4)\n"))
#endif
	PRINT(("    -ifan           (InterFace Address) the network interface to use is the\n"))
	PRINT(("                    one attached to the local IPv4/IPv6 addr or host name n\n"))
	PRINT(("                    Only used on multi-homed hosts/routers.\n"))
#ifndef WIN32
	PRINT(("    -ifnstr         (InterFace Name) the network interface to use is the\n"))
	PRINT(("                    one attached to the interface name str (e.g. -ifneth0)\n"))
	PRINT(("                    Only used on multi-homed hosts/routers.\n"))
#endif /* OS */
#if defined(ALC)
	PRINT(("    -demuxn         set the LCT Transport Session Id (TSI) to n (default 0)\n"))
	PRINT(("                    - at a sender TSI is included in each packet sent\n"))
	PRINT(("                    - at a receiver {src_addr; TSI} is used for packet filtering\n"))
#elif defined(NORM)
	/* no equivalent */
#endif /* RM_PROTOCOL */
	PRINT(("    -vn             set MCL verbosity level to n (add statistics too)\n"))
	PRINT(("    -statn          set MCL statistic level to n (0: none, 1: final, 2: all)\n"))
	PRINT(("    -silent         silent mode\n"))
	PRINT(("    -tmpdir         set temporary directory to dir (e.g. -tmp/home/toto/tmp/)\n"))
	PRINT(("                    (unix default: \"/tmp\")\n"))
#if defined(ALC)
	PRINT(("    -objaggr        set object aggregation mode (AKA meta_object). Highly\n"))
	PRINT(("                    efficient when sending a large number of small objects.\n"))
	PRINT(("                    Requires the recursive tx mode at a sender.\n"))
	PRINT(("    -ospeed         use it to optimize speed (default)\n"))
	PRINT(("    -ospace         use it to reduce the required memory at receivers\n"))
	PRINT(("    -ocpu           use it if receiver is CPU limited\n"))
#elif defined(NORM)
	/* no equivalent */
#endif /* RM_PROTOCOL */
#ifdef WIN32
	PRINT(("    -P              Request user input (pause) before exiting (for win console)\n"))
#endif
	PRINT(("TRANSMISSION PROFILES\n"))
	PRINT(("    one can use either predefined profiles, or do everything by hand\n"))
	PRINT(("    or redefine some parameters of the predefined profiles\n"))
	PRINT(("    -plow | -pmed | -phigh	 predefined tx profiles for Low|Medium|High\n"))
	PRINT(("                    Speed Internet. (default: medium)\n"))
	PRINT(("                    specifies: number of layers, rate on base layer, datagram\n"))
	PRINT(("                    size, and either FLID-SL (default) or RLC cong. control\n"))
	PRINT(("    -plan           predefined tx profile for High Speed LAN\n"))
	PRINT(("                    specifies: 1 layer, rate, datagram size, and no cong.\n"))
	PRINT(("                    control\n"))
	PRINT(("    -psize[/rate]   sets one (or two) parameters:\n"))
#if defined(ALC)	
	PRINT(("                    size is the datagram size (bytes) (used by sender/recv)\n"))
	PRINT(("                    rate is the base layer tx rate (bits/s) (used by sender)\n"))
	PRINT(("    -ln             set number of layers to n\n"))
	PRINT(("    -ccn            set congestion control (CC) scheme to n:\n"))
	PRINT(("                    0 for no CC (automatically selects 1 transmission layer)\n"))
	PRINT(("                    1 for RLC (often needed for interoperability tests)\n"))
	PRINT(("                    2 for FLID_SL (default, less aggressive than RLC)\n"))
	PRINT(("                    Must be specified on both sender and receivers sides!\n"))
#endif
	PRINT(("SENDER SPECIFIC OPTIONS\n"))
	PRINT(("    -ttln           set the TTL (Time To Live) to n (default 1)\n"))
	PRINT(("    -R              recursive transmission of a whole directory content\n"))
#if defined(ALC)
	PRINT(("    -cont           continuous delivery mode (same as -repeat)\n"))
	PRINT(("                    also known as ``on-demand'' mode (default is ``push'')\n"))
	PRINT(("    -repeatn        repeat n times on each layer then stop\n"))
	PRINT(("                    ignored in ``on-demand mode''\n"))
#elif defined(NORM)
	/* no equivalent */
#endif /* RM_PROTOCOL */
	PRINT(("    -fecn           set FEC expansion ratio to n, floating point value >= 1.0.\n"))
	PRINT(("                    (default is %.2f). A ratio of 1.0 means no FEC as it is\n", fec_ratio))
	PRINT(("                    the n/k ratio, inverse of the so-called \"code rate\".\n"))
	PRINT(("RECEIVER SPECIFIC OPTIONS\n"))
	PRINT(("    -srcn           set the source IPv4/IPv6 addr or hostname to n\n"))
	PRINT(("                    {src_addr; TSI} is used for incoming packet filtering.\n"))
	PRINT(("    -int|-never|-force    set the overwriting mode:\n"))
	PRINT(("                    interactive:  user is prompted if file already exists\n"))
	PRINT(("                    never:        never overwrite an existing file\n"))
	PRINT(("                    force:        always write file, even if it already exists\n"))
	PRINT(("Type \"CTRL-C\" to abort\n"))
#ifdef WIN32
	system("pause");
#endif
	exit(0);
}



#if defined(ALC)
/*
 * ALC version
 */
static void
ParseCommandLine (int argc, char *argv[])
{
	int	c;
	char *OptList = "a:v:h:l:s:r:o:p:f:n:i:c:RPt:d:";
	bool	address_set = false;
	char	addr_str[256]; /* remote name string */

#ifdef SOLARIS
	extern char *optarg;
#elif defined(WIN32)
	char *optarg = NULL;
#endif

	if(argc < 2)
		Usage(argc, argv);


#ifdef WIN32
	while ((c = GetOption(argc, argv, OptList, &optarg)) != 0)
#else
	while ((c = getopt(argc, argv, OptList)) != EOF)
#endif
	{
		switch (c) {
		case 'a':
			/* process addr/port argument */
			char		*p;

			if (*optarg == '\0') {
				EXIT(( "Fcast: ERROR, bad argument -a%s: do not use extra space\n",
					optarg))
			}
			strncpy(addr_str, optarg, sizeof(addr_str));
			addr_str[255] = '\0';
			if ((p = strchr(addr_str, '/')) != NULL) {
				*p = '\0';
				/* in host order */
				port = (unsigned short)atoi(p+1);
			}
			address_set = true;
			break;
		case 'd':
			if (!strncmp(optarg, "emux", 4) && isdigit((int)*(optarg+4))) {
				demux_label = atoi(optarg+4);
			}
			else
				EXIT(("Fcast: ERROR, bad argument -d%s (NB: dont use space between -demux and value)\n", optarg))
			break;
		case 'v':	/* verbosity level */
			if (isdigit((int)*optarg)) {
                                verbose = atoi(optarg);
				if (verbose > 0)
					stats = 2;
			} else
				EXIT(("Fcast: ERROR, bad argument -v%s\n", optarg))
			break;
		case 'h':
			Usage(argc, argv);
			break;
		case 'l':	/* number of layers */
			if (isdigit((int)*optarg)) {
					nb_layers = atoi(optarg);
			} else
				EXIT(("Fcast: ERROR, bad argument -l%s\n", optarg))
			break;
		case 't':	/* ttl value */
			if (!strncmp(optarg, "tl", 2) &&
			    isdigit((int)*(optarg+2))) {
                                ttl = atoi(optarg+2);
			} else if(!strncmp(optarg, "mp", 2)) {
  				strncpy(tmp_dir, optarg+2, sizeof(tmp_dir));
				tmp_dir[MAX_FILE_NAME_LEN-1] = 0;
				/* add final / and \0 if not already present */
				if (tmp_dir[strlen(tmp_dir)-1] != '/' &&
				    strlen(tmp_dir) <= MAX_FILE_NAME_LEN-2)
					strcat(tmp_dir, "/");
				tmp_dir_set = 1;
			}
			else
				EXIT(("Fcast: ERROR, bad argument -t%s\n", optarg))
			break;
		case 's':
			if (!strcmp(optarg, "end"))
				mode = SEND;
			else if (!strncmp(optarg, "rc", 2)) {
				/* process addr argument */

				if (*(optarg + 2) == '\0') {
					EXIT(("Fcast: ERROR, bad argument -s%s: do not use extra space\n", optarg))
				}
  				strncpy(addr_str, optarg + 2, sizeof(addr_str));
				addr_str[255] = '\0';
				if (extract_addr(addr_str, &src_addr, 0) < 0) {
					EXIT(("Fcast: ERROR, extract_addr failed for src_addr argument -s%s\n", optarg))
				}
			} else if(!strcmp(optarg, "ilent"))
				silent = 1;
			else if(!strncmp(optarg, "tat", 3)) {
				if (isdigit((int)*(optarg+3)))
					stats = atoi(optarg+3);
				else
					EXIT(("Fcast: ERROR, bad argument -s%s\n (NB: dont use space between -stat and value)", optarg))
			} else
				EXIT(("Fcast: ERROR, bad argument -s%s\n", optarg))
			break;
		case 'r':
			if (!strcmp(optarg, "ecv"))
				mode = RECV;
			 else if(!strncmp(optarg, "epeat", 5) &&
				 isdigit((int)*(optarg+5))) {
				 	/* nb of tx is original_tx + repeat_nb*/
					nb_tx = atoi(optarg+5) + 1;
			} else
				EXIT(("Fcast: ERROR, bad argument -r%s (NB: dont use space between -repeat and value)\n", optarg))
			break;
		case 'o':
			if (!strcmp(optarg, "space")) {
				optimode = OPTIMIZE_SPACE;
			} else if(!strcmp(optarg, "speed")) {
				optimode = OPTIMIZE_SPEED;
			} else if(!strcmp(optarg, "cpu")) {
				optimode = OPTIMIZE_CPU;
			} else if(!strcmp(optarg, "bjaggr")) {
				meta_object_mode = true;
			} else {
				EXIT(("Fcast: ERROR, bad argument -o%s\n",
					optarg))
			}
			break;
		case 'p':
			if (txprof_set) {
				EXIT(("Fcast: ERROR, tx profile can be set only once\n"))
			} else {
				txprof_set = 1;
			}
			if (!strcmp(optarg, "low")) {
				txprof_mode = MCL_TX_PROFILE_LOW_RATE_INTERNET;
			} else if(!strcmp(optarg, "med")) {
				txprof_mode = MCL_TX_PROFILE_MID_RATE_INTERNET;
			} else if(!strcmp(optarg, "high")) {
				txprof_mode = MCL_TX_PROFILE_HIGH_SPEED_INTERNET;
			} else if(!strcmp(optarg, "lan")) {
				txprof_mode = MCL_TX_PROFILE_HIGH_SPEED_LAN;
				congestion_control = NO_CC;	/* true in a LAN! */
			} else if (*optarg) {
				/* process size/rate argument */
				char	str[128]; /* size/rate string */
				char	*p;
				int	rate = 0;

  				strncpy(str, optarg, sizeof(str));
				str[127] = '\0';
				if ((p = strchr(str, '/')) != NULL) {
					*p = '\0'; /* cut str into two strings*/
					rate = (int)atoi(p+1);
					if (rate <= 0) {
						EXIT(("Fcast: ERROR, invalid rate for argument -p%s\n", optarg))
					}
				}
				txprof_dt_size = (int)atoi(str);
				if (txprof_dt_size <= 0) {
					EXIT(("Fcast: ERROR, invalid size for argument -p%s\n", optarg))
				}
				if (rate > 0) {
					txprof_rate = (double)rate /
						((double)txprof_dt_size * 8.0);
					/* no less than 1 pkt/s */
					txprof_rate = max(1.0, txprof_rate);
				}
				/*PRINT(("fcast: txprof_dt_size=%d, rate=%d, txprof_rate=%f\n", txprof_dt_size, rate, txprof_rate))*/
			} else {
				EXIT(("Fcast: ERROR, bad argument -p%s\n", optarg))
			}
			break;

		case 'R':
#ifdef SOLARIS
			EXIT(("Flute: ERROR, argument -R not supported with this release on Solaris.\n"))
#endif
			recursive = 1;
			break;

		case 'P':
#ifdef WIN32
			pause = 1;
#endif
			break;

		case 'f':
			if (!strncmp(optarg, "ec", 2)) {
				if (isdigit((int)*(optarg+2)))
					fec_ratio = (float)atof(optarg+2);
				else
					EXIT(("Fcast: ERROR, bad argument -f%s (NB: dont use space between -fec and value)\n", optarg))
			} else if (!strcmp(optarg, "orce")) {
				overwrite = ALWAYS;
			} else
				EXIT(("Fcast: ERROR, bad argument -f%s\n", optarg))
			break;
		case 'c':

			if (!strcmp(optarg, "ont")) {
				delivery_mode = DEL_MODE_ON_DEMAND;
			} else if (!strncmp(optarg, "c", 1)) {
				int temp = (int)atoi(optarg+1);
				switch(temp){
					case 0: 
						congestion_control = NO_CC;
						if (nb_layers == 0) nb_layers = 1;
						break;
					case 1: 
						congestion_control = RLC_CC;
						break;
					case 2: 
						congestion_control = FLID_SL_CC;
						break;
				}
			} else
				EXIT(("Fcast: ERROR, bad argument -c%s\n", optarg))
			break;
		case 'i':
			if (!strcmp(optarg, "nt")) {
				overwrite = PROMPT;
			}
#ifndef WIN32
			else if (!strncmp(optarg, "fn", 2)) {
				/* this is an interface name */
				char	name_str[256]; /* if name string */

				if (*(optarg + 2) == '\0') {
					EXIT(("Fcast: ERROR, bad argument -i%s: do not use extra space\n", optarg))
				}
				//strncpy(name_str, optarg + 2, sizeof(name_str));
				strncpy(name_str, optarg + 2, 256);
				name_str[255] = '\0';
				mcast_if_name = (char *)
					calloc(1, strlen(name_str) + 1);
				if (mcast_if_name == NULL) {
					EXIT(("Fcast: ERROR, calloc failed during -i%s argument processing\n", optarg))
				}
				strncpy(mcast_if_name, name_str, strlen(name_str));
			}
#else
			else if (!strncmp(optarg, "pv", 2)) {
				ip_version = (int)atoi(optarg+2);
				if (ip_version != 4 && ip_version !=6)
				{
					EXIT(("Fcast: ERROR, unknown IP version %i. Specify IP version 4 or 6\n",ip_version))
				}
			}
#endif /* OS */
			else if (!strncmp(optarg, "fa", 2)) {
				/* process addr/hostname argument */
				char	addr_str[256]; /* addr/name string */

				if (*(optarg + 2) == '\0') {
					EXIT(("Fcast: ERROR, bad argument -i%s: do not use extra space\n", optarg))
				}
				strncpy(addr_str, optarg + 2, sizeof(addr_str));
				addr_str[255] = '\0';
				if (extract_addr(addr_str, &mcast_if_addr, 0) < 0) {
					EXIT(("Fcast: ERROR, address extraction failed during -i%s argument processing\n", optarg))
				}
			} else
				EXIT(("Fcast: ERROR, bad argument -i%s\n", optarg))
			break;
		case 'n':
			if (!strcmp(optarg, "ever")) {
				overwrite = NEVER;
			} else
				EXIT(("Fcast: ERROR, bad argument -n%s\n", optarg))
			break;
#ifdef WIN32
		case 1:
			break;
#endif
		default:
			/*
			 * NB: getopt returns '?' when finding an
			 * unknown argument; avoid the following
			 * error msg in that case
			 */
			if (c != '?')
				PRINT(("bad argument %c\n", c))
			Usage(argc, argv);
			break;
		}
	}

	/* last arg is file name */
	ASSERT((argv[argc-1]))
	if (mode == SEND) {
		strncpy(fileparam, argv[argc-1], MAX_PATH+MAX_FILENAME);
	}

	/* Process address string */
	if (address_set == true && extract_addr(addr_str, &addr, port) < 0) {
			EXIT(("Fcast: ERROR, extract_addr failed for session addr argument -a%s\n", addr_str))
	}

	/* Some checks */
	if ((meta_object_mode == true) && (recursive == false) && (mode == SEND)) {
		EXIT(("Fcast: ERROR, the \"object aggregation\" mode is only possible in recursive directory transmission mode.\n"))
	}
#ifndef WIN32		
	if (addr != NULL && addr->sa_family == AF_INET6 && mcast_if_name == NULL)
	{
		EXIT(("Flute: ERROR, you must specify an interface name (using -ifnstr) with IPv6\n"))
	}
#endif
}

#elif defined(NORM)

/*
 * NORM version
 */
static void
ParseCommandLine (int argc, char *argv[])
{
	int	c;
	char *OptList = "a:v:h:s:r:p:f:n:i:RPt:";

#ifdef SOLARIS
	extern char *optarg;
#elif defined(WIN32)
	char *optarg = NULL;
#endif

	if(argc < 2)
		Usage(argc, argv);


#ifdef WIN32
	while ((c = GetOption(argc, argv, OptList, &optarg)) != 0)
#else
	while ((c = getopt(argc, argv, OptList)) != EOF)
#endif
	{
		switch (c) {
		case 'a':
			if (*optarg == '\0') {
				EXIT(( "Fcast: ERROR, bad argument -a%s\n", optarg))
			}
			/* process addr/port argument */
			char		addr_str[256]; /* remote name string */
			char		*p;

			strncpy(addr_str, optarg, sizeof(addr_str));
			addr_str[255] = '\0';
			if ((p = strchr(addr_str, '/')) != NULL) {
				*p = '\0';
				/* in host order */
				port = (unsigned short)atoi(p+1);
			}
			/* get IPv4/IPv6 addr from argument */
			if (extract_addr(addr_str, &addr, port) < 0) {
				EXIT(("Fcast: ERROR, extract_addr failed for session addr argument -a%s\n", optarg))
			}
			break;

		case 'v':	/* verbosity level */
			if (isdigit((int)*optarg)) {
                                verbose = atoi(optarg);
				if (verbose > 0)
					stats = 2;
			} else
				EXIT(("Fcast: ERROR, bad argument -v%s\n", optarg))
			break;
		case 'h':
			Usage(argc, argv);
			break;
		case 't':	/* ttl value */
			if (!strncmp(optarg, "tl", 2) &&
			    isdigit((int)*(optarg+2))) {
                                ttl = atoi(optarg+2);
			} else if(!strncmp(optarg, "mp", 2)) {
  				strncpy(tmp_dir, optarg+2, sizeof(tmp_dir));
				tmp_dir[MAX_FILE_NAME_LEN-1] = 0;
				/* add final / and \0 if not already present */
				if (tmp_dir[strlen(tmp_dir)-1] != '/' &&
				    strlen(tmp_dir) <= MAX_FILE_NAME_LEN-2)
					strcat(tmp_dir, "/");
				tmp_dir_set = 1;
			}
			else
				EXIT(("Fcast: ERROR, bad argument -t%s\n", optarg))
			break;
		case 's':
			if (!strcmp(optarg, "end"))
				mode = SEND;
			else if (!strncmp(optarg, "rc", 2)) {
				/* process addr argument */
				char	addr_str[256]; /* remote name string */

  				strncpy(addr_str, optarg + 2, sizeof(addr_str));
				addr_str[255] = '\0';
				/* get IPv4/IPv6 addr from argument */
				if (extract_addr(addr_str, &addr, port) < 0) {
					EXIT(("Fcast: ERROR, extract_addr failed for src_addr argument -s%s\n", optarg))
				}
			} else if(!strcmp(optarg, "ilent"))
				silent = 1;
			else if(!strncmp(optarg, "tat", 3)) {
				if (isdigit((int)*(optarg+3)))
					stats = atoi(optarg+3);
				else
					EXIT(("Fcast: ERROR, bad argument -s%s\n (NB: dont use space between -stat and value)", optarg))
			} else
				EXIT(("Fcast: ERROR, bad argument -s%s\n", optarg))
			break;
		case 'r':
			if (!strcmp(optarg, "ecv"))
				mode = RECV;
			else
				EXIT(("Fcast: ERROR, bad argument -r%s (NB: dont use space between -repeat and value)\n", optarg))
			break;
		case 'p':
			if (!strcmp(optarg, "low")) {
				txprof_mode = MCL_TX_PROFILE_LOW_RATE_INTERNET;
			} else if(!strcmp(optarg, "med")) {
				txprof_mode = MCL_TX_PROFILE_MID_RATE_INTERNET;
			} else if(!strcmp(optarg, "high")) {
				txprof_mode = MCL_TX_PROFILE_HIGH_SPEED_INTERNET;
			} else if(!strcmp(optarg, "lan")) {
				txprof_mode = MCL_TX_PROFILE_HIGH_SPEED_LAN;
			} else if (*optarg) {
				/* process size/rate argument */
				char	str[128]; /* size/rate string */
				char	*p;
				int	rate = 0;

  				strncpy(str, optarg, sizeof(str));
				str[127] = '\0';
				if ((p = strchr(str, '/')) != NULL) {
					*p = '\0'; /* cut str into two strings*/
					rate = (int)atoi(p+1);
					if (rate <= 0) {
						EXIT(("Fcast: ERROR, invalid rate for argument -p%s\n", optarg))
					}
				}
				txprof_dt_size = (int)atoi(str);
				if (txprof_dt_size <= 0) {
					EXIT(("Fcast: ERROR, invalid size for argument -p%s\n", optarg))
				}
				if (rate > 0) {
					txprof_rate = (int)((float)rate /
						((float)txprof_dt_size * 8.0));
					/* no less than 1 pkt/s */
					txprof_rate = max(1, txprof_rate);
				}
				/*PRINT(("fcast: txprof_dt_size=%d, rate=%d, txprof_rate=%d\n", txprof_dt_size, rate, txprof_rate))*/
			} else {
				EXIT(("Fcast: ERROR, bad argument -p%s\n", optarg))
			}
			break;


		case 'R':
			recursive = true;
			break;

		case 'P':
#ifdef WIN32
			pause = 1;
#endif
			break;

		case 'f':
			if (!strncmp(optarg, "ec", 2)) {
				if (isdigit((int)*(optarg+2)))
					fec_ratio = (float)atof(optarg+2);
				else
					EXIT(("Fcast: ERROR, bad argument -f%s (NB: dont use space between -fec and value)\n", optarg))
			} else if (!strcmp(optarg, "orce")) {
				overwrite = ALWAYS;
			} else
				EXIT(("Fcast: ERROR, bad argument -f%s\n", optarg))
			break;
		case 'i':
			if (!strcmp(optarg, "nt")) {
				overwrite = PROMPT;
			} else if (!strncmp(optarg, "f", 1)) {
				/* process addr argument */
				char	addr_str[256]; /* addr/name string */

				if (*(optarg + 1) == '\0') {
					EXIT(("Fcast: ERROR, bad argument -i%s: do not use extra space\n", optarg))
				}
				strncpy(addr_str, optarg + 1, sizeof(addr_str));
				addr_str[255] = '\0';
				if (extract_addr(addr_str, &mcast_if_addr, 0) < 0) {
					/* this is an interface name */
					mcast_if_name = (char *)
						calloc(1, strlen(addr_str) + 1);
					if (mcast_if_name == NULL) {
						EXIT(("Fcast: ERROR, calloc failed during -i%s argument processing\n", optarg))
					}
					strncpy(mcast_if_name, addr_str, strlen(addr_str));
				}
			} else
				EXIT(("Fcast: ERROR, bad argument -i%s\n", optarg))
			break;
		case 'n':
			if (!strcmp(optarg, "ever")) {
				overwrite = NEVER;
			} else
				EXIT(("Fcast: ERROR, bad argument -n%s\n", optarg))
			break;
#ifdef WIN32
		case 1:
			break;
#endif
		default:
			/*
			 * NB: getopt returns '?' when finding an
			 * unknown argument; avoid the following
			 * error msg in that case
			 */
			if (c != '?')
				PRINT(("bad argument %c\n", c))
			Usage(argc, argv);
			break;
		}
	}

	/* last arg is file name */
	ASSERT((argv[argc-1]))
	if( mode == SEND )
	{
		strncpy(fileparam, argv[argc-1], MAX_PATH+MAX_FILENAME);
	}

}

#endif /* RM_PROTOCOL */


/**
 * Extract an IPv4 or IPv6 address from an address string or host name.
 * Copies the address in the sockaddr structure.
 * @param addr_string	IPv4/IPv6 address string or host name.
 * @param addr		address of the pointer to sockaddr where to store
 * 			the result. This function allocates the appropriate
 * 			sockaddr_in/sockaddr_in6 structure.
 * @param port		port number in HOST byte order to store in sockaddr.
 * @return		0 if OK, < 0 in case of error.
 */
static int
extract_addr (char		*addr_string,
	      struct sockaddr	**addr,
	      int		port)
{
// choose one resolver method according to what is available...
#if defined(LINUX)
#define HAS_GETHOSTBYNAME2	// GNU extension, ok on linux
#elif defined(SOLARIS) || defined(FREEBSD) //|| defined(WIN32)
#define HAS_GETIPNODEBYNAME	// POSIX  1003.1-2001 version
#elif defined(WIN32)
#define HAS_NOTHING			// Normal, it's WinSOCK ;-)
#endif




#ifdef HAS_GETHOSTBYNAME2
	struct hostent	*hp;

	/* search for IPv4/IPv6 hostname or address first */
	hp = gethostbyname2(addr_string, AF_INET);
	if (hp != NULL) {
		struct sockaddr_in	*sa;
		/* addr must be in host byte order, but
		 * hostent is in network order! */
		*addr = (struct sockaddr*)calloc(1, sizeof(struct sockaddr_in));
		if (*addr == NULL) {
			EXIT(("Fcast: ERROR, no memory\n"))
		}
		sa = (struct sockaddr_in*)*addr;
		sa->sin_family = AF_INET;
		sa->sin_port = htons(port);
		memcpy(&(sa->sin_addr.s_addr),
			hp->h_addr_list[0],
			sizeof(struct in_addr));
		return 0;
	}
	hp = gethostbyname2(addr_string, AF_INET6);
	if (hp != NULL) {
		struct sockaddr_in6	*sa6;
		/* addr must be in host byte order, but
		 * hostent is in network order! */
		*addr = (struct sockaddr*)
				calloc(1, sizeof(struct sockaddr_in6));
		if (*addr == NULL) {
			EXIT(("Fcast: ERROR, no memory\n"))
		}
		sa6 = (struct sockaddr_in6*)*addr;
		sa6->sin6_family = AF_INET6;
		sa6->sin6_port = htons(port);
		memcpy(&(sa6->sin6_addr), hp->h_addr_list[0],
			sizeof(struct in6_addr));
		return 0;
	}
	/* everything failed */
	PRINT(("Fcast: ERROR, unknown host ""%s""\n", addr_string))
	return -1;
#endif // HAS_GETHOSTBYNAME2
#ifdef HAS_GETIPNODEBYNAME
	struct hostent	*hp;
	int		err;

	/* search for IPv4 hostname or address first */
	hp = getipnodebyname(addr_string, AF_INET, AI_DEFAULT, &err);
	//PRINT(("search for IPv4: returned x%x, err=%d\n", hp, err))
	if (hp != NULL) {
		struct sockaddr_in	*sa;
		/* addr must be in host byte order, but
		 * hostent is in network order! */
		*addr = (struct sockaddr*)calloc(1, sizeof(struct sockaddr_in));
		if (*addr == NULL) {
			EXIT(("Fcast: ERROR, no memory\n"))
		}
		sa = (struct sockaddr_in*)*addr;
		sa->sin_family = AF_INET;
		sa->sin_port = htons(port);
		memcpy(&(sa->sin_addr.s_addr),
			hp->h_addr_list[0],
			sizeof(struct in_addr));
		return 0;
	}
	/* search for IPv6 hostname or address then */
	hp = getipnodebyname(addr_string, AF_INET6, AI_DEFAULT, &err);
	//PRINT(("search for IPv6: returned x%x, err=%d\n", hp, err))
	if (hp != NULL) {
		struct sockaddr_in6	*sa6;
		/* addr must be in host byte order, but
		 * hostent is in network order! */
		*addr = (struct sockaddr*)
				calloc(1, sizeof(struct sockaddr_in6));
		if (*addr == NULL) {
			EXIT(("Fcast: ERROR, no memory\n"))
		}
		sa6 = (struct sockaddr_in6*)*addr;
		sa6->sin6_family = AF_INET6;
		sa6->sin6_port = htons(port);
		memcpy(&(sa6->sin6_addr), hp->h_addr_list[0],
			sizeof(struct in6_addr));
		return 0;
	}
	/* everything failed */
	PRINT(("Fcast: ERROR, unknown host ""%s""\n", addr_string))
	return -1;
#endif // HAS_GETIPNODEBYNAME
#ifdef HAS_NOTHING

	if (ip_version ==4)
	{
		struct sockaddr_in	*sa;

		/* addr must be in host byte order, but
		 * hostent is in network order! */
		*addr = (struct sockaddr*)calloc(1, sizeof(struct sockaddr_in));
		if (*addr == NULL) {
			EXIT(("Fcast: ERROR, no memory\n"))
		}
		sa = (struct sockaddr_in*)*addr;
		sa->sin_family = AF_INET;
		sa->sin_port = htons(port);
		/*
		 * addr must be in host byte order, but
		 * gethostbyname/inet_addr return network order
		 */
		if (isalpha((int)addr_string[0])) {
			if (gethostbyname(addr_string) == NULL) {
				PRINT(("Fcast: ERROR, unknown host ""%s""\n", addr_string))
				PRINT(("Fcast: did you specify the right IP version (with option -ipvn)?\n"))
				free(*addr);
				*addr = NULL;
				return -1;
			}
			sa->sin_addr.s_addr = *(unsigned long *)((gethostbyname(addr_string))->h_addr);
		} else {
			sa->sin_addr.s_addr = inet_addr(addr_string);
		}
	}
	else if (ip_version == 6)
	{
		struct sockaddr_in6	*sa6;
		int len = sizeof(struct sockaddr_in6);
		/* addr must be in host byte order, but
		 * hostent is in network order! */
		*addr = (struct sockaddr*)
				calloc(1, sizeof(struct sockaddr_in6));
		if (*addr == NULL) {
			EXIT(("Fcast: ERROR, no memory\n"))
		}
		sa6 = (struct sockaddr_in6*)*addr;
		sa6->sin6_family = AF_INET6;
		if (WSAStringToAddress(addr_string, AF_INET6, NULL, (struct sockaddr*) sa6, &len) != 0)
		{
			EXIT(("Fcast: ERROR, WSAStringToAddress failed: %i\n",  WSAGetLastError()))
		}
		sa6->sin6_port = htons(port);
	}
	return 0;
#endif // HAS_NOTHING
}


/*
 * we received a SIGINT...
 */
static void
interrupted (void)
{
	static int	abort_in_progress = 0;

	if (!abort_in_progress) {
		abort_in_progress = 1;
		PRINT(("fcast: aborted, call mcl_abort()\n"));
		mcl_abort(id);
		PRINT(("fcast: aborted, exit\n"));
		exit(2);
	} /* else do nothing */
}
