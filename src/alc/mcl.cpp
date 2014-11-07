/* $Id: mcl.cpp,v 1.15 2005/05/24 15:43:22 roca Exp $ */
/*
 *  Copyright (c) 1999-2004 INRIA - All rights reserved
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


#include "mcl_includes.h"

/****** definition of global MCL variables ******/

/* MCL version of standard streams.  */
FILE		*mcl_stdout;	/* MCL standard output stream */
FILE		*mcl_stderr;	/* MCL standard error output stream */

char		mcl_tmp_dir_name[MAX_FILE_NAME_LEN];

const UINT32	mcl_iss = 1;
UINT32		mcl_time_count = 0;

mcl_periodic_timer	mcl_periodic_timer_obj;


/****** local static variables ******/


#if 0
/* private functions */
static int	mcl_init_sender	(mcl_cb *mclcb);
static int	mcl_init_receiver(mcl_cb *mclcb);
#endif


/*
 *	version # and credits
 *	edit as appropriate!
 */
void
mcl_moreabout(void)
{
	/*
	 * credits (very important :-)
	 */
	PRINT_OUT((mcl_stdout, "\n-- MultiCast Library version 3 (MCLv3) --\n"))
	PRINT_OUT((mcl_stdout, MCLv3_VERSION))
	PRINT_OUT((mcl_stdout, "  Copyright (c) 1999-2005 INRIA - All rights reserved\n"))
	PRINT_OUT((mcl_stdout, "  main author/contact: vincent.roca@inrialpes.fr\n"))
	PRINT_OUT((mcl_stdout, "  web site: http://www.inrialpes.fr/planete/people/roca/mcl/\n"))
	PRINT_OUT((mcl_stdout, "  MCL comes with ABSOLUTELY NO WARRANTY; This is free software,\n"))
	PRINT_OUT((mcl_stdout, "  and you are welcome to redistribute it under certain conditions;\n"))
	PRINT_OUT((mcl_stdout, "  See the GNU General Public License as published by the Free \n"))
	PRINT_OUT((mcl_stdout, "  Software Foundation, version 2 or later, for more details.\n"))
	PRINT_OUT((mcl_stdout, "-- Credits:\n"))
	PRINT_OUT((mcl_stdout, "* Vincent Roca, Christoph Neumann, Julien Laboure, Benoit Mordelet\n"))
	PRINT_OUT((mcl_stdout, "  and many others (see the web site for more details...)\n"))
#ifdef RSE_FEC
	PRINT_OUT((mcl_stdout, "* fec.c -- forward error corection based on Vandermonde matrices\n"))
	PRINT_OUT((mcl_stdout, "  (C) 1997-98 Luigi Rizzo (luigi@iet.unipi.it) (980624)\n"))
	PRINT_OUT((mcl_stdout, "  Portions derived from code by Phil Karn (karn@ka9q.ampr.org),\n"))
	PRINT_OUT((mcl_stdout, "  Robert Morelos-Zaragoza (robert@spectra.eng.hawaii.edu) and\n"))
	PRINT_OUT((mcl_stdout, "  Hari Thirumoorthy (harit@spectra.eng.hawaii.edu), Aug 1995\n"))
#endif /* RSE_FEC */
#ifdef LDPC_FEC
	{
	LDPCFecSession	*ldpc_ses = new LDPCFecSession;
	ldpc_ses->InitSession(10, 20, 4);	/* don't care about args */
	ldpc_ses->MoreAbout(mcl_stdout);
	ldpc_ses->EndSession();
	}
#endif /* LDPC_FEC */
	/*
	 * compilation options
	 */
	PRINT_OUT((mcl_stdout, "-- Compiled with:\n  "))
#ifdef INET6
	PRINT_OUT((mcl_stdout, "INET6 "))
#endif

#ifdef SSM
	PRINT_OUT((mcl_stdout, "SSM "))
#endif

#ifdef RLC
	PRINT_OUT((mcl_stdout, "RLC "))
#endif

#ifdef FLIDS
	PRINT_OUT((mcl_stdout, "FLID-SL "))
#endif

#ifdef RSE_FEC
	PRINT_OUT((mcl_stdout, "RSE_FEC "))
#endif

#ifdef LDPC_FEC
	PRINT_OUT((mcl_stdout, "LDPC_FEC "))
#endif

#ifdef POSTPONE_FEC_DECODING
	PRINT_OUT((mcl_stdout, "POSTPONE_FEC_DECODING "))
#endif

#ifdef LCT_SCHED1
	PRINT_OUT((mcl_stdout, "LCT_SCHED1 "))
#endif

#ifdef ANTICIPATED_TX_FOR_PUSH
	PRINT_OUT((mcl_stdout, "ANTICIPATED_TX_FOR_PUSH "))
#endif

#ifdef VIRTUAL_TX_MEM
	PRINT_OUT((mcl_stdout, "VIRTUAL_TX_MEM (%d) ",VIRTUAL_TX_MEM_MAX_BUFFERING_SIZE))
#endif

#ifdef VIRTUAL_RX_MEM
	PRINT_OUT((mcl_stdout, "VIRTUAL_RX_MEM (%d) ",VIRTUAL_RX_MEM_MAX_BUFFERING_SIZE))
#endif

#ifdef DEBUG
	PRINT_OUT((mcl_stdout, "DEBUG "))
#endif

#ifdef GET_SYSINFO
	PRINT_OUT((mcl_stdout, "GET_SYSINFO "))
#endif

#ifdef SVSOA_RECV
	PRINT_OUT((mcl_stdout, "SVSOA_RECV "))
#endif

#ifdef SIMUL_LOSSES
	PRINT_OUT((mcl_stdout, "!!!SIMUL_LOSSES!!! "))
#endif
	PRINT_OUT((mcl_stdout, "\n"))
}


