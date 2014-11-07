/* $Id: Flute.cpp,v 1.7 2005/05/24 09:56:15 moi Exp $ */
/*
 *  Copyright (c) 2003-2004 INRIA - All rights reserved
 *  main authors: Christoph Neumann - christoph.neumann@inrialpes.fr
 *		  Vincent Roca - vincent.roca@inrialpes.fr
 *		  Julien Laboure - julien.laboure@inrialpes.fr
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

#include "flute_includes.h"


/**
 * Flute constructor
 */
Flute::Flute () {

	flutecb = new flute_cb();

}


/**
 * Flute destructor
 */
Flute::~Flute () {

	/* KILL threads */

	if (flutecb->id != 0) mcl_close(flutecb->id);
	delete flutecb;

}


/**
 * Abort the current Flute session
 */
void Flute::abort () {

	if (flutecb->id != 0) mcl_abort(flutecb->id);
	
}


/**
 * Print an mcl about (showing compile options, version, etc.)
 */
void Flute::about () {

	flutecb->lock();

	if (mcl_ctl(flutecb->id, MCL_OPT_MOREABOUT, NULL, 0))
		EXIT(("Flute: Usage: mcl_ctl failed for MOREABOUT\n"))
		
	flutecb->unlock();
}


/**
 * Sets the mcl verbosity level
 * @param verbosity 	verbosity level
 */
void Flute::setVerbosity (INT32 verbosity) {

	flutecb->lock();
	
	flutecb->verbose = verbosity;
	flutecb->stats = 2;

	flutecb->unlock();

}


/**
 * Get the mcl verbosity level used
 * @return 	verbosity level
 */
INT32 Flute::getVerbosity () {

	INT32 result;
	flutecb->lock();
	result = flutecb->verbose;
	flutecb->unlock();
	return result;

	
}


/**
 * Sets the mcl stat level
 * @param stats 	statistic level
 */
void Flute::setStatsLevel (INT32 stats) {

	flutecb->lock();

	flutecb->stats = stats;

	flutecb->unlock();

}


/**
 * Get the mcl stat level used
 * @return 	statistic level
 */
INT32 Flute::getStatsLevel () {

	INT32 result;
	flutecb->lock();
	result = flutecb->stats;
	flutecb->unlock();
	return result;
	
}


/**
 * Sets the congestion control scheme:
 *	INVALID_CC:	initial value, nothing choosen
 *	NO_CC:		choose not to have any CC
 *	RLC_CC:		choose to have RLC CC
 *	FLID_SL_CC:	choose to have FLID SL (static layer) CC
 * @param cc_scheme	the congestion control scheme
 */
void Flute::setCCScheme (INT32 cc_scheme) {

	flutecb->lock();

	flutecb->congestion_control = cc_scheme;
	if (flutecb->congestion_control == NO_CC && flutecb->nb_layers == 0) flutecb->nb_layers = 1;

	flutecb->unlock();

}


/**
 * Sets the transmission profile:
 *	MCL_TX_PROFILE_LOW_RATE_INTERNET:	modem connection
 *	MCL_TX_PROFILE_MID_RATE_INTERNET:	eg with VPNs, xDSL
 *	MCL_TX_PROFILE_HIGH_SPEED_INTERNET:	several Mbps connection
 *	MCL_TX_PROFILE_HIGH_SPEED_LAN
 * @param tx_profile	the transmission profile
 */
void Flute::setTxProfile (INT32 tx_profile) {

	flutecb->lock();

	flutecb->txprof_mode = tx_profile;
	if (flutecb->txprof_mode == MCL_TX_PROFILE_HIGH_SPEED_LAN)
		flutecb->congestion_control = NO_CC; /* true in a LAN! */
	flutecb->txprof_set = true;

	flutecb->unlock();
	
}


/**
 * Sets the datagram size in bytes.
 * @param dt_size	datagram size in bytes
 */
void Flute::setDatagramSize (INT32 dt_size) {

	flutecb->lock();

	flutecb->txprof_dt_size = dt_size;

	flutecb->unlock();

}


/**
 * Sets the tx rate (in bits/sec).
 * @param tx_rate	 transmission rate in bits/s
 */
void Flute::setTxRate (INT32 tx_rate) {

	flutecb->lock();

	if (tx_rate > 0) {
		flutecb->txprof_rate = (double)tx_rate /
			((double)flutecb->txprof_dt_size * 8.0);
		/* no less than 1 pkt/s */
		flutecb->txprof_rate = max(1.0, flutecb->txprof_rate);
	}

	flutecb->unlock();

}


/**
 * Sets the nb of layers used.
 * @param nb_layers	 the number of layers
 */
void Flute::setNbLayers (INT32 nb_layers) {

	flutecb->lock();

	flutecb->nb_layers = nb_layers;

	flutecb->unlock();

}


