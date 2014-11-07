/* $Id: main.cpp,v 1.5 2005/05/23 11:11:26 roca Exp $ */
/*
 *  Copyright (c) 2003-2004 INRIA - All rights reserved
 *  main authors: Christoph Neumann - christoph.neumann@inrialpes.fr
 *		  Vincent Roca - vincent.roca@inrialpes.fr
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
 * flute.c
 *
 *	Flute file transfert tool over the Flute Library.
 */
#include "../src/flute_lib/FluteAPI.h"
#include "fsend.h"
#include "frecv.h"
#include "display.h"
#include "macros.h"
#ifdef WIN32
#include "getopt.h"
#endif

static void	Usage	(int argc, char	*argv[]);
static void 	ParseCommandLine (int argc, char *argv[]);
static void	interrupted (void);

/* the flute session class */
class Flute	*myflute = NULL;
class FluteSender	*myflutesender = NULL;
class FluteReceiver	*myflutereceiver = NULL;

/* some global parameters */
#ifdef WIN32
bool	pause = 0; /* Request user input (pause) before exiting  or not */
#endif
bool 	interactive = false;
char	**fileparam = NULL;
int		nb_tx = 1;

int main (int argc, char *argv[])
{

	/* variables for display thread*/
	flute_thread_t thread_display;

	signal(SIGINT, (sighandler_t)interrupted);

	/* Parameters parsing...
	 * This call also creates the sender or receiver FLUTE session
	 */
	ParseCommandLine(argc, argv);

#ifdef WIN32
	if (myflute->getVerbosity() == 0 && myflute->getStatsLevel() == 0) {
		CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE) display, (void*)NULL, 0, (LPDWORD)&thread_display);
	}
#else
	if (myflute->getVerbosity() == 0 && myflute->getStatsLevel() == 0) {
		pthread_create(&thread_display, NULL, display, (void *)NULL);
	}
#endif


	if (myflute->isSender())
	{
		myflutesender = (class FluteSender *) myflute;
		FluteSend();
	}
	else if (myflute->isReceiver())
	{
		myflutereceiver = (class FluteReceiver *) myflute;
		FluteRecv();
	}
	else if (myflute->isSender() == false && myflute->isReceiver() == false) 
	{
		PRINT(("Error: -send or -recv mode required\n"))
		Usage(argc, argv);
		return -1;
	}

#ifndef WIN32		
	if (myflute->getVerbosity() == 0 && myflute->getStatsLevel() == 0) pthread_cancel(thread_display);	
#endif

#ifdef WIN32
	if (pause) {
		system("pause");
	}
	WSACleanup();
#endif

	/* now free and delete everything */
	if (myflute->isSender())
	{
		delete myflutesender;	
	}
	else if (myflute->isReceiver())
	{
		delete myflutereceiver;	
	}
	else delete myflute;

	return 0;
}