/**
 * emergency exit function.
 * do as little as possible as we are in an unknown state...
 * @param n	int to be returned by exit()
 */
void
mcl_exit (INT32	n)
{
	int	id;
	mcl_cb	*mclcb = NULL;

	PRINT_ERR((mcl_stderr, "mcl_exit: ERROR, exit everything...\n"))
	/* find the first valid mclcb first */
	for (id = 0; id < MAX_NB_MCLCB; id++) {
		if (mclcb_tab[id] != NULL) {
			mclcb = mclcb_tab[id];
			break;
		}
	}
	/* close the vtm/vrm services to destroy the temp files */
#ifdef VIRTUAL_TX_MEM
	if (mclcb)
		mcl_vtm_close (mclcb);
#endif
#ifdef VIRTUAL_RX_MEM
	if (mclcb)
		mclcb->rx_storage.close_vrmem(mclcb);
#endif
	exit(n);
}


/**
 * new_handler function.
 */
static void
mcl_new_handler_func ()
{
	PRINT_ERR((mcl_stderr, "mcl_new_handler_func: ERROR, not memory left, exit everything...\n"))
	mcl_exit(-1);
}


/**
 * Global MCL library initialization.
 * Done only once.
 */
void
mcl_global_init (void)
{
	// for the one time MCL global initialization
	static bool mcl_glob_initialized = false;

	if (mcl_glob_initialized == true)
		return;
	mcl_glob_initialized = true;

	set_new_handler(mcl_new_handler_func);
	mcl_init_random();
	strcat(mcl_tmp_dir_name, MCL_DEFAULT_TMP_DIR_NAME);

	mcl_stdout_stderr_init();

	/* initialize the session control block table */
	mcl_init_mclcb_tab();

#ifdef WIN32
	/* Socket initialisation for WinSock */
	mcl_winsock_init();
#endif  /* WIN32 */

	/* create and start the global periodic timer service */
	mcl_periodic_timer_obj.start();

	ASSERT(mcl_iss > 0);	/* must be > 0 (0 means non initialized) */
}


/**
 * Initialize the mcl_stdout and mcl_stderr outputs.
 * Assumes that the mcl_tmp_dir_name global variable is already initialized.
 */
void
mcl_stdout_stderr_init (void)
{
#ifdef STDOUT_TO_FILE
	char stdout_name[MAX_FILE_NAME_LEN];
#endif

#ifdef STDOUT_TO_FILE
	memset(stdout_name, 0, MAX_FILE_NAME_LEN);
	strncat(stdout_name, mcl_tmp_dir_name, MAX_FILE_NAME_LEN);
	strncat(stdout_name, "/mcl_out.txt", MAX_FILE_NAME_LEN);
	/* create temp file now in the tmp dir specified in mcl_tmp_dir_name */
#ifdef WIN32
	if ((mcl_stdout = open(vcb->f_name, _O_RDWR | O_CREAT | O_BINARY)) < 0) {
		perror("mcl_glob_init: ERROR, cannot open mlc_out.trc file");
		mcl_exit(-1);
	}
#else /* UNIX case */
	if ((mcl_stdout = fopen(stdout_name, "w")) == NULL) {
printf("error: cannot open %s\n", stdout_name);
		perror("mcl_glob_init: ERROR, cannot open mlc_out.trc file");
		mcl_exit(-1);
	}
#endif /* OS */
	mcl_stderr = mcl_stdout;	/* err is the same then */
#else  /* STDOUT_TO_FILE */
	mcl_stdout = stdout;
	mcl_stderr = stderr;
#endif /* STDOUT_TO_FILE */
}



#if 0



/*
 * initialize the receiving context...
 */
static int
mcl_init_receiver (mcl_cb	*mclcb)		/* session cb */
{
	TRACELVL(5, (mcl_stdout, "-> mcl_init_receiver: id=%d\n",
		mclcb->get_id()))
	mclcb->rxlvl.socket_head = mclcb->socket_tab;
	mclcb->rxlvl.next_adu2give = NULL;	/* not yet known */
	mclcb->rxlvl.next_adu2give_seq = mcl_iss;
	/*
	 * ... and create the reception thread and lock.
	 */

#ifdef WIN32
	if (CreateThread(THREAD_ALL_ACCESS, 0, (LPTHREAD_START_ROUTINE) mcl_rx_thread, (void*)mclcb, 0, (LPDWORD)&mclcb->rx_thread) == NULL) {
		perror("mcl_init_receiver: CreateThread");
		mcl_exit(1);
	}
#else
	if ((pthread_create((pthread_t*)&mclcb->rx_thread, NULL, mcl_rx_thread, (void*)mclcb)) != 0) {
		perror("mcl_init_receiver: pthread_create");
		mcl_exit(1);
	}
#endif
	TRACELVL(5, (mcl_stdout, "<- mcl_init_receiver: OK\n"))
	return 0;
}


#endif