/**
 * Sets the IP version used.
 * @param ip_version	 the ip version
 */
#ifdef WIN32
void Flute::setIpVersion (INT32 ip_version) {

	flutecb->lock();

	flutecb->ip_version = ip_version;
	if (flutecb->ip_version != 4 && flutecb->ip_version !=6)
	{
		EXIT(("Flute: ERROR, unknown IP version %i. Specify IP version 4 or 6\n",ip_version))
	}

	flutecb->unlock();
}
#endif


/**
 * Sets the SSM (source specific multicast) mode on/off.
 * @param set	 on or off?
 */
void Flute::setSSM (bool set) {

	flutecb->lock();

	flutecb->ssm = set;

	flutecb->unlock();
}


/**
 * Sets the session address on which to send/recv.
 * @param session_addr	 session address
 */
void Flute::setSessionAddr (struct sockaddr *session_addr) {

	flutecb->lock();
	
	flutecb->session_addr = session_addr;
	
	flutecb->unlock();
}


/**
 * Sets the session address on which to send/recv.
 * @param session_addr_str	 session address string
 */
void Flute::setSessionAddr (char	*session_addr_str) {

	flutecb->lock();

	strncpy(flutecb->session_addr_str, session_addr_str, sizeof(flutecb->session_addr_str));
	flutecb->session_address_set = true;

	flutecb->unlock();

}


/**
 * Sets the port of the Flute session
 * @param port	port number of the session
 */
void Flute::setPort (INT32 port) {

	flutecb->lock();

	flutecb->port = port;

	flutecb->unlock();

}


#ifndef WIN32
/**
 * Sets the interface to be used for rx and tx.
 * @param if_addr_str	 interface address string
 */
void Flute::setInterface (char *if_addr_str) {

	flutecb->lock();

	if (flutecb->tools->extract_addr(if_addr_str, &flutecb->mcast_if_addr, 0) < 0) {
		EXIT(("Flute: setInterface: ERROR, address extraction failed\n"))
	}

	flutecb->unlock();

}


/**
 * Sets the interface to be used for rx and tx.
 * @param if_name	 interface address
 */
void Flute::setInterface (struct sockaddr *if_addr) {

	flutecb->lock();
	
	flutecb->mcast_if_addr = if_addr;
	
	flutecb->unlock();

}


/**
 * Sets the interface to be used for rx and tx.
 * @param if_name	 interface name
 */
void Flute::setInterfaceName (char* if_name) {

	flutecb->lock();

	flutecb->mcast_if_name = (char *) calloc(1, strlen(if_name) + 1);
	if (flutecb->mcast_if_name == NULL) {
		EXIT(("Flute: ERROR, calloc failed during Flute::setInterfaceName\n"))
	}
	strncpy(flutecb->mcast_if_name, if_name, strlen(if_name) + 1);

	flutecb->unlock();

}
#endif /* !WIN32 */


/**
 * Sets the TSI of the Flute session
 * @param tsi	tsi of the session
 */
void Flute::setTSI (INT32 tsi) {

	flutecb->lock();

	flutecb->demux_label = tsi;

	flutecb->unlock();

}


/**
 * Set the temp dir (where temporary files are stored).
 * @param tmp_dir	name of the temp dir
 */
void Flute::setTmpDir (char * tmp_dir) {

	flutecb->lock();

	strncpy(flutecb->tmp_dir, tmp_dir, sizeof(flutecb->tmp_dir));
	flutecb->tmp_dir[MAX_PATH + MAX_FILENAME -1] = 0;
	/* add final / and \0 if not already present */
	if (tmp_dir[strlen(flutecb->tmp_dir)-1] != '/' &&
		    strlen(tmp_dir) <= MAX_PATH + MAX_FILENAME -2)
		    strcat(flutecb->tmp_dir, "/");
 	flutecb->tmp_dir_set = true;

	flutecb->unlock();

}


/**
 * Creartes an sdp file using the parameters of the current Flute session.
 * @param	filename of the sdp file
 */
void Flute::createSdp (char* fileparam) {

	flutecb->lock();

	flutecb->sdp = new FluteSDP(this->flutecb);
	flutecb->sdp->generateSDP(fileparam);
	delete flutecb->sdp;
	flutecb->sdp = NULL;

	flutecb->unlock();

}


/**
 * Parses an sdp file and sets all parameters accordingly.
 * @param	filename of the sdp file
 */
void Flute::parseSdp (char* fileparam) {

	flutecb->lock();

	flutecb->sdp = new FluteSDP(this->flutecb);
	flutecb->sdp->parseSDP(fileparam);
	delete flutecb->sdp;
	flutecb->sdp = NULL;

	flutecb->unlock();
}


/**
 * Get the port of the Flute session.
 * @return 	port of the flute session
 */