void Usage (int argc, char *argv[])
{

	PRINT(("\nFLUTE Multicast File Transfert Tool\n"))

	if (myflute == NULL || myflute->isReceiver() == true) {
		myflute = new FluteSender();
	}
	myflute->about();

	PRINT(("\n\n"))
	PRINT(("USAGE   (sender)   %s -send [options] {file|directory}\n", argv[0]))
	PRINT(("      (receiver)   %s -recv [options] \n\n", argv[0]))
	PRINT(("COMMON OPTIONS (NB: do not use extra space in arguments)\n"))
	PRINT(("    -help           this help\n"))
	PRINT(("    -send or -recv  choose Flute mode:  sender or receiver.\n"))
	PRINT(("    -an[/p]         set uni/multicast IPv4/IPv6 address or name to n and\n"))
	PRINT(("                    port number to p (default 127.0.0.1/%d)\n", myflute->getPort()))
#ifdef WIN32
	PRINT(("    -ipvn           IP version, 4 or 6 (Win32 specific) (default: 4)\n"))
#endif
	PRINT(("    -ssm            use SSM (source specific multicast) (default: no)\n"))
	PRINT(("    -ifan           (InterFace Address) the network interface to use is the\n"))
	PRINT(("                    one attached to the local IPv4/IPv6 addr or host name n\n"))
	PRINT(("                    Only used on multi-homed hosts/routers.\n"))
	PRINT(("    -ifnstr         (InterFace Name) the network interface to use is the\n"))
	PRINT(("                    one attached to the interface name str (e.g. -ifneth0)\n"))
	PRINT(("                    Only used on multi-homed hosts/routers.\n"))
	PRINT(("    -demuxn         set the LCT Transport Session Id (TSI) to n (default 0)\n"))
	PRINT(("                    - at a sender TSI is included in each packet sent\n"))
	PRINT(("                    - at a receiver {src_addr; TSI} is used for packet filtering\n"))
	PRINT(("    -vn             set (MCL) verbosity level to n (add statistics too)\n"))
	PRINT(("    -statn          set (MCL) statistic level to n (0: none, 1: final, 2: all)\n"))
	PRINT(("    -tmpdir         the temporary directory is dir (string)\n"))
	PRINT(("                    (unix default: \"/tmp\")\n"))
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
	PRINT(("                    size is the datagram size (bytes) (used by sender/recv)\n"))
	PRINT(("                    rate is the base layer tx rate (bits/s) (used by sender)\n"))
#if defined(ALC)	
	PRINT(("    -ln             set number of layers to n\n"))
	PRINT(("    -ccn            set congestion control (CC) scheme to n:\n"))
	PRINT(("                    0 for no CC (automatically selects 1 transmission layer)\n"))
	PRINT(("                    1 for RLC (often needed for interoperability tests)\n"))
	PRINT(("                    2 for FLID_SL (default, less aggressive than RLC)\n"))
	PRINT(("                    Must be specified on both sender and receivers sides!\n"))
#endif	
	PRINT(("SENDER SPECIFIC OPTIONS\n"))
	PRINT(("    -ttln           set the ttl (time to live) to n (default 1)\n"))
	PRINT(("    -cont           continuous delivery mode (same as -repeat)\n"))
	PRINT(("                    also known as ``on-demand'' mode (default is ``push'')\n"))
	PRINT(("    -repeatn        repeat n times on each layer then stop\n"))
	PRINT(("                    ignored in ``on-demand mode''\n"))
	PRINT(("    -fecn           set FEC expansion ratio to n, floating point value >= 1.0.\n"))
	PRINT(("                    (default is %.2f). A ratio of 1.0 means no FEC as it is\n", ((FluteSender *)myflute)->getFECRatio()))
	PRINT(("                    the n/k ratio, inverse of the so-called \"code rate\".\n"))
	PRINT(("    -createsdpstr   create an SDP file for that session\n"))
	PRINT(("                    the result will be written to the file str (string)\n"))

	PRINT(("RECEIVER SPECIFIC OPTIONS\n"))
	PRINT(("    -srcn          set the IPv4/IPv6 source address or name to n (default 0)\n"))
	PRINT(("                   Mandatory in SSM mode.\n"))
	PRINT(("                   {src_addr; TSI} is used for incoming packet filtering.\n"))
	PRINT(("    -parsesdpstr   parse an SDP file, of pathname str (string), for that \n"))
	PRINT(("                   session and set the parameters accordingly\n"))	
#ifndef WIN32
	PRINT(("    -int	   interactive mode where the user select files to be received\n"))
	PRINT(("                   (default is to receive all files and overwrite existing\n"))
	PRINT(("                   files when needed).\n"))
	PRINT(("                   Move with up/down arrows, select with ENTER.\n"))
#else
	PRINT(("    -int|-never|-force    set the overwriting mode:\n"))
	PRINT(("                    interactive:  user is prompted if file already exists\n"))
	PRINT(("                    never:        never overwrite an existing file\n"))
	PRINT(("                    force:        always write file, even if it already exists\n"))
#endif
#ifndef WIN32
	PRINT(("Type \"q\" to abort FLUTE\n"))
#else
	PRINT(("Type \"CTRL-C\" to abort FLUTE\n"))
	system("pause");
#endif

	/* now free and delete everything */
	if (myflute->isSender())
	{
		delete (class FluteSender *)myflute;	
	}
	else if (myflute->isReceiver())
	{
		delete (class FluteReceiver *)myflute;	
	}
	else delete myflute;

	exit(0);
}


