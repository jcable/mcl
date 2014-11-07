/* $Id: FluteReceiver.h,v 1.3 2005/05/23 11:11:18 roca Exp $ */
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

#ifndef FLUTE_RECEIVER_H
#define FLUTE_RECEIVER_H
/**
 * This Class is the API for flute
 */
class FluteReceiver : public Flute {
  
public:
	/****** Public Members ************************************************/
	/**
	 * Default constructor.
	 */
	FluteReceiver ();

	/**
	 * Default destructor.
	 */
	~FluteReceiver ();

	/*** get/set functions ***/
	
	void setOverwriteFile (INT32 overwrite);
	
	void setSrcAddr (struct sockaddr *src_addr);

	void setSrcAddr (char	*source_addr_str);

	void setRecvDir (char * recv_dir);
	
	void setSelectAll (bool on_off);
	
	/*** File info and session control functions ***/
	
	UINT64 recv (bool blocking);

	void selectTOI (TOI_t toi) ;
	
	void unselectTOI (TOI_t toi) ;

	void setCallbackReceivedNewFileDescription (void* 
			(*ReceivedNewFileDescription_callback) (class FluteFileInfo * fileInfo));

	void setCallbackEndOfRx ( void* (*EndOfRx_callback)(UINT64 BytesRecvd));


private:
	/****** Private Members ***********************************************/
	UINT64	BytesReceived;		/* total amount of data recv'd*/

	void *waitRx(void * arg);
	void* (*endOfRx_callback)(UINT64 BytesRecvd);
	void* (*receivedNewFileDescription_callback)(class FluteFileInfo * fileInfo);
	
	flute_thread_t reception_thread;
	
};

#endif