INT32 Flute::getPort () {

	INT32 val;
	
	flutecb->lock();
	val = flutecb->port;
	flutecb->unlock();

	return val;

}


/**
 * Are we are flute sender?
 * @return	true or false
 */
bool Flute::isSender() {


	flutecb->lock();
	
	if (flutecb->mode == SEND) {
		
		flutecb->unlock();
		return true;
	}
	
	flutecb->unlock();
	return false;
	
}


/**
 * Are we are flute receiver?
 * @return	true or false
 */
bool Flute::isReceiver() {

	flutecb->lock();
	
	if (flutecb->mode == RECV) {
		flutecb->unlock();
		return true;
	}
	flutecb->unlock();
	return false;
}


/**
 * Returns file information of the given toi
 * @param toi	toi of the file
 * @return	a FluteFileInfo class containing info about the file
 *              NULL if no file with given toi exists
 */
class FluteFileInfo *Flute::getFileInfo (TOI_t toi) 
{

	class FluteFileInfo* returnedFFileInfo;
	flutecb->lock();
	returnedFFileInfo = flutecb->fdt->getFileInfo(toi);
	flutecb->unlock();
	
	return returnedFFileInfo;
	
}


/**
 * Returns a list of file information. It is a list of files
 * contained in the FDT.
 * @return	a list of FluteFileInfo classes 
 *              containing info about the files
 *		NULL if FDT is empty.
 */
class FluteFileInfo *Flute::getFileInfoList () 
{
	class FluteFileInfo* returnedFFileInfo;
	flutecb->lock();
	returnedFFileInfo = flutecb->fdt->getFileInfoList();
	flutecb->unlock();	
	
	return returnedFFileInfo;

}


/**
 * Initialize the ALC library now.
 * All selected options are passed to MCL.
 */