/*
 * 
 */
void ParseCommandLine (int argc, char *argv[])
{
	int	c;
	char *OptList = "a:v:h:l:s:r:p:f:n:i:c:RPt:d:";
	bool create_sdp = false;
	char sdp_file[MAX_PATH + MAX_FILENAME];

#ifdef SOLARIS
	extern char *optarg;
#elif defined(WIN32)
	char *optarg = NULL;
	int optind = 0;
#endif

	if(argc < 2) {
		Usage(argc, argv);
	}

	for (int i = 0; i < argc; i++) {
		if (!strcmp(argv[i], "-recv"))
				myflute = new FluteReceiver();
		else if (!strcmp(argv[i], "-send"))
				myflute = new FluteSender();
	}
	/* We must select sender or receiver */
	if (myflute == NULL) {
		EXIT(("Flute: ERROR, you must specify either -send or -recv\n"))
	}

#ifdef WIN32
	while ((c = GetOption(argc, argv, OptList, &optarg, &optind)) != 0 && c != 1)
#else
	while ((c = getopt(argc, argv, OptList)) != EOF)
#endif
	{
		switch (c) {
		case 'a':
			/* process addr/port argument */
			char		*p;
			char		session_addr_str[256];
			int 		port;
			
			if (*optarg == '\0') {
				EXIT(("Flute: ERROR, bad argument -a%s: do not use extra space\n",
					optarg))
			}
			strncpy(session_addr_str, optarg, sizeof(session_addr_str));
			session_addr_str[255] = '\0';
			if ((p = strchr(session_addr_str, '/')) != NULL) {
				*p = '\0';
				/* in host order */
				port = (unsigned short)atoi(p+1);
				myflute->setPort(port);
			}
			myflute->setSessionAddr(session_addr_str);
			break;
		case 'd':
			if (!strncmp(optarg, "emux", 4) && isdigit((int)*(optarg+4))) {
				int tsi = atoi(optarg+4);
				myflute->setTSI(tsi);
			}
			else
				EXIT(("Flute: ERROR, bad argument -d%s (NB: dont use space between -demux and value)\n", optarg))
			break;
		case 'v':	/* verbosity level */
			if (isdigit((int)*optarg)) {
               			int verbosity = atoi(optarg);
				myflute->setVerbosity(verbosity);
			} else
				EXIT(("Flute: ERROR, bad argument -v%s\n", optarg))
			break;
		case 'h':
			Usage(argc, argv);
			break;
		case 'l':	/* number of layers */
			if (isdigit((int)*optarg)) {
                		int nb_layers = atoi(optarg);
				myflute->setNbLayers(nb_layers);
			} else
				EXIT(("Flute: ERROR, bad argument -l%s\n", optarg))
			break;
		case 't':	/* ttl value */
			if (!strncmp(optarg, "tl", 2) &&
			    isdigit((int)*(optarg+2))) {
                                int ttl = atoi(optarg+2);
				((FluteSender *) myflute)->setTTL(ttl);
			} else if(!strncmp(optarg, "mp", 2)) {
				char	tmp_dir[MAX_PATH + MAX_FILENAME];
  				strncpy(tmp_dir, optarg+2, sizeof(tmp_dir));
				tmp_dir[MAX_PATH + MAX_FILENAME-1] = 0;
				/* add final / and \0 if not already present */
				if (tmp_dir[strlen(tmp_dir)-1] != '/' &&
				    strlen(tmp_dir) <= MAX_PATH + MAX_FILENAME -2)
					strcat(tmp_dir, "/");
				((FluteReceiver *) myflute)->setTmpDir(tmp_dir);
			}
			else
				EXIT(("Flute: ERROR, bad argument -t%s\n", optarg))
			break;
		case 's':
			if (!strncmp(optarg, "end", 3)) {
				/* already processed */
			} else if (!strncmp(optarg, "sm", 2)) {
				bool	set = true;
				((FluteReceiver *) myflute)->setSSM(set);
			} else if (!strncmp(optarg, "rc", 2)) {
				/* process addr argument */
				char	source_addr_str[256];
				if (*(optarg + 2) == '\0') {
					EXIT(("Flute: ERROR, bad argument -s%s: do not use extra space\n", optarg))
				}
  				strncpy(source_addr_str, optarg + 2, sizeof(source_addr_str));
				source_addr_str[255] = '\0';
				((FluteReceiver *) myflute)->setSrcAddr(source_addr_str);

			} else if(!strncmp(optarg, "tat", 3)) {
				if (isdigit((int)*(optarg+3)))
				{
					int stats = atoi(optarg+3);
				   	myflute->setStatsLevel(stats);
				}
				else
					EXIT(("Flute: ERROR, bad argument -s%s\n (NB: dont use space between -stat and value)", optarg))
			} else
				EXIT(("Flute: ERROR, bad argument -s%s\n", optarg))
			break;
		case 'r':
			if (!strncmp(optarg, "ecv", 3)) {
				/* already processed */
			} else if(!strncmp(optarg, "epeat", 5) &&
				 isdigit((int)*(optarg+5))) {
				 	/* nb of tx is original_tx + repeat_nb*/
					nb_tx = atoi(optarg+5) + 1;
			} else
				EXIT(("Flute: ERROR, bad argument -r%s (NB: dont use space between -repeat and value)\n", optarg))
			break;
		case 'p':
			if (!strcmp(optarg, "low")) {
				myflute->setTxProfile(MCL_TX_PROFILE_LOW_RATE_INTERNET);
			} else if(!strcmp(optarg, "med")) {
				myflute->setTxProfile(MCL_TX_PROFILE_MID_RATE_INTERNET);
			} else if(!strcmp(optarg, "high")) {
				myflute->setTxProfile(MCL_TX_PROFILE_HIGH_SPEED_INTERNET);
			} else if(!strcmp(optarg, "lan")) {
				myflute->setTxProfile(MCL_TX_PROFILE_HIGH_SPEED_LAN);
			} else if(!strncmp(optarg, "arsesdp", 7)) {
  				strncpy(sdp_file, optarg+7, sizeof(sdp_file));
				sdp_file[MAX_PATH + MAX_FILENAME-1] = 0;
				myflute->parseSdp(sdp_file);
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
						EXIT(("Flute: ERROR, invalid rate for argument -p%s (must be > 0)\n", optarg))
					}
				}
				int txprof_dt_size = (int)atoi(str);
				if (txprof_dt_size <= 0) {
					EXIT(("Flute: ERROR, invalid size for argument -p%s\n", optarg))
				}
				myflute->setDatagramSize(txprof_dt_size);
				myflute->setTxRate(rate);
			} else {
				EXIT(("Flute: ERROR, bad argument -p%s\n", optarg))
			}
			break;
			
		case 'P':
#ifdef WIN32
			pause = 1;
#endif
			break;

		case 'f':
			if (!strncmp(optarg, "ec", 2)) {
				if (isdigit((int)*(optarg+2))) {
					float fec_ratio = (float)atof(optarg+2);
					((FluteSender*) myflute)->setFECRatio(fec_ratio);
				} else
					EXIT(("Flute: ERROR, bad argument -f%s (NB: dont use space between -fec and value)\n", optarg))
#ifdef WIN32			
			} else if (!strcmp(optarg, "orce")) {
				((FluteReceiver*) myflute)->setOverwriteFile(ALWAYS);
#endif
			} else
				EXIT(("Flute: ERROR, bad argument -f%s\n", optarg))
			break;
#ifdef WIN32		
		case 'n':
			if (!strcmp(optarg, "ever")) {
				((FluteReceiver*) myflute)->setOverwriteFile(NEVER);
			} else
				EXIT(("Flute: ERROR, bad argument -n%s\n", optarg))
			break;
#endif
		case 'c':
			if (!strcmp(optarg, "ont")) {
				nb_tx = CONTINUOUS;
			} else if(!strncmp(optarg, "reatesdp", 8)) {
  				strncpy(sdp_file, optarg+8, sizeof(sdp_file));
				sdp_file[MAX_PATH + MAX_FILENAME-1] = 0;
				create_sdp = true;
			} else if (!strncmp(optarg, "c", 1)) {
				int temp = (int)atoi(optarg+1);
				switch(temp){
					case 0: 
						myflute->setCCScheme(NO_CC);
						break;
					case 1: 
						myflute->setCCScheme(RLC_CC);
						break;
					case 2: 
						myflute->setCCScheme(FLID_SL_CC);
						break;
				}
			} else
				EXIT(("Flute: ERROR, bad argument -c%s\n", optarg))
			break;
		case 'i':
#ifndef WIN32
			if (!strcmp(optarg, "nt")) {
				interactive=true;
				((FluteReceiver *)myflute)->setOverwriteFile(PROMPT);
			} else if (!strncmp(optarg, "fn", 2)) {
				/* this is an interface name */
				char	name_str[256]; /* if name string */

				if (*(optarg + 2) == '\0') {
					EXIT(("Flute: ERROR, bad argument -i%s: do not use extra space\n", optarg))
				}
				strncpy(name_str, optarg + 2, sizeof(name_str));
				strncpy(name_str, optarg + 2, 256);
				name_str[255] = '\0';
				myflute->setInterfaceName(name_str);
			} else if (!strncmp(optarg, "fa", 2)) {
				/* process addr/hostname argument */
				if (*(optarg + 2) == '\0') {
					EXIT(("Flute: ERROR, bad argument -i%s: do not use extra space\n", optarg))
				}
				char	interface_addr_str[256]; /* if name string */
				strncpy(interface_addr_str, optarg + 2, sizeof(interface_addr_str));
				interface_addr_str[255] = '\0';
				myflute->setInterface(interface_addr_str);
			} 
#else  /*WIN32 */
			if (!strncmp(optarg, "pv", 2)) {
				int ip_version = (int)atoi(optarg+2);
				myflute->setIpVersion(ip_version);
			}
#endif			
			else
				EXIT(("Flute: ERROR, bad argument -i%s\n", optarg))
			break;
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
	
	ASSERT((argv[argc-1]))

	/* we went through all options */
	/* Now lets go through file names */
	if (myflute->isSender() == true) {

		if (argv[optind] == NULL) {
			Usage(argc, argv);	
		}

		fileparam = (char**) calloc(1,MAXNUMBER_OF_FILES);
		for (int i=optind; i<argc;i++)
		{
			fileparam[i - optind] = (char *) malloc(MAX_PATH+MAX_FILENAME);
			strncpy((char *) fileparam[i - optind], argv[i], MAX_PATH+MAX_FILENAME);
		}

	}
	

	/* start sdp generator if required */
	if (create_sdp == true)
	{
		myflute->createSdp(sdp_file);
	}



}


/*
 * we received a SIGINT...
 */
void
interrupted (void)
{
	static int	abort_in_progress = 0;

	if (!abort_in_progress) {
		abort_in_progress = 1;
		PRINT(("Flute: aborted, call flute->abort()\n"));
		myflute->abort();
		delete myflute;
		PRINT(("Flute: aborted, exit\n"));
		exit(2);
	} /* else do nothing */
}
