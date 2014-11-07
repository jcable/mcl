/* $Id: flute_cb.cpp,v 1.4 2005/05/23 11:11:18 roca Exp $ */
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
 
flute_cb::flute_cb ()
{

	this->id = 0;
	this->verbose = 0;
	this->stats = 0;
	this->mode=NOT_INITIALIZED;
	this->session_address_set = false;
	this->session_addr = NULL;
	this->src_addr = NULL;
	this->mcast_if_addr = NULL;
	this->mcast_if_name = NULL;
	this->port = 9998;
	this->demux_label = 0;
	this->ssm = false;
	this->ttl = 1;
	this->fec_ratio = 2.0;
	this->reuse_tx_buff = 1;

#ifdef WIN32
	this->overwrite = PROMPT;
#else
	this->overwrite = ALWAYS;
#endif

	this->storeAllAdu = true; // by default

#ifdef WIN32
	this->pause = false;
	this->ip_version = 4;
#endif
	this->tx_huge_file = 0;
	this->tmp_dir_set = 0;
	this->txprof_set = false;	/* set tx profile only once */
	this->txprof_mode = MCL_TX_PROFILE_MID_RATE_INTERNET;
	this->txprof_dt_size = 0;	/* default datagram size in bytes */
	this->txprof_rate = 0.0;	/* default rate in pkts/s (converted
					   from bps)*/
	this->congestion_control = FLID_SL_CC;

	this->nb_layers = 0;
	this->nb_tx = 0;
	this->delivery_mode = DEL_MODE_PUSH;
	this->myfiles=NULL;
	
	/* FEC is NULL at startup and only get initizlized if is a sender*/
	this->fec = NULL;
	
	/* No sdp parser nedded to start with */
	this->sdp = NULL;

	/* Initialize fdt */
	this->fdt = new FluteFDT(this);

	/* Initialize tools */
	this->tools = new FluteTools(this);
	
	this->myfiles = new FluteFileList(this);
	
	/* Initialize mutex */
	flute_init_lock(&this->flutemutex);

	this->initialized = false;
	this->toiindex = 0;
	
	this->fdtinstance = NULL;

}
 
 
flute_cb::~flute_cb ()
{
	
	/* cleanup everything */

	
	/* free sdp class if needed */
	if (this->sdp != NULL) {
		delete this->sdp;
		this->sdp = NULL;
	}

	/* free tools class if needed */
	if (this->tools != NULL) {
		delete this->tools;
		this->tools = NULL;
	}

	/* free fdt class */
	if (this->fdt != NULL) {	
	 	delete this->fdt;
		this->fdt = NULL;		
	}

	/* free myfiles class */	
	if (this->myfiles != NULL) {
		delete this->myfiles;
		this->myfiles = NULL;		
	}

	/* free fdt-instance class */
	if (this->fdtinstance != NULL) {
		fdtinstance->release();
		this->fdtinstance=NULL;
	}

	/* free FEC class if needed */
	if (this->fec != NULL) {
		delete this->fec;
		this->fec = NULL;
	}	
}