void Flute::init ()
{
	int err = 0;

	ASSERT(flutecb->mode == SEND || flutecb->mode == RECV);

	/* Process address string */
	if (flutecb->session_address_set == true &&
	    flutecb->tools->extract_addr(flutecb->session_addr_str, &flutecb->session_addr, flutecb->port) < 0) {
			EXIT(("Flute: ERROR, extract_addr failed for session addr argument -a%s\n", flutecb->session_addr_str))
	}

	/* Some sanity checks */
	if (flutecb->congestion_control == NO_CC && flutecb->nb_layers > 1)
		EXIT(("Flute: ERROR, cannot set nb of layers > 1 with no congestion control\n"))
#ifndef WIN32		
	if (flutecb->session_addr != NULL && flutecb->session_addr->sa_family == AF_INET6 && flutecb->mcast_if_name == NULL)
	{
		EXIT(("Flute: ERROR, you must specify an interface name with IPv6\n"))
	}
#endif
	if (flutecb->ssm && (flutecb->src_addr == NULL) && (flutecb->mode == RECV)) {
		EXIT(("Flute: ERROR, you must specify a source address in SSM/Receiver mode\n"))
	}

	/* specify few important parameters... */
	err += mcl_ctl(flutecb->id, MCL_OPT_VERBOSITY, (void*)&flutecb->verbose, sizeof(flutecb->verbose));
	EXIT_ON_ERROR(err, ("Flute: ERROR, mcl_ctl failed for VERBOSITY\n"))
	err += mcl_ctl(flutecb->id, MCL_OPT_STATS,
				(void*)&flutecb->stats, sizeof(int));
	EXIT_ON_ERROR(err, ("Flute: ERROR, mcl_ctl failed for STATS\n"))
	err += mcl_ctl(flutecb->id, MCL_OPT_SET_FLUTE_MODE, NULL, 0);
	EXIT_ON_ERROR(err, ("Flute: ERROR, mcl_ctl failed for SET_FLUTE_MODE\n"))
	if (flutecb->congestion_control > 0) {	/* must be before TX_PROFILE */
		err += mcl_ctl(flutecb->id, MCL_OPT_SET_CC_SCHEME,
				(void*)&flutecb->congestion_control,
				sizeof(flutecb->congestion_control));
		EXIT_ON_ERROR(err, (
			"Flute: ERROR, mcl_ctl failed for MCL_OPT_SET_CC_SCHEME %d\n",
			flutecb->congestion_control))
	}
	if (flutecb->txprof_set) {
		err += mcl_ctl(flutecb->id, MCL_OPT_TX_PROFILE, (void*)&flutecb->txprof_mode,
				sizeof(flutecb->txprof_mode));
		EXIT_ON_ERROR(err, (
			"Flute: ERROR, mcl_ctl failed for TX_PROFILE\n"))
	}
	if (flutecb->txprof_dt_size > 0) {	/* must be after TX_PROFILE */
		err += mcl_ctl(flutecb->id, MCL_OPT_DATAGRAM_SIZE,
				(void*)&flutecb->txprof_dt_size,
				sizeof(flutecb->txprof_dt_size));
		EXIT_ON_ERROR(err, (
			"Flute: ERROR, mcl_ctl failed for DATAGRAM_SIZE %d\n",
			flutecb->txprof_dt_size))
	}
	if (flutecb->txprof_rate > 0.0) {	/* must be after TX_PROFILE */
		err += mcl_ctl(flutecb->id, MCL_OPT_TX_RATE,
				(void*)&flutecb->txprof_rate,
				sizeof(flutecb->txprof_rate));
		EXIT_ON_ERROR(err, (
			"Flute: ERROR, mcl_ctl failed for TX_RATE %f\n",
			flutecb->txprof_rate))
	}
	if (flutecb->nb_layers > 0) {
		err += mcl_ctl(flutecb->id, MCL_OPT_LAYER_NB,
				(void*)&flutecb->nb_layers,
				sizeof(int));
		EXIT_ON_ERROR(err, ("Flute: ERROR, mcl_ctl failed for LAYER_NB\n"))
	}
	if (flutecb->session_addr != NULL) {
		int	addr_len;
		addr_len = (flutecb->session_addr->sa_family == AF_INET6) ?
				sizeof(struct sockaddr_in6) :
				sizeof(struct sockaddr_in);
		err += mcl_ctl(flutecb->id, MCL_OPT_BIND,
				(void*)flutecb->session_addr, addr_len);
		EXIT_ON_ERROR(err, ("Flute: ERROR, mcl_ctl failed for BIND\n"))
	}
	if (flutecb->mcast_if_addr != NULL) {		/* in host format! */
		err += mcl_ctl(flutecb->id, MCL_OPT_SET_NETIF_ADDR,
				(void*)&flutecb->mcast_if_addr,
				sizeof(*flutecb->mcast_if_addr));
		EXIT_ON_ERROR(err, ("Flute: ERROR, mcl_ctl failed for SET_NETIF_ADDR\n"))
	} else if (flutecb->mcast_if_name != NULL) {
		err += mcl_ctl(flutecb->id, MCL_OPT_SET_NETIF_NAME,
				(void*)flutecb->mcast_if_name,
				strlen(flutecb->mcast_if_name));
		EXIT_ON_ERROR(err, ("Flute: ERROR, mcl_ctl failed for SET_NETIF_NAME\n"))
	}
	if (flutecb->ssm == true) {
		int	val = (int)flutecb->ssm;
		err += mcl_ctl(flutecb->id, MCL_OPT_SET_SSM,
				(void*)&val, sizeof(val));
		EXIT_ON_ERROR(err, (
			"Flute: ERROR, mcl_ctl failed for DEMUX_LABEL\n"))
	}
	if (flutecb->demux_label > 0) {
		err += mcl_ctl(flutecb->id, MCL_OPT_DEMUX_LABEL,
				(void*)&flutecb->demux_label,
				sizeof(flutecb->demux_label));
		EXIT_ON_ERROR(err, (
			"Flute: ERROR, mcl_ctl failed for DEMUX_LABEL\n"))
	}
	if (flutecb->ttl >= 0) {
		err += mcl_ctl(flutecb->id, MCL_OPT_TTL,
				(void*)&flutecb->ttl,
				sizeof(flutecb->ttl));
		EXIT_ON_ERROR(err, (
			"Flute: ERROR, mcl_ctl failed for TTL\n"))
	}
	if (flutecb->tmp_dir_set) {
		err += mcl_ctl(flutecb->id, MCL_OPT_TMP_DIR,
				(void*)&flutecb->tmp_dir,
				strlen(flutecb->tmp_dir));
		EXIT_ON_ERROR(err, (
			"Flute: ERROR, mcl_ctl failed for TMP_DIR \"%s\"\n",
			flutecb->tmp_dir))
	}

	if (flutecb->mode == SEND) {
		/*
		 * always use LCT1 now, in all cases...
		 */
		int mcl_option = MCL_SCHED_LCT1;
		if (mcl_ctl(flutecb->id, MCL_OPT_SCHED,
					(void*)&mcl_option,
					sizeof(mcl_option))) {
			EXIT(("Flute: mcl_ctl MCL_OPT_SCHED failed for LCT1\n"))
		}
	
	}
	else if (flutecb->mode == RECV) {
		if (flutecb->src_addr != NULL) {
			int	addr_len;
			addr_len = (flutecb->src_addr->sa_family == AF_INET6) ?
				sizeof(struct sockaddr_in6) :
				sizeof(struct sockaddr_in);
			if (mcl_ctl(flutecb->id, MCL_OPT_SRC_ADDR,
					(void*)flutecb->src_addr, addr_len)) {
				EXIT(("mcl_ctl: MCL_OPT_SRC_ADDR failed\n"))
			}
		}
	
	}
	flutecb->initialized = true;
}

