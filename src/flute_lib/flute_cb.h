/* $Id: flute_cb.h,v 1.5 2005/05/24 09:56:16 moi Exp $ */
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
 
#ifndef FLUTE_CB_H
#define FLUTE_CB_H

#include "flute_includes.h"


/**
 * This Class handles the main flute parameters
 */
class flute_cb {
  
public:
	/****** Public Members ************************************************/
	/**
	 * Default constructor.
	 */
	flute_cb ();

	/**
	 * Default destructor.
	 */
	~flute_cb ();

	/**
	 *
	 */
	void lock ();

	/**
	 *
	 */
	void unlock ();

	/** 
	 * id of the mcl session
	 */
	INT32	id;
	
	/** 
	 * has the cb been succesfully intitialized ?
	 */
	bool initialized;
	
	/** 
	 * verbosity
	 */
	INT32	verbose;
	
	/** 
	 * how detailled display the statistic
	 */
	INT32	stats;
	
	/**
	 * either in SEND or RECV mode (or NOT_INITIALIZED)
	 */
	INT32	mode;
	
	/**
	 * session name string.
	 */
	char	session_addr_str[256]; 		/* session address string */

	bool	session_address_set;

	struct	sockaddr	*session_addr;	/* IPv4/v6 session address */
	struct	sockaddr	*src_addr;	/* IPv4/v6 sender address */
	struct	sockaddr 	*mcast_if_addr;	/* IPv4/v6 interface address */
	char	*mcast_if_name;			/* interface name (IPv4/IPv6) */
	INT32	port;
	INT32	demux_label;
	bool	ssm;		/* tells if SSM mode is set or not */

	INT32	ttl;

	float	fec_ratio;

	bool 	storeAllAdu;

	INT32	reuse_tx_buff;

	INT32	overwrite; /* PROMPT, ALWAYS, NEVER */
#ifdef WIN32
	bool	pause;
	INT32	ip_version;
#endif
	INT32	tx_huge_file;
	
	char	tmp_dir[MAX_PATH + MAX_FILENAME];
	INT32	tmp_dir_set;
	
	/* transmission profile variables */
	bool	txprof_set;	/* set tx profile only once */
	INT32	txprof_mode;
	INT32	txprof_dt_size;	/* default datagram size in bytes */
	double	txprof_rate;	/* default rate in pkts/s (converted from
				   bps and rounded to upper integer) */
	INT32	congestion_control;

	INT32	nb_layers;
	INT32	nb_tx;
	INT32	delivery_mode;


	XERCES_CPP_NAMESPACE_QUALIFIER DOMDocument*	fdtinstance;

	/** Classes */
	class FluteSDP	 	*sdp;
	class FluteFDT	 	*fdt;
	class FluteTools 	*tools;

	/** Sender Side */
	class FluteFec		*fec;


	/** Receiver Side */
	class FluteFileList 	*myfiles;

	/* sender side */
	TOI_t 	toiindex;
	
	
private:
	flute_mutex_t flutemutex;

};
 
 
inline void
flute_cb::lock ()
{
	flute_lock(&(this->flutemutex));
}

inline void
flute_cb::unlock ()
{
	flute_unlock(&(this->flutemutex));
}


#endif
