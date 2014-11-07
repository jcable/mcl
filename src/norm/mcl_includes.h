/* $Id: mcl_includes.h,v 1.7 2004/12/10 10:37:14 chneuman Exp $ */
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

/*
 * System and MCL includes.
 * Included by all source files.
 */


/****** System includes *******************************************************/

#ifdef SOLARIS
#define __EXTENSIONS__		// XXX: we have pbs without on Solaris 2.5.1
#endif

#include <stdio.h>
#include <stdlib.h>
//#include <iostream.h>
#include <iostream>
using namespace std;

#include <errno.h>
extern int errno;
#include <string.h>
#include <fcntl.h>
#include <signal.h>
#include <ctype.h>
#include <sys/types.h>
#include <math.h>

#if defined(DEBUG) && defined(MPATROL) && !defined(WIN32)
#include <mpatrol.h>
#else
#include <malloc.h>
#endif
#include <new>		// required by _set_new_handler on linux

#ifdef WIN32	/* WIN32 */

#include <time.h>
#include <io.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <STDDEF.H>

#else	/* UNIX Systems */

#include <unistd.h>
#include <values.h>	/* for MAXINT */
#include <inttypes.h>
#include <strings.h>
#include <sys/time.h>
#include <sys/wait.h>
#include <pthread.h>
#include <sys/uio.h>
#if defined(SOLARIS) || defined(FREEBSD)
#include <netinet/in_systm.h>
#endif /* SOLARIS || FREEBSD */
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <netinet/ip.h>
#include <netinet/udp.h>

#endif /* OS */


/****** MCL includes **********************************************************/

#include "../common/mcl_types.h"	// First: define portable types
#include "../common/mcl_osdep.h"	
#include "mcl_profile.h"		// Second: MCL-NORM default profile
#include "../common/mcl_defines.h"	// Third: major defines/constants
#include "../common/mcl_lib_api.h"

#include "../common/mcl_version.h"
#include "../common/mcl_addr.h"
#include "../common/mcl_debug.h"
#include "../common/mcl_itime.h"
#include "../common/mcl_list.h"
#ifdef RSE_FEC
#include "../common/rse_fec.h"
#endif

#include "mcl.h"
#include "mcl_norm_hdr.h"
#include "mcl_adu.h"
#include "mcl_du.h"
#include "mcl_block.h"
#include "mcl_cb.h"		// needed by all the following class decl.
#include "mcl_node.h"
#include "mcl_fsm.h"
#include "mcl_timer_handler.h"
#include "mcl_timer.h"
#include "../common/mcl_periodic_timer.h"
#include "mcl_periodic_proc.h"
#include "mcl_fec.h"
#include "mcl_stats.h"
#include "mcl_ses_channel.h"
#include "mcl_norm_pkt_mgmt.h"
#include "mcl_tx.h"
#include "mcl_tx_window.h"
#include "mcl_tx_ctrl.h"
//#include "mcl_tx_pgmcc.h"	not yet...
//#include "mcl_tx_tfmcc.h"	not yet...
#include "mcl_tx_storage.h"
#include "mcl_rx.h"
#include "mcl_rx_window.h"
#include "mcl_rx_thread.h"
#include "mcl_rx_ctrl.h"
//#include "mcl_rx_pgmcc.h"	not yet...
//#include "mcl_rx_tfmcc.h"	not yet...
#include "mcl_rx_storage.h"
#include "mcl_fsm.h"


