/* $Id: FluteFileDeliveryCarousel.h,v 1.2 2005/05/12 16:03:37 moi Exp $ */
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
 
#ifndef FLUTE_FILE_DELIVERY_CAROUSEL_H
#define FLUTE_FILE_DELIVERY_CAROUSEL_H


class FluteFileDeliveryCarousel {

public:
	/****** Public Members ************************************************/
	/**
	 * Default constructor.
	 */
	FluteFileDeliveryCarousel (class FluteSender *flute);

	/**
	 * Default destructor.
	 */
	~FluteFileDeliveryCarousel ();

	void reset();
	
	TOI_t addFile(char* filename);
	
	void removeFile(TOI_t toi);
	
	UINT64 startTxCycles(INT32 nb_cycles, bool blocking);
	
	void addTxCycles(INT32 nb_cycles);
	
	UINT64 stopTx();
	
	void setCallbackEndOfTxCycles(void* (*EndOfTxCycles_callback)(UINT64 BytesSent));

private:

	UINT64 BytesSent;

	void* waitTx(void * arg);
	void* (*endOfTxCycles_callback)(UINT64 BytesSent);

	flute_thread_t send_thread;
	
	int mode;

	class flute_cb * flutecb;
	class FluteSender * flute;

};

#endif
