/* $Id: Flute.h,v 1.3 2005/05/23 11:11:17 roca Exp $ */
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

#ifndef FLUTE_H
#define FLUTE_H

/**
 * This Class is the API for flute
 */
class Flute {
  
public:
	/****** Public Members ************************************************/
	/**
	 * Default constructor.
	 */
	Flute ();

	/**
	 * Default destructor.
	 */
	~Flute ();

	void abort ();
	
	/********* Low level debug trace functions *******/
	void about ();

	void setVerbosity (INT32 verbosity);

	INT32 getVerbosity ();
	
	void setStatsLevel (INT32 stats);

	INT32 getStatsLevel ();

	
	/******* Common Receiver and Sender options ******/
	void setCCScheme (INT32 cc_scheme);

#ifdef WIN32
	void setIpVersion (INT32 ip_version);
#endif

	void setSSM (bool set);

	void setSessionAddr (struct sockaddr *session_addr);
	
	void setSessionAddr (char	*session_addr_str);

	void setPort (INT32 port);

	INT32 getPort ();

#ifndef WIN32
	void setInterface (char *if_addr_str);

	void setInterface (struct sockaddr *if_addr);

	void setInterfaceName (char* if_name);
#endif

	void setTSI (INT32 tsi);

	void setTmpDir (char * tmp_dir);

	/****** SDP functions *****/
	
	void createSdp (char* fileparam);

	void parseSdp (char* fileparam);
	

	/******* Transmission Profiles *******/
	void setTxProfile (INT32 tx_profile);
	
	void setDatagramSize (INT32 dt_size);
	
	void setTxRate (INT32 tx_rate);

	void setNbLayers (INT32 nb_layers);

	/**** Used as Sender or Receiver */
	
	bool isSender();
	
	bool isReceiver();

	/**** File Informations */

	class FluteFileInfo* getFileInfoList ();
	
	class FluteFileInfo* getFileInfo (TOI_t toi);


protected:
	void init();

	class flute_cb* flutecb;  /* the flute control block */

};

#endif
