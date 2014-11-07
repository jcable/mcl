/* $Id: mcl_includes.h,v 1.22 2005/05/24 10:37:57 roca Exp $ */
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

#ifndef MCL_INCLUDES_H  /* { */
#define MCL_INCLUDES_H

#ifdef SOLARIS
#define __EXTENSIONS__		/* XXX: we have pbs without on Solaris 2.5.1 */
#endif

/****** System includes *******************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <iostream>
using namespace std;

#include <errno.h>
#ifndef WIN32
extern int errno;
#endif /* OS */
#include <string.h>
#include <fcntl.h>
#include <signal.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <math.h>

#ifndef FREEBSD
#include <malloc.h>
#endif
#include <new>


/*
 * OS dependant includes
 */
#ifdef WIN32	/* { WIN32 */

#include <time.h>
#include <io.h>
#include <winsock2.h>
#include <ws2tcpip.h>

#include <windows.h> /* ADDED */
#include <shellapi.h> /* ADDED */
#include <mmsystem.h> /* ADDED */
#include <process.h> /* ADDED */
#include <direct.h> /* ADDED */


#else	/* } UNIX Systems { */

#include <unistd.h>

#ifndef FREEBSD
#include <values.h>	/* for MAXINT */
#endif

#include <strings.h>
#include <sys/time.h>
#include <sys/wait.h>
#include <pthread.h>
#include <sys/uio.h>
#if defined(SOLARIS) || defined(FREEBSD) || defined(HPUX) || defined(IRIX)
#include <netinet/in_systm.h>
#endif /* SOLARIS || FREEBSD */
#if defined(SOLARIS)
#include <sys/sockio.h>
#endif /* SOLARIS */
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <netinet/ip.h>
#include <netinet/udp.h>
#include <net/if.h>
#include <sys/ioctl.h>

#endif	/* } OS */


/****** MCL includes **********************************************************/


#include "../common/mcl_types.h"	// First: define portable data types
#include "../common/mcl_osdep.h"	// Second: OS specific defines
#include "mcl_profile.h"		// Third: MCL-ALC default profile
#include "../common/mcl_defines.h"	// Fourth: major defines/constants
#include "../common/mcl_lib_api.h"
#include "../common/mcl_version.h"
#include "../common/mcl_addr.h"
#include "../common/ssm_for_linux.h"
#include "../common/mcl_socket_lib.h"
#include "../common/mcl_debug.h"
#include "../common/mcl_itime.h"
#include "../common/mcl_blocking_struct.h"
#include "../common/mcl_periodic_timer.h"
#include "../common/mcl_file_io.h"

#include "mcl.h"	/* general purpose MCL constants */
#include "mcl_error.h"

#ifdef RSE_FEC
#include <rse_fec.h>
#endif
#ifdef LDPC_FEC
#include <ldpc_fec.h>
#endif

#include "mcl_lib.h"
#include "mcl_data.h"
#ifdef VIRTUAL_TX_MEM
#include "mcl_vtmem.h"
#endif
#ifdef VIRTUAL_RX_MEM
#include "mcl_vrmem.h"
#endif
#include "mcl_rx_storage.h"

#include "mcl_tx_tab.h"
#include "mcl_rlc.h"
#include "mcl_flid_sl.h"

#include "mcl_socket.h"
#include "mcl_periodic_proc.h"
#include "mcl_lct_hdr.h"
#include "mcl_alc_hdr.h"
#include "mcl_stats.h"
#include "mcl_fsm.h"
#include "mcl_proto.h"
#include "mcl_rx.h"
#include "mcl_rx_window.h"
#include "mcl_flute.h"
#include "mcl_tx.h"
#include "mcl_cb.h"
#include "mcl_fec.h"

#ifdef METAOBJECT_USED
#include "mcl_meta_object.h"
#include "mcl_meta_object_layer.h"
#endif

#endif /* }  MCL_INCLUDES_H */
