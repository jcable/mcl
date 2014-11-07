/* $Id: mcl.cpp,v 1.5 2004/05/26 12:54:56 roca Exp $ */
/*
 *  Copyright (c) 2004 INRIA - All rights reserved
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
UINT32		mcl_time_count;

mcl_periodic_timer	mcl_periodic_timer_obj;


/****** local static variables ******/


/**
 * Return the version # and credits.
 * Edit as appropriate...
 */
void
mcl_moreabout (void)
{
	//
	// credits (very important :-)
	//
	PRINT_OUT((mcl_stdout, "\n-- MultiCast Library (MCL) for the NORM reliable multicast protocol --\n"))
	PRINT_OUT((mcl_stdout, MCLv3_VERSION))
	PRINT_OUT((mcl_stdout, "  Copyright (c) 2003-2004 INRIA - All rights reserved\n"))
	PRINT_OUT((mcl_stdout, "  main author/contact: vincent.roca@inrialpes.fr\n"))
	PRINT_OUT((mcl_stdout, "  web site: http://www.inrialpes.fr/planete/people/roca/mcl/\n"))
	PRINT_OUT((mcl_stdout, "  MCL comes with ABSOLUTELY NO WARRANTY; This is free software,\n"))
	PRINT_OUT((mcl_stdout, "  and you are welcome to redistribute it under certain conditions;\n"))
	PRINT_OUT((mcl_stdout, "  See the GNU Lesser General Public License as published by the Free \n"))
	PRINT_OUT((mcl_stdout, "  Software Foundation, version 2.1 or later, for more details.\n"))
	PRINT_OUT((mcl_stdout, "-- Credits:\n"))
	PRINT_OUT((mcl_stdout, "* Vincent Roca (INRIA R.A.)\n"))
#ifdef RSE_FEC
	PRINT_OUT((mcl_stdout, "* fec.c -- forward error corection based on Vandermonde matrices\n"))
	PRINT_OUT((mcl_stdout, "  (C) 1997-98 Luigi Rizzo (luigi@iet.unipi.it) (980624)\n"))
	PRINT_OUT((mcl_stdout, "  Portions derived from code by Phil Karn (karn@ka9q.ampr.org),\n"))
	PRINT_OUT((mcl_stdout, "  Robert Morelos-Zaragoza (robert@spectra.eng.hawaii.edu) and\n"))
	PRINT_OUT((mcl_stdout, "  Hari Thirumoorthy (harit@spectra.eng.hawaii.edu), Aug 1995\n"))
#endif
	PRINT_OUT((mcl_stdout, "* This product includes software developed by TASC, Inc. and the\n"))
	PRINT_OUT((mcl_stdout, "  University of Massachusetts at Amherst.\n"))
	//PRINT_OUT((mcl_stdout, "  Copyright (c) 1997-2001 TASC, Inc.\n"))
	//PRINT_OUT((mcl_stdout, "  Copyright (c) 1997-2001 University of Massachusetts at Amherst.\n"))

	//
	// compilation options
	//
	PRINT_OUT((mcl_stdout, "-- Compiled with:\n  "))
#ifdef INET6
	PRINT_OUT((mcl_stdout, "INET6 "))
#endif

#ifdef RSE_FEC
	PRINT_OUT((mcl_stdout, "RSE_FEC "))
#endif

#ifdef DEBUG
	PRINT_OUT((mcl_stdout, "DEBUG "))
#endif

#ifdef SIMUL_TX_LOSSES
	PRINT_OUT((mcl_stdout, "!!!SIMUL_TX_LOSSES!!! "))
#endif

#ifdef SIMUL_RX_LOSSES
	PRINT_OUT((mcl_stdout, "!!!SIMUL_RX_LOSSES!!! "))
#endif
	PRINT_OUT((mcl_stdout, "\n"))
}


/**
 * emergency exit function.
 * do as little as possible as we are in an unknown state...
 * @param n	int to be returned by exit()
 */
void
mcl_exit (int	n)
{
	int	id;
	mcl_cb	*mclcb = NULL;

	PRINT_ERR((mcl_stderr, "mcl_exit: ERROR, exit everything...\n"))
#if 0
	/* find the first valid mclcb first */
	for (id = 0; id < MAX_NB_MCLCB; id++) {
		if (mclcb_tab[id] != NULL) {
			mclcb = mclcb_tab[id];
			break;
		}
	}
#endif
	exit(n);
}


/**
 * new_handler function.
 */
static void
mcl_new_handler_func ()
{
	PRINT_ERR((mcl_stderr, "mcl_new_handler_func: ERROR, no memory left, exit everything...\n"))
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

